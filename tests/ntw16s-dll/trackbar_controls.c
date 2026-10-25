#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TRACKBAR_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_style)(HWND);
static WNDPROC original;static HWND active;static unsigned mode,pre,items,post,scrolls,selected,action;static BOOL fired;
static LRESULT CALLBACK alias_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==TBM_GETTHUMBRECT && action && attached && attached(w) && !fired){
        fired=TRUE;if(action==1)remove_style(w);else {DestroyWindow(w);return 0;}
    }
    return CallWindowProcW(original,w,m,p,l);
}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_NOTIFY && l){NMHDR *h=(NMHDR*)l;
        if(h->hwndFrom==active && h->code==NM_CUSTOMDRAW){NMCUSTOMDRAW *d=(NMCUSTOMDRAW*)l;
            if(d->dwDrawStage==CDDS_PREPAINT){++pre;
                if(mode==1 || mode==4)return CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
                if(mode==2){RECT r;GetClientRect(active,&r);HBRUSH b=CreateSolidBrush(RGB(17,131,37));FillRect(d->hdc,&r,b);DeleteObject(b);return CDRF_SKIPDEFAULT;}
                if(mode==3 && !fired){fired=TRUE;remove_style(active);}
            }else if(d->dwDrawStage==CDDS_ITEMPREPAINT){++items;if(d->dwItemSpec==TBCD_THUMB && (d->uItemState&CDIS_SELECTED))++selected;
                if(mode==4 && d->dwItemSpec==TBCD_THUMB){HBRUSH b=CreateSolidBrush(RGB(131,17,197));FillRect(d->hdc,&d->rc,b);DeleteObject(b);return CDRF_SKIPDEFAULT;}
            }
            else if(d->dwDrawStage==CDDS_POSTPAINT)++post;
        }
    }
    if((m==WM_HSCROLL || m==WM_VSCROLL) && (HWND)l==active)++scrolls;
    return DefWindowProcW(w,m,p,l);
}
static DWORD native_bits[320*200],styled_bits[320*200];static DWORD *bits;static HDC dc;static HBITMAP bitmap;static HGDIOBJ old;
static void capture(HWND w,DWORD *out,BOOL frame)
{
    RECT clear={0,0,320,200};int save=SaveDC(dc);FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));
    SendMessageW(w,frame?WM_PRINT:WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();CopyMemory(out,bits,sizeof(native_bits));RestoreDC(dc,save);
}
static void save_image(HWND w,BOOL aware,unsigned kind,unsigned vertical)
{
    WCHAR dir[32768],path[32768];RECT r;GetClientRect(w,&r);
    if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",dir,32768))return;
    if(swprintf_s(path,32768,L"%ls/trackbar-%u-aware%u-kind%u-vertical%u.bmp",dir,(unsigned)sizeof(void*)*8,aware,kind,vertical)<0)return;
    FILE *f=_wfopen(path,L"wb");if(!f)return;
    BITMAPFILEHEADER h={.bfType=0x4d42,.bfOffBits=54,.bfSize=54+(DWORD)(r.right*r.bottom*4)};
    BITMAPINFOHEADER i={.biSize=40,.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};fwrite(&h,sizeof(h),1,f);fwrite(&i,sizeof(i),1,f);
    for(int y=0;y<r.bottom;++y)fwrite(styled_bits+y*320,4,(size_t)r.right,f);
    fclose(f);
}
static int child(BOOL styled,BOOL aware)
{
    CHECK(ImmDisableIME(0));if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&cc));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(styled?dll!=NULL:dll==NULL);
    attached=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;remove_style=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    WNDCLASSW parent={.lpfnWndProc=parent_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"TrackbarProtocolParent"};CHECK(RegisterClassW(&parent));
    HWND root=CreateWindowW(parent.lpszClassName,L"Trackbar protocol",WS_OVERLAPPEDWINDOW,0,0,640,300,NULL,NULL,parent.hInstance,NULL);CHECK(root);ShowWindow(root,SW_SHOWNORMAL);ShowWindow(root,SW_SHOW);
    WNDCLASSEXW alias={.cbSize=sizeof(alias)};CHECK(GetClassInfoExW(NULL,TRACKBAR_CLASSW,&alias));original=alias.lpfnWndProc;alias.hInstance=parent.hInstance;alias.lpszClassName=L"TTrackBar";alias.lpfnWndProc=alias_proc;alias.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&alias));
    if(styled){WCHAR path[32768];CHECK(attached && remove_style && attached(root) && GetModuleFileNameW(dll,path,32768));printf("TRACKBAR_MODULE bits=%u pid=%lu path=%ls bootstrap_frame_attached=1\n",(unsigned)sizeof(void*)*8,GetCurrentProcessId(),path);}
    dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};info.bmiHeader.biSize=40;info.bmiHeader.biWidth=320;info.bmiHeader.biHeight=-200;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(dc && bitmap);old=SelectObject(dc,bitmap);
    unsigned changed=0;const WCHAR *names[]={TRACKBAR_CLASSW,L"TTrackBar"};
    for(unsigned kind=0;kind<2;++kind)for(unsigned vertical=0;vertical<2;++vertical)for(unsigned variant=0;variant<5;++variant){
        DWORD option=variant==1?TBS_NOTICKS:variant==2?TBS_BOTH:variant==3?TBS_TOP:variant==4?TBS_FIXEDLENGTH:0;
        HWND w=CreateWindowW(names[kind],L"",WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|option|(vertical?TBS_VERT:0),0,0,vertical?64:320,vertical?200:64,root,(HMENU)31,parent.hInstance,NULL);CHECK(w);active=w;if(styled)CHECK(attached(w));
        SendMessageW(w,TBM_SETRANGEMIN,FALSE,(LPARAM)-20);SendMessageW(w,TBM_SETRANGEMAX,FALSE,80);SendMessageW(w,TBM_SETPOS,TRUE,30);SendMessageW(w,TBM_SETLINESIZE,0,3);SendMessageW(w,TBM_SETPAGESIZE,0,11);
        HFONT font=CreateFontW(-13,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(font);SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);HFONT stored=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        RECT thumb,channel,client,after;GetClientRect(w,&client);SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);SendMessageW(w,TBM_GETCHANNELRECT,0,(LPARAM)&channel);
        if(variant==4)SendMessageW(w,TBM_SETTHUMBLENGTH,19,0);
        SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);SendMessageW(w,TBM_GETCHANNELRECT,0,(LPARAM)&channel);
        mode=0;pre=items=post=0;SendMessageW(w,WM_SYSCOLORCHANGE,0,0);capture(w,styled_bits,FALSE);CHECK(pre && !items && !post);
        capture(w,native_bits,TRUE);CHECK(!memcmp(styled_bits,native_bits,sizeof(native_bits)));
        if(styled){int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));CHECK((styled_bits[thumb.top*320+thumb.left]&0xffffff)==0);CHECK((styled_bits[(thumb.top+u)*320+thumb.left+u]&0xffffff)==0xffffff);++changed;if(!variant)save_image(w,aware,kind,vertical);}
        RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);HDC visible=GetDC(w);CHECK(visible);if(styled)CHECK(GetPixel(visible,thumb.left,thumb.top)==0);ReleaseDC(w,visible);
        mode=1;pre=items=post=selected=0;SendMessageW(w,WM_SYSCOLORCHANGE,0,0);capture(w,styled_bits,FALSE);CHECK(pre && items>=2 && post);
        mode=4;SendMessageW(w,WM_SYSCOLORCHANGE,0,0);capture(w,styled_bits,FALSE);CHECK((styled_bits[((thumb.top+thumb.bottom)/2)*320+(thumb.left+thumb.right)/2]&0xffffff)==0x8311c5);
        mode=0;SendMessageW(w,WM_SYSCOLORCHANGE,0,0);BYTE saved[256],keys[256]={0};CHECK(GetKeyboardState(saved));keys[VK_LBUTTON]=0x80;CHECK(SetKeyboardState(keys));LPARAM pt=MAKELPARAM((thumb.left+thumb.right)/2,(thumb.top+thumb.bottom)/2);
        PostMessageW(w,WM_LBUTTONUP,0,pt);SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,pt);capture(w,styled_bits,FALSE);CHECK(GetCapture()==w);if(styled){int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));CHECK((styled_bits[(thumb.top+u)*320+thumb.left+u]&0xffffff)==0x808080);}SendMessageW(w,WM_LBUTTONUP,0,pt);CHECK(GetCapture()!=w);CHECK(SetKeyboardState(saved));MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE))DispatchMessageW(&msg);
        mode=0;SetFocus(w);CHECK(GetFocus()==w);SendMessageW(w,WM_KEYDOWN,vertical?VK_DOWN:VK_RIGHT,1);SendMessageW(w,WM_KEYUP,vertical?VK_DOWN:VK_RIGHT,1);CHECK(SendMessageW(w,TBM_GETPOS,0,0)==33 && scrolls);
        SendMessageW(w,TBM_SETPOS,TRUE,30);EnableWindow(w,FALSE);capture(w,styled_bits,FALSE);EnableWindow(w,TRUE);
        if(styled){LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);SetWindowLongPtrW(w,GWL_STYLE,style|TBS_ENABLESELRANGE);capture(w,styled_bits,FALSE);SetWindowLongPtrW(w,GWL_STYLE,style);}
        mode=2;SendMessageW(w,WM_SYSCOLORCHANGE,0,0);capture(w,styled_bits,FALSE);CHECK((styled_bits[20*320+20]&0xffffff)==0x118325);mode=0;
        SendMessageW(w,WM_SYSCOLORCHANGE,0,0);capture(w,styled_bits,FALSE);if(styled)CHECK(remove_style(w)&&!attached(w));capture(w,native_bits,FALSE);
        unsigned difference=0;for(int y=0;y<client.bottom;++y)for(int x=0;x<client.right;++x)difference+=(native_bits[y*320+x]&0xffffff)!=(styled_bits[y*320+x]&0xffffff);CHECK(styled?difference>10:!difference);
        GetClientRect(w,&after);RECT t,c;SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&t);SendMessageW(w,TBM_GETCHANNELRECT,0,(LPARAM)&c);
        CHECK(EqualRect(&client,&after)&&EqualRect(&thumb,&t)&&EqualRect(&channel,&c));CHECK(SendMessageW(w,TBM_GETPOS,0,0)==30 && SendMessageW(w,TBM_GETLINESIZE,0,0)==3 && SendMessageW(w,TBM_GETPAGESIZE,0,0)==11 && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==stored);
        printf("TRACKBAR_VARIANT kind=%u vertical=%u variant=%u styled=%u pixels=%u input_capture_keyboard=1 app_stages_preserved=1 geometry_font_data=1\n",kind,vertical,variant,styled,difference);active=NULL;CHECK(DestroyWindow(w)&&DeleteObject(font));
    }
    if(styled){
        mode=3;fired=FALSE;active=CreateWindowW(TRACKBAR_CLASSW,L"",WS_CHILD,0,0,300,64,root,(HMENU)32,parent.hInstance,NULL);CHECK(active && attached(active));capture(active,styled_bits,FALSE);CHECK(fired&&!attached(active));CHECK(DestroyWindow(active));active=NULL;mode=0;
        for(action=1;action<=2;++action){fired=FALSE;HWND w=CreateWindowW(L"TTrackBar",L"",WS_CHILD,0,0,300,64,root,(HMENU)33,parent.hInstance,NULL);CHECK(w&&attached(w));RECT r;SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&r);CHECK(fired);CHECK(action==1?!attached(w):!IsWindow(w));if(IsWindow(w))CHECK(DestroyWindow(w));}action=0;
        HWND w=CreateWindowW(TRACKBAR_CLASSW,L"",WS_CHILD|TBS_ENABLESELRANGE,0,0,300,64,root,(HMENU)34,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));
        CHECK(UnregisterClassW(L"TTrackBar",parent.hInstance));WNDCLASSW fake={.lpfnWndProc=DefWindowProcW,.hInstance=parent.hInstance,.lpszClassName=L"TTrackBar",.cbWndExtra=sizeof(void*)};CHECK(RegisterClassW(&fake));w=CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,300,64,root,(HMENU)35,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));
        puts("TRACKBAR_RETIRE_FALLBACK_OK notify_remove=1 query_remove_destroy=1 special_fake_native=1");
    }
    active=NULL;mode=0;DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(unsigned cycle=0;cycle<64;++cycle){HWND w=CreateWindowW(TRACKBAR_CLASSW,L"",WS_CHILD,0,0,300,64,root,(HMENU)36,parent.hInstance,NULL);CHECK(w);capture(w,styled_bits,FALSE);CHECK(DestroyWindow(w));}
    DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),user_after=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);CHECK(gdi_after<=gdi+2 && user_after<=user+2);
    printf("TRACKBAR_RESOURCE_OK cycles=64 gdi=%lu,%lu user=%lu,%lu\n",gdi,gdi_after,user,user_after);
    printf("TRACKBAR_CONTROL_OK bits=%u styled=%u aware=%u dpi=%u changed=%u\n",(unsigned)sizeof(void*)*8,styled,aware,GetDpiForWindow(root),changed);
    CHECK(DestroyWindow(root)&&SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc));return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD started=GetTickCount(),code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        printf("TRACKBAR_WORKER aware=%u styled=%u child_pid=%lu creator_pid=%lu elapsed_ms=%lu wait=%lu exit=%lu\n",aware,styled,pi.dwProcessId,GetCurrentProcessId(),GetTickCount()-started,wait,code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("TRACKBAR_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);return isolated_process(argc,argv,worker);}
