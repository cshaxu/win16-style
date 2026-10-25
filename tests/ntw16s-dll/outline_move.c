#include "ntw16s-dll/ntw16s.h"
#include "isolated_process.h"
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <dwmapi.h>
#include <wincrypt.h>
#include <tlhelp32.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MOVE FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static BOOL (*install_style)(HWND,const win16_style_palette *);
static BOOL (*remove_style)(HWND);
static BOOL (*attached)(HWND);
static RECT initial,initial_screen,last_screen;
static int mode,stage,enters,exits,moving,position_changes,failed;
static HWND other;
static RECT last_preview;
static DWORD before_pixels[960*640],during_pixels[960*640],expected_pixels[960*640],underlay_pixels[960*640];
static unsigned iteration;
static HWND overlay;
static UINT outline_dpi;
static HANDLE cancel_go;
enum { TEST_MOTION=WM_APP+101,TEST_KEY,TEST_RELEASE };
/* Thread-local probe input only. Do not let unrelated physical user input
 * commit/cancel a scripted transaction. No global hook or injected input. */
static LRESULT CALLBACK probe_messages(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0){MSG *m=(MSG *)lp;
        if(m->message==TEST_MOTION)m->message=WM_MOUSEMOVE;
        else if(m->message==TEST_KEY)m->message=WM_KEYDOWN;
        else if(m->message==TEST_RELEASE)m->message=WM_LBUTTONUP;
        else if((m->message>=WM_MOUSEFIRST && m->message<=WM_MOUSELAST) ||
                (m->message>=WM_KEYFIRST && m->message<=WM_KEYLAST) ||
                (m->message>=WM_NCMOUSEMOVE && m->message<=WM_NCXBUTTONDBLCLK))m->message=WM_NULL;
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
static DWORD WINAPI cancel_sender(void *data)
{
    DWORD_PTR result;
    if(WaitForSingleObject(cancel_go,3000)!=WAIT_OBJECT_0)return 1;
    return SendMessageTimeoutW((HWND)data,WM_CANCELMODE,0,0,SMTO_ABORTIFHUNG,1000,&result)?0:2;
}
static BOOL display_ready;
static unsigned display_ticks,stable_frames;
static unsigned default_a,default_w;
static int describe_image(const WCHAR *path,const char *role,BOOL native)
{
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;BYTE block[65536],digest[32];DWORD bytes=0,size=32,signature;
    IMAGE_DOS_HEADER dos;IMAGE_FILE_HEADER header;LARGE_INTEGER where={0};unsigned width;
    CHECK(file!=INVALID_HANDLE_VALUE && ReadFile(file,&dos,sizeof(dos),&bytes,NULL) && bytes==sizeof(dos) && dos.e_magic==IMAGE_DOS_SIGNATURE);
    where.QuadPart=dos.e_lfanew;CHECK(SetFilePointerEx(file,where,NULL,FILE_BEGIN));
    CHECK(ReadFile(file,&signature,sizeof(signature),&bytes,NULL) && bytes==sizeof(signature) && signature==IMAGE_NT_SIGNATURE);
    CHECK(ReadFile(file,&header,sizeof(header),&bytes,NULL) && bytes==sizeof(header));
    width=header.Machine==IMAGE_FILE_MACHINE_I386?32:header.Machine==IMAGE_FILE_MACHINE_AMD64?64:0;
    CHECK(width && (!native || width==sizeof(void*)*8));
    CHECK(CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT|CRYPT_SILENT) && CryptCreateHash(provider,CALG_SHA_256,0,0,&hash));
    where.QuadPart=0;CHECK(SetFilePointerEx(file,where,NULL,FILE_BEGIN));
    do{CHECK(ReadFile(file,block,sizeof(block),&bytes,NULL));if(bytes)CHECK(CryptHashData(hash,block,bytes,0));}while(bytes);
    CHECK(CryptGetHashParam(hash,HP_HASHVAL,digest,&size,0) && size==32);
    printf("OUTLINE_IMAGE role=%s bits=%u path=%ls sha256=",role,width,path);
    for(unsigned i=0;i<32;++i){printf("%02x",digest[i]);}
    puts("");
    CHECK(CryptDestroyHash(hash) && CryptReleaseContext(provider,0) && CloseHandle(file));return 0;
}
static int identities(HMODULE dll,const WCHAR *expected)
{
    WCHAR actual[4096],full[4096],parent[4096];DWORD parent_pid=0,length=4096;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);PROCESSENTRY32W entry={.dwSize=sizeof(entry)};
    CHECK(snapshot!=INVALID_HANDLE_VALUE);
    if(Process32FirstW(snapshot,&entry))do{if(entry.th32ProcessID==GetCurrentProcessId()){parent_pid=entry.th32ParentProcessID;break;}}while(Process32NextW(snapshot,&entry));
    CHECK(CloseHandle(snapshot) && parent_pid);
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,parent_pid);CHECK(process);
    CHECK(QueryFullProcessImageNameW(process,0,parent,&length) && CloseHandle(process));
    printf("OUTLINE_PROCESS pid=%lu creator=%lu thread=%lu\n",GetCurrentProcessId(),parent_pid,GetCurrentThreadId());
    CHECK(!describe_image(parent,"creator",FALSE));
    CHECK(GetModuleFileNameW(NULL,actual,ARRAYSIZE(actual)) && !describe_image(actual,"target",TRUE));
    CHECK(GetModuleFileNameW(dll,actual,ARRAYSIZE(actual)) && GetFullPathNameW(expected,ARRAYSIZE(full),full,NULL));
    for(WCHAR *p=full;*p;++p)if(*p==L'/')*p=L'\\';
    CHECK(!_wcsicmp(actual,full) && !describe_image(actual,"dll",TRUE));return 0;
}
static HBRUSH original_brush(int unit)
{
    WORD rows[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
    HDC a=CreateCompatibleDC(NULL),b=CreateCompatibleDC(NULL);
    HBITMAP source=CreateBitmap(8,8,1,1,rows),scaled=CreateBitmap(8*unit,8*unit,1,1,NULL);
    HGDIOBJ oa=NULL,ob=NULL;HBRUSH brush=NULL;
    if(a && b && source && scaled){
        oa=SelectObject(a,source);ob=SelectObject(b,scaled);SetStretchBltMode(b,COLORONCOLOR);
        if(StretchBlt(b,0,0,8*unit,8*unit,a,0,0,8,8,SRCCOPY))brush=CreatePatternBrush(scaled);
    }
    if(oa){SelectObject(a,oa);}
    if(ob){SelectObject(b,ob);}
    if(source){DeleteObject(source);}
    if(scaled){DeleteObject(scaled);}
    if(a){DeleteDC(a);}
    if(b){DeleteDC(b);}
    return brush;
}
/* Independent Win32 GDI reference. The bitmap is the original USER CPU
 * observation; GDI supplies pattern phase, monochrome colors and the ROP.
 * This does not call the product's software rasterizer or copy its parity math. */
static int reference_pixels(DWORD *output,const DWORD *input,RECT frame,int stroke,int unit,POINT origin,BOOL live)
{
    BITMAPINFO info={0};void *bits;HDC dc=CreateCompatibleDC(NULL);HBITMAP bitmap;HGDIOBJ old,old_brush;
    HBRUSH brush=original_brush(unit);HRGN outer,inner;
    info.bmiHeader.biSize=40;info.bmiHeader.biWidth=960;info.bmiHeader.biHeight=-640;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    bitmap=CreateDIBSection(NULL,&info,DIB_RGB_COLORS,&bits,NULL,0);CHECK(dc && brush && bitmap);
    old=SelectObject(dc,bitmap);old_brush=SelectObject(dc,brush);CHECK(old && old_brush);
    CopyMemory(bits,input,sizeof(before_pixels));OffsetRect(&frame,-60,-60);
    outer=CreateRectRgnIndirect(&frame);inner=CreateRectRgn(frame.left+stroke,frame.top+stroke,frame.right-stroke,frame.bottom-stroke);
    CHECK(outer && inner && CombineRgn(outer,outer,inner,RGN_DIFF)!=ERROR && SelectClipRgn(dc,outer)!=ERROR);
    if(live){
        /* Capture one complete native source rectangle, independent of the
         * product's four strips. Redirected unaware windows can resample a
         * screen source differently for different source rectangles. */
        int width=frame.right-frame.left,height=frame.bottom-frame.top;
        BITMAPINFO crop_info=info;HDC crop_dc=CreateCompatibleDC(NULL),screen=GetDC(NULL);
        HBITMAP crop;HGDIOBJ crop_old;void *crop_bits;
        crop_info.bmiHeader.biWidth=width;crop_info.bmiHeader.biHeight=-height;
        crop=CreateDIBSection(NULL,&crop_info,DIB_RGB_COLORS,&crop_bits,NULL,0);CHECK(crop_dc && screen && crop);
        crop_old=SelectObject(crop_dc,crop);CHECK(crop_old);
        CHECK(BitBlt(crop_dc,0,0,width,height,screen,frame.left+60,frame.top+60,SRCCOPY|CAPTUREBLT));
        CHECK(BitBlt(dc,frame.left,frame.top,width,height,crop_dc,0,0,SRCCOPY) && GdiFlush());
        CHECK(SelectObject(crop_dc,crop_old) && DeleteObject(crop) && DeleteDC(crop_dc) && ReleaseDC(NULL,screen));
    }
    SetTextColor(dc,RGB(0,0,0));SetBkColor(dc,RGB(255,255,255));
    CHECK(SetBrushOrgEx(dc,origin.x-60,origin.y-60,NULL));
    CHECK(PatBlt(dc,frame.left,frame.top,frame.right-frame.left,frame.bottom-frame.top,PATINVERT) && GdiFlush());
    CopyMemory(output,bits,sizeof(before_pixels));
    CHECK(SelectObject(dc,old_brush) && SelectObject(dc,old));
    CHECK(DeleteObject(outer) && DeleteObject(inner) && DeleteObject(brush) && DeleteObject(bitmap) && DeleteDC(dc));return 0;
}
static int raster_unit(void)
{
    /* Actual VGA FASTBORDER CPU trace, not generated by the product. */
    static const int strips[4][4]={{40,50,137,3},{43,147,137,3},{177,50,3,97},{40,53,3,97}};
    static const BYTE rows[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
    RECT frame={100,110,240,210};POINT origin={100,110};unsigned inverted=0;
    for(unsigned i=0;i<ARRAYSIZE(before_pixels);++i)before_pixels[i]=((i*31)&0xffffff);
    CopyMemory(during_pixels,before_pixels,sizeof(before_pixels));
    for(unsigned strip=0;strip<4;++strip)for(int y=strips[strip][1];y<strips[strip][1]+strips[strip][3];++y)
        for(int x=strips[strip][0];x<strips[strip][0]+strips[strip][2];++x)
            if(rows[(unsigned)(y-50)&7]&(0x80>>((unsigned)(x-40)&7))){during_pixels[y*960+x]^=0xffffff;++inverted;}
    CHECK(!reference_pixels(expected_pixels,before_pixels,frame,3,1,origin,FALSE));
    for(unsigned i=0;i<ARRAYSIZE(before_pixels);++i)CHECK(!((expected_pixels[i]^during_pixels[i])&0xffffff));
    CHECK(!reference_pixels(during_pixels,expected_pixels,frame,3,1,origin,FALSE));
    for(unsigned i=0;i<ARRAYSIZE(before_pixels);++i)CHECK(!((before_pixels[i]^during_pixels[i])&0xffffff));
    CHECK(inverted);printf("OUTLINE_GDI_REFERENCE_OK bits=%u original_strips=4 erase=exact invert_pixels=%u\n",(unsigned)(sizeof(void*)*8),inverted);return 0;
}
static BOOL CALLBACK find_overlay(HWND w,LPARAM data)
{
    WCHAR cls[100];GetClassNameW(w,cls,100);
    if(GetWindow(w,GW_OWNER)==(HWND)data && !wcsncmp(cls,L"Win16Outline-",13))overlay=w;
    return TRUE;
}
static BOOL screen_pixels(DWORD *pixels,const WCHAR *name)
{
    DPI_AWARENESS_CONTEXT context=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    BITMAPINFO info={0};void *bits=NULL;HDC screen=GetDC(NULL),dc=CreateCompatibleDC(NULL);
    HBITMAP bitmap;HGDIOBJ old;BOOL ok;WCHAR dir[32768],path[32768];
    info.bmiHeader.biSize=40;info.bmiHeader.biWidth=960;info.bmiHeader.biHeight=-640;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    bitmap=CreateDIBSection(NULL,&info,DIB_RGB_COLORS,&bits,NULL,0);
    if(!screen || !dc || !bitmap){if(screen)ReleaseDC(NULL,screen);if(dc)DeleteDC(dc);if(bitmap)DeleteObject(bitmap);SetThreadDpiAwarenessContext(context);return FALSE;}
    old=SelectObject(dc,bitmap);ok=BitBlt(dc,0,0,960,640,screen,60,60,SRCCOPY|CAPTUREBLT);GdiFlush();
    if(ok)CopyMemory(pixels,bits,sizeof(before_pixels));
    if(ok && name && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",dir,ARRAYSIZE(dir)) &&
       swprintf_s(path,ARRAYSIZE(path),L"%ls/%ls.bmp",dir,name)>0){
        FILE *f=_wfopen(path,L"wb");
        if(f){BITMAPFILEHEADER h={.bfType=0x4d42,.bfOffBits=54,.bfSize=54+sizeof(before_pixels)};
            fwrite(&h,sizeof(h),1,f);fwrite(&info.bmiHeader,40,1,f);fwrite(bits,sizeof(before_pixels),1,f);fclose(f);}
    }
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,screen);SetThreadDpiAwarenessContext(context);return ok;
}
static void fail(const char *what){fprintf(stderr,"MOVE scenario=%d stage=%d %s\n",mode,stage,what);failed=1;}
static LRESULT owner_core(HWND w,UINT msg,WPARAM wp,LPARAM lp,BOOL unicode)
{
    if(msg==WM_SYSCOMMAND && (wp&0xfff0)==SC_MOVE && mode==10)return 777;
    if(msg==WM_TIMER && wp==88){
        ++display_ticks;
        if(!screen_pixels(during_pixels,NULL)){fail("stability capture failed");display_ready=TRUE;}
        unsigned rgb_changes=0,raw_changes=0;
        for(unsigned i=0;i<ARRAYSIZE(before_pixels);++i){
            DWORD delta=before_pixels[i]^during_pixels[i];rgb_changes+=(delta&0xffffff)!=0;raw_changes+=delta!=0;
        }
        if((during_pixels[0]&0xffffff)==0x245184 &&
           (during_pixels[ARRAYSIZE(during_pixels)-1]&0xffffff)==0x245184 &&
           (during_pixels[(initial_screen.top+100-60)*960+initial_screen.left+100-60]&0xffffff)==0x245184 &&
           !rgb_changes)++stable_frames;else stable_frames=0;
        CopyMemory(before_pixels,during_pixels,sizeof(before_pixels));
        if(stable_frames>=3 || display_ticks>=40){
            if(display_ticks>=40){screen_pixels(during_pixels,L"unstable");fprintf(stderr,"DISPLAY color=%lx rect=%ld,%ld,%ld,%ld foreground=%p self=%p\n",during_pixels[(initial_screen.top+100-60)*960+initial_screen.left+100-60]&0xffffff,initial.left,initial.top,initial.right,initial.bottom,(void *)GetForegroundWindow(),(void *)w);}
            if(display_ticks>=40){
                fprintf(stderr,"DISPLAY_DELTA rgb=%u raw=%u\n",rgb_changes,raw_changes);
                DWORD cloaked=0,pid=0;WCHAR path[4096]=L"",desktop[128]=L"";DWORD count=4096,needed=0;
                GetWindowThreadProcessId(GetForegroundWindow(),&pid);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
                if(process){QueryFullProcessImageNameW(process,0,path,&count);CloseHandle(process);}
                GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed);
                DwmGetWindowAttribute(w,DWMWA_CLOAKED,&cloaked,sizeof(cloaked));
                fprintf(stderr,"DISPLAY_STATE visible=%d iconic=%d cloaked=%lu ex=%lx desktop=%ls foreground_image=%ls unicode=%d\n",IsWindowVisible(w),IsIconic(w),cloaked,(unsigned long)GetWindowLongPtrW(w,GWL_EXSTYLE),desktop,path,IsWindowUnicode(w));
            }
            display_ready=TRUE;KillTimer(w,88);}
        return 0;
    }
    if(msg==WM_ENTERSIZEMOVE){++enters;DwmFlush();
        if((mode==0 || mode==1) && !screen_pixels(before_pixels,mode==0?L"before":L"cancel-before"))fail("baseline capture failed");
        return 0;}
    if(msg==WM_EXITSIZEMOVE){++exits;return 0;}
    if(msg==WM_MOVING){
        ++moving;
        if(mode==6){RECT *r=(RECT *)lp;OffsetRect(r,7,5);}
        if(mode==7){SendMessageW(w,WM_SYSCOMMAND,SC_MOVE,0);if(moving==1)OffsetRect((RECT *)lp,100000,100000);}
        last_preview=*(RECT *)lp;
        return TRUE;
    }
    if(msg==WM_WINDOWPOSCHANGED && enters && !exits && !(((WINDOWPOS *)lp)->flags&SWP_NOMOVE))++position_changes;
    if(msg==WM_ERASEBKGND){RECT r;HBRUSH b=CreateSolidBrush(RGB(36,81,132));GetClientRect(w,&r);FillRect((HDC)wp,&r,b);DeleteObject(b);return 1;}
    if(msg==WM_PAINT){
        PAINTSTRUCT paint;RECT r;HDC dc=BeginPaint(w,&paint);HBRUSH b=CreateSolidBrush(RGB(36,81,132));
        GetClientRect(w,&r);FillRect(dc,&r,b);DeleteObject(b);EndPaint(w,&paint);return 0;
    }
    if(msg==WM_TIMER && wp==99){
        BOOL negative=(mode==0 || mode==1) && iteration>=32;
        RECT r;POINT p={initial.left+(negative?51:140),initial.top+(negative?-14:70)};GetWindowRect(w,&r);
        if(!EqualRect(&r,&initial))fail("true window moved before commit");
        if(GetCapture()!=w)fail("target capture missing");
        if(stage++==0){
            if(mode==2)PostMessageW(w,TEST_KEY,VK_RIGHT,0);
            else{ScreenToClient(w,&p);PostMessageW(w,TEST_MOTION,MK_LBUTTON,MAKELPARAM(p.x,p.y));}
        }else if((mode==6 || mode==7) && stage==2){
            if(mode==7){
                RECT outline_rect;DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                overlay=NULL;EnumThreadWindows(GetCurrentThreadId(),find_overlay,(LPARAM)w);
                if(!overlay || !GetWindowRect(overlay,&outline_rect) || !EqualRect(&outline_rect,&initial_screen))fail("fully off-screen candidate was not rejected");
                SetThreadDpiAwarenessContext(previous);
            }
            ScreenToClient(w,&p);PostMessageW(w,TEST_MOTION,MK_LBUTTON,MAKELPARAM(p.x,p.y));
        }else{
            KillTimer(w,99);
            if(!moving)fail("WM_MOVING missing");
            overlay=NULL;EnumThreadWindows(GetCurrentThreadId(),find_overlay,(LPARAM)w);
            if(!overlay || !IsWindowVisible(overlay))fail("owned visible outline missing");
            outline_dpi=GetDpiForWindow(overlay);
            {DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            if(!GetWindowRect(overlay,&last_screen)){fail("outline bounds missing");}
            SetThreadDpiAwarenessContext(previous);}
            if(mode==0 || mode==1){
                if(!screen_pixels(during_pixels,mode==0?L"during":L"cancel-during"))fail("display capture failed");
                ShowWindow(overlay,SW_HIDE);DwmFlush();
                if(!screen_pixels(underlay_pixels,L"underlay"))fail("underlay capture failed");
                if(mode==0){
                    UINT physical_dpi=GetDpiForWindow(overlay);POINT origin={initial_screen.left,initial_screen.top};
                    int border=(GetWindowLongPtrW(w,GWL_STYLE)&WS_THICKFRAME)?3:1;
                    DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                    if(reference_pixels(expected_pixels,underlay_pixels,last_screen,MulDiv(border,(int)physical_dpi,96),
                        max(1,MulDiv(1,(int)physical_dpi,96)),origin,TRUE))fail("native GDI reference failed");
                    SetThreadDpiAwarenessContext(previous);
                }
                ShowWindow(overlay,SW_SHOWNOACTIVATE);DwmFlush();
            }
            if(mode==0){
                unsigned mismatch=0,inverted=0;
                for(int y=0;y<640;++y)for(int x=0;x<960;++x){
                    int sx=x+60,sy=y+60;DWORD expected=expected_pixels[y*960+x]&0xffffff;
                    inverted+=expected!=(underlay_pixels[y*960+x]&0xffffff);
                    if((during_pixels[y*960+x]&0xffffff)!=expected){
                        if(mismatch<8)printf("PIXEL_DIFF x=%d y=%d before=%lx expected=%lx actual=%lx\n",sx,sy,before_pixels[y*960+x]&0xffffff,expected,during_pixels[y*960+x]&0xffffff);
                        ++mismatch;}
                }
                printf("OUTLINE_PIXELS mismatch=%u inverted=%u dpi=%u\n",mismatch,inverted,GetDpiForWindow(w));
                if(mismatch || !inverted)fail("display differs from original PATINVERT oracle");
            }
            if(mode==1)PostMessageW(w,TEST_KEY,VK_ESCAPE,0);
            else if(mode==3)SendMessageW(w,WM_CANCELMODE,0,0);
            else if(mode==4)SetCapture(other);
            else if(mode==5)DestroyWindow(w);
            else if(mode==8)remove_style(w);
            else if(mode==9)SendMessageW(w,WM_ACTIVATE,WA_INACTIVE,0);
            else if(mode==12)PostQuitMessage(73);
            else if(mode==13)SetWindowPos(w,NULL,initial.left+10,initial.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
            else if(mode==14)ShowWindow(w,SW_HIDE);
            else if(mode==15)SetEvent(cancel_go);
            else PostMessageW(w,mode==2?TEST_KEY:TEST_RELEASE,mode==2?VK_RETURN:0,0);
        }
        return 0;
    }
    if(msg==WM_SYSCOMMAND && (wp&0xfff0)==SC_MOVE){if(unicode)++default_w;else ++default_a;}
    return unicode?DefWindowProcW(w,msg,wp,lp):DefWindowProcA(w,msg,wp,lp);
}
static LRESULT CALLBACK owner(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{return owner_core(w,msg,wp,lp,TRUE);}
static LRESULT CALLBACK owner_ansi(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{return owner_core(w,msg,wp,lp,FALSE);}
static int worker(int argc,WCHAR **argv)
{
    WNDCLASSW cls={.lpfnWndProc=owner,.lpszClassName=L"OutlineMoveProbe"};
    HMODULE dll;BOOL setting_before,setting_after;DWORD resources;int i;HWND backdrop;HHOOK input_hook;
    BOOL snapshot=argc>2 && !wcscmp(argv[2],L"snapshot");
    BYTE saved_keys[256],keys[256]={0};
    WNDCLASSA ansi={.lpfnWndProc=owner_ansi,.lpszClassName="OutlineMoveProbeA"};
    CHECK(argc>=2);dll=LoadLibraryW(argv[1]);CHECK(dll);
    install_style=(void *)GetProcAddress(dll,"win16_style_install");
    remove_style=(void *)GetProcAddress(dll,"win16_style_remove");CHECK(install_style && remove_style);
    attached=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(attached);
    CHECK(SetProcessDpiAwarenessContext(argc>2 && !wcscmp(argv[2],L"unaware")?DPI_AWARENESS_CONTEXT_UNAWARE:DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    CHECK(!identities(dll,argv[1]));
    CHECK(GetKeyboardState(saved_keys) && SetKeyboardState(keys));
    input_hook=SetWindowsHookExW(WH_GETMESSAGE,probe_messages,NULL,GetCurrentThreadId());CHECK(input_hook);
    cls.hInstance=GetModuleHandleW(NULL);CHECK(RegisterClassW(&cls));
    ansi.hInstance=cls.hInstance;CHECK(RegisterClassA(&ansi));
    backdrop=CreateWindowExW(WS_EX_TOPMOST|WS_EX_NOACTIVATE,cls.lpszClassName,L"Owned backdrop",WS_POPUP,0,0,1040,720,NULL,NULL,cls.hInstance,NULL);CHECK(backdrop);
    ShowWindow(backdrop,SW_SHOWNOACTIVATE);CHECK(SetWindowPos(backdrop,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW));
    CHECK((GetWindowLongPtrW(backdrop,GWL_EXSTYLE)&(WS_EX_TOPMOST|WS_EX_NOACTIVATE))==(WS_EX_TOPMOST|WS_EX_NOACTIVATE));
    RedrawWindow(backdrop,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
    {RECT br,bc;GetWindowRect(backdrop,&br);GetClientRect(backdrop,&bc);
    printf("OUTLINE_BACKDROP visible=%d style=%lx ex=%lx rect=%ld,%ld,%ld,%ld client=%ld,%ld\n",IsWindowVisible(backdrop),(DWORD)GetWindowLongPtrW(backdrop,GWL_STYLE),(DWORD)GetWindowLongPtrW(backdrop,GWL_EXSTYLE),br.left,br.top,br.right,br.bottom,bc.right,bc.bottom);fflush(stdout);}
    other=CreateWindowW(cls.lpszClassName,L"Owned capture receiver",WS_OVERLAPPEDWINDOW,600,100,300,200,NULL,NULL,cls.hInstance,NULL);CHECK(other);
    CHECK(SystemParametersInfoW(SPI_GETDRAGFULLWINDOWS,0,&setting_before,0));
    resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(i=0;i<(snapshot?1:64);++i){
        HWND w;RECT after;BOOL cancelled;HANDLE sender=NULL;DWORD sender_code=1;
        DWORD frame_style=WS_OVERLAPPEDWINDOW&~(i>=32?WS_THICKFRAME:0);
        HMENU bar=CreateMenu(),file=CreatePopupMenu(),help=CreatePopupMenu();
        CHECK(bar && file && help && AppendMenuW(file,MF_STRING,101,L"&Open...") && AppendMenuW(file,MF_STRING,102,L"E&xit") &&
            AppendMenuW(help,MF_STRING,103,L"&About") && AppendMenuW(bar,MF_POPUP,(UINT_PTR)file,L"&File") && AppendMenuW(bar,MF_POPUP,(UINT_PTR)help,L"&Help"));
        BOOL unicode=((i/16)&1)==0;
        iteration=(unsigned)i;
        mode=i%16;stage=enters=exits=moving=position_changes=0;
        w=unicode?CreateWindowExW(WS_EX_NOACTIVATE,cls.lpszClassName,L"Win16 outline move",frame_style,100,100,400,270,backdrop,bar,cls.hInstance,NULL):
            CreateWindowExA(WS_EX_NOACTIVATE,ansi.lpszClassName,"Win16 outline move",frame_style,100,100,400,270,backdrop,bar,cls.hInstance,NULL);CHECK(w);
        CHECK((WNDPROC)(unicode?GetClassLongPtrW(w,GCLP_WNDPROC):GetClassLongPtrA(w,GCLP_WNDPROC))==(unicode?owner:owner_ansi));
        CHECK(attached(w));ShowWindow(w,SW_SHOWNOACTIVATE);SetWindowPos(w,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);UpdateWindow(w);CHECK(IsWindowVisible(w));CHECK(GetWindowRect(w,&initial));
        if(mode==10){CHECK(SendMessageW(w,WM_SYSCOMMAND,SC_MOVE,0)==777 && !enters);CHECK(DestroyWindow(w) && !IsMenu(bar));continue;}
        CHECK(SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(initial.left+80,initial.top+15))==HTCAPTION);
        {DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        CHECK(previous && GetWindowRect(w,&initial_screen));SetThreadDpiAwarenessContext(previous);}
        if(mode==0 || mode==1){
            DwmFlush();display_ready=FALSE;display_ticks=stable_frames=0;CHECK(SetTimer(w,88,30,NULL));
            while(!display_ready){MSG m;CHECK(GetMessageW(&m,NULL,0,0)>0);TranslateMessage(&m);DispatchMessageW(&m);}
            CHECK(stable_frames>=3 && !failed);
        }
        if(snapshot){
            CHECK(screen_pixels(before_pixels,L"snapshot") && DestroyWindow(w) && !IsMenu(bar));
            printf("OUTLINE_SNAPSHOT_OK bits=%u\n",(unsigned)(sizeof(void*)*8));break;
        }
        if(mode==15){cancel_go=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(cancel_go);sender=CreateThread(NULL,0,cancel_sender,w,0,NULL);CHECK(sender);}
        if(mode==2 && i>=32){keys[VK_CONTROL]=0x80;CHECK(SetKeyboardState(keys));}
        CHECK(SetTimer(w,99,25,NULL));
        SendMessageW(w,mode==11?WM_NCLBUTTONDOWN:WM_SYSCOMMAND,mode==11?HTCAPTION:SC_MOVE|(mode==2?0:HTCAPTION),MAKELPARAM(initial.left+80,initial.top+15));
        if(sender){CHECK(WaitForSingleObject(sender,2000)==WAIT_OBJECT_0 && GetExitCodeThread(sender,&sender_code) && !sender_code);CHECK(CloseHandle(sender) && CloseHandle(cancel_go));cancel_go=NULL;}
        keys[VK_CONTROL]=0;CHECK(SetKeyboardState(keys));
        if(mode==12){MSG quit;CHECK(PeekMessageW(&quit,NULL,WM_QUIT,WM_QUIT,PM_REMOVE) && quit.wParam==73);}
        printf("MOVE_RESULT mode=%d stage=%d enters=%d exits=%d moving=%d positions=%d\n",mode,stage,enters,exits,moving,position_changes);fflush(stdout);
        CHECK(stage==((mode==6 || mode==7)?3:2) && !failed && enters==1 && moving>=1);
        if(mode==5){CHECK(!IsWindow(w) && !IsMenu(bar));continue;}
        CHECK(GetWindowRect(w,&after));cancelled=mode==1 || mode==3 || mode==4 || mode==8 || mode==9 || mode>=12;
        if(mode==13){CHECK(after.left==initial.left+10 && after.top==initial.top);}
        else if(mode!=8){CHECK(cancelled?EqualRect(&initial,&after):!EqualRect(&initial,&after));}
        CHECK(exits==1);if(mode!=8)CHECK(position_changes==((cancelled && mode!=13)?0:1));
        if(!cancelled){
            int dx=mode==2?MulDiv(i>=32?1:9,(int)GetDpiForWindow(w),96):mode==6?67:mode==0 && i>=32?-29:60;
            int dy=mode==2?0:mode==6?60:mode==0 && i>=32?-29:55;
            CHECK(after.left==initial.left+dx && after.top==initial.top+dy);
        }
        if(mode==0 || mode==1){
            RECT physical;unsigned differences=0;DPI_AWARENESS_CONTEXT previous;
            DwmFlush();CHECK(screen_pixels(during_pixels,mode==0?L"committed":L"cancelled"));
            previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            CHECK(GetWindowRect(w,&physical));SetThreadDpiAwarenessContext(previous);
            if(mode==0)CHECK(EqualRect(&physical,&last_screen));
            if(mode==1){
                for(unsigned pixel=0;pixel<ARRAYSIZE(before_pixels);++pixel)
                    differences+=((underlay_pixels[pixel]^during_pixels[pixel])&0xffffff)!=0;
            }else{
                CHECK(physical.right-physical.left==initial_screen.right-initial_screen.left && physical.bottom-physical.top==initial_screen.bottom-initial_screen.top);
                for(int y=0;y<physical.bottom-physical.top;++y)for(int x=0;x<physical.right-physical.left;++x){
                    DWORD old=underlay_pixels[(initial_screen.top+y-60)*960+initial_screen.left+x-60];
                    DWORD now=during_pixels[(physical.top+y-60)*960+physical.left+x-60];
                    differences+=((old^now)&0xffffff)!=0;
                }
            }
            if(mode==0 && GetAwarenessFromDpiAwarenessContext(GetWindowDpiAwarenessContext(w))==DPI_AWARENESS_UNAWARE && outline_dpi!=GetDpiForWindow(w)){
                unsigned native_vs_product=0,native_vs_origin=0;
                /* Evidence: native-position-diag1 recorded product_vs_native=0
                 * and native_vs_translated_origin=7. Pure SetWindowPos fails
                 * the translated-screen-bitmap oracle too. Only for legacy
                 * redirection, compare every actual screen pixel to this
                 * independent native positioning control, with zero tolerance.
                 * Report the original translation difference; never erase it. */
                CHECK(SetWindowPos(w,NULL,initial.left,initial.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));
                CHECK(SetWindowPos(w,NULL,after.left,after.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));DwmFlush();
                CHECK(screen_pixels(expected_pixels,L"native-diagnostic"));
                for(unsigned pixel=0;pixel<ARRAYSIZE(during_pixels);++pixel)
                    native_vs_product+=((expected_pixels[pixel]^during_pixels[pixel])&0xffffff)!=0;
                for(int y=0;y<physical.bottom-physical.top;++y)for(int x=0;x<physical.right-physical.left;++x){
                    DWORD old=underlay_pixels[(initial_screen.top+y-60)*960+initial_screen.left+x-60];
                    DWORD now=expected_pixels[(physical.top+y-60)*960+physical.left+x-60];
                    native_vs_origin+=((old^now)&0xffffff)!=0;
                }
                printf("NATIVE_POSITION_DIAGNOSTIC product_vs_native=%u native_vs_translated_origin=%u\n",native_vs_product,native_vs_origin);
                CHECK(!native_vs_product && differences==native_vs_origin);
                printf("OUTLINE_FINAL_PIXELS mode=0 differences=%u reference=direct_SetWindowPos raw_translation_differences=%u\n",native_vs_product,differences);
            }else{
                printf("OUTLINE_FINAL_PIXELS mode=%d differences=%u\n",mode,differences);CHECK(!differences);
            }
        }
        CHECK(GetCapture()!=(HWND)w);if(GetCapture()==other)ReleaseCapture();
        CHECK(DestroyWindow(w) && !IsMenu(bar));
        printf("MOVE_RESOURCES iteration=%d gdi=%lu baseline=%lu\n",i,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),resources);
        if(i==31)resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    }
    CHECK(SystemParametersInfoW(SPI_GETDRAGFULLWINDOWS,0,&setting_after,0));CHECK(setting_after==setting_before);
    if(!snapshot){
        CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==resources);
        CHECK(default_a && default_w);printf("OUTLINE_DEFAULT_APIS ansi=%u unicode=%u\n",default_a,default_w);
    }
    CHECK(DestroyWindow(other));CHECK(DestroyWindow(backdrop));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance) && UnregisterClassA(ansi.lpszClassName,ansi.hInstance));CHECK(FreeLibrary(dll));
    CHECK(UnhookWindowsHookEx(input_hook) && SetKeyboardState(saved_keys));
    if(snapshot)return 0;
    printf("OUTLINE_MOVE_OK bits=%u scenarios=64 ansi_unicode=1 thick_fixed_frames=1 commit_cancel_capture_destroy_detach_reentry=1 application_veto=1 sent_cancel=1 quit=1 settings_unchanged=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
static int passive(int argc,WCHAR **argv)
{
    WNDCLASSW cls={.lpfnWndProc=owner,.lpszClassName=L"OutlinePassive"};HWND w;HMODULE dll;
    BYTE a[16],b[16];
    CopyMemory(a,(const void *)(ULONG_PTR)&DefWindowProcA,sizeof(a));CopyMemory(b,(const void *)(ULONG_PTR)&DefWindowProcW,sizeof(b));
    CHECK(argc==2);dll=LoadLibraryW(argv[1]);CHECK(dll);
    install_style=(void *)GetProcAddress(dll,"win16_style_install");CHECK(install_style);
    cls.hInstance=GetModuleHandleW(NULL);CHECK(RegisterClassW(&cls));
    mode=10;w=CreateWindowW(cls.lpszClassName,L"Passive API",WS_OVERLAPPEDWINDOW,20,20,300,200,NULL,NULL,cls.hInstance,NULL);
    CHECK(w && install_style(w,NULL));CHECK(SendMessageW(w,WM_SYSCOMMAND,SC_MOVE,0)==777);
    CHECK(!memcmp(a,(const void *)(ULONG_PTR)&DefWindowProcA,sizeof(a)) && !memcmp(b,(const void *)(ULONG_PTR)&DefWindowProcW,sizeof(b)));
    CHECK(!GetPropW(w,L"Win16Style.MoveTransaction") && DestroyWindow(w));
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance) && FreeLibrary(dll));
    puts("OUTLINE_PASSIVE_OK native_command_veto_preserved=1");return 0;
}
static int launch(int argc,WCHAR **argv)
{
    WCHAR directory[4096],run[4096],d32[4096],d64[4096],command[16384];
    HANDLE job,port;JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting;
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT association;BOOL empty=FALSE;
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};DWORD wait,code=1;
    CHECK(argc==6 || argc==7);
    if(argc==7)CHECK(!wcscmp(argv[6],L"unaware") || !wcscmp(argv[6],L"snapshot"));
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,4096));
    CHECK(!wcscat_s(directory,4096,L"/outline-runtime") && CreateDirectoryW(directory,NULL));
    CHECK(swprintf_s(run,4096,L"%ls/run16s.exe",directory)>0 && CopyFileW(argv[2],run,TRUE));
    CHECK(swprintf_s(d32,4096,L"%ls/ntw16s32.dll",directory)>0 && CopyFileW(argv[3],d32,TRUE));
    CHECK(swprintf_s(d64,4096,L"%ls/ntw16s64.dll",directory)>0 && CopyFileW(argv[4],d64,TRUE));
    CHECK(swprintf_s(command,16384,L"\"%ls\" --wait \"%ls\" --real \"%ls\" %ls",run,argv[5],sizeof(void*)==8?d64:d32,argc==7?argv[6]:L"")>0);
    /* Each driver is paired with a target of its own width. */
    job=CreateJobObjectW(NULL,NULL);limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
    port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);CHECK(port);
    association.CompletionKey=job;association.CompletionPort=port;
    CHECK(SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
    si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    CHECK(CreateProcessW(run,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
    CHECK(AssignProcessToJobObject(job,pi.hProcess));CHECK(ResumeThread(pi.hThread)!=-1u);
    wait=WaitForSingleObject(pi.hProcess,45000);if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
    if(wait!=WAIT_OBJECT_0)TerminateJobObject(job,ERROR_TIMEOUT);
    {ULONGLONG end=GetTickCount64()+3000;
    while(GetTickCount64()<end){DWORD event;ULONG_PTR key;OVERLAPPED *value;
        if(!GetQueuedCompletionStatus(port,&event,&key,&value,(DWORD)(end-GetTickCount64())))break;
        if(event==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO){empty=TRUE;break;}
    }}
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL));
    printf("OUTLINE_OWNED_JOB wait=%lu exit=%lu active=%lu\n",wait,code,accounting.ActiveProcesses);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CloseHandle(port);
    CHECK(wait==WAIT_OBJECT_0 && !code && empty && !accounting.ActiveProcesses);
    CHECK(DeleteFileW(run) && DeleteFileW(d32) && DeleteFileW(d64) && RemoveDirectoryW(directory));
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"--raster"))return raster_unit();
    if(argc>1 && !wcscmp(argv[1],L"--launch"))return launch(argc,argv);
    if(argc>1 && !wcscmp(argv[1],L"--real"))return worker(argc-1,argv+1);
    int result=isolated_process(argc,argv,passive);
    if(result)fprintf(stderr,"OUTLINE_PRIVATE_FAILURE exit=%d error=%lu\n",result,GetLastError());
    return result;
}
