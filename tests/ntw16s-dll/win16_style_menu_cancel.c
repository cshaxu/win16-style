/* Actual DLL/native popup loops on an owned private desktop. No SendInput. */
#include "ntw16s-dll/win16_style.h"
#define NTW16S_TEST_PROCESS_TIMEOUT 140000
#include "isolated_process.h"
#include <windowsx.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MENU CANCEL FAIL line=%d error=%lu\n",__LINE__,GetLastError());ExitProcess(1);}}while(0)
static unsigned scenario,phase,entries,exits,commands;
static BOOL timed_out;
static POINT first,second,outside;
static HWND other,other_edit;
static BOOL native_case,observe_only,outline_only,preview_only;
static unsigned visible_shadows,border_checks,hidden_shadows;
static HWND preview_frame;
static void capture_menu(HWND popup)
{
    WCHAR path[MAX_PATH],suffix[80];DWORD n=GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",path,MAX_PATH);
    if(!n || n>=MAX_PATH-80 || !((scenario==0 && phase==0) || (scenario==9 && phase==2)))return;
    RECT frame,r;CHECK(GetWindowRect(preview_frame,&frame) && GetWindowRect(popup,&r));
    int width=max(frame.right,r.right)-frame.left,height=max(frame.bottom,r.bottom)-frame.top;
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=width,.biHeight=-height,.biPlanes=1,.biBitCount=32,.biCompression=BI_RGB}};
    void *pixels=NULL;HDC dc=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateDIBSection(NULL,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(dc && bitmap && pixels);
    HGDIOBJ old=SelectObject(dc,bitmap);RECT full={0,0,width,height};CHECK(FillRect(dc,&full,GetStockObject(WHITE_BRUSH)));
    SendMessageW(preview_frame,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND|PRF_CHILDREN);
    CHECK(SetViewportOrgEx(dc,r.left-frame.left,r.top-frame.top,NULL));
    SendMessageW(popup,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
    wsprintfW(suffix,L"\\menu-open-%u-case%u-width%d.bmp",GetDpiForWindow(popup),scenario,r.right-r.left);CHECK(wcslen(path)+wcslen(suffix)<MAX_PATH);wcscat(path,suffix);
    DWORD bytes=(DWORD)(width*height*4),written;BITMAPFILEHEADER header={.bfType=0x4d42,.bfSize=sizeof(header)+sizeof(info.bmiHeader)+bytes,.bfOffBits=sizeof(header)+sizeof(info.bmiHeader)};
    HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(file!=INVALID_HANDLE_VALUE);
    CHECK(WriteFile(file,&header,sizeof(header),&written,NULL) && written==sizeof(header));
    CHECK(WriteFile(file,&info.bmiHeader,sizeof(info.bmiHeader),&written,NULL) && written==sizeof(info.bmiHeader));
    CHECK(WriteFile(file,pixels,bytes,&written,NULL) && written==bytes && CloseHandle(file));
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc));
    printf("MENU OPEN CAPTURE PASS dpi=%u native WM_PRINT owned frame and open popup\n",GetDpiForWindow(popup));
}
static int popup_expected_width(UINT dpi)
{
    HFONT font=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,0,NONANTIALIASED_QUALITY,0,L"System");
    HDC dc=CreateCompatibleDC(NULL);CHECK(font && dc);HGDIOBJ old=SelectObject(dc,font);
    const WCHAR *texts[]={L"Deal",L"Undo",L"Deck...",L"Options...",L"Exit",L"F2"};int label=0,shortcut=0;
    for(unsigned i=0;i<6;++i){int width=0;for(const WCHAR *c=texts[i];*c;++c){SIZE size;CHECK(GetTextExtentPoint32W(dc,c,1,&size));width+=MulDiv(size.cx,(int)dpi,96);}
        if(i<5)label=max(label,width);else shortcut=width;
    }
    SelectObject(dc,old);DeleteObject(font);DeleteDC(dc);
    return MulDiv(18,(int)dpi,96)+label+MulDiv(8,(int)dpi,96)+shortcut+MulDiv(4,(int)dpi,96);
}
static BOOL CALLBACK border_probe(HWND w,LPARAM unused)
{
    WCHAR cls[32];RECT outer,client;(void)unused;
    if(!GetClassNameW(w,cls,32))return TRUE;
    if(!IsWindowVisible(w)){
        if(!native_case && !wcscmp(cls,L"SysShadow")){
            HWND previous=GetWindow(w,GW_HWNDPREV);WCHAR pc[32];RECT a,b;
            if(previous && IsWindowVisible(previous) && GetClassNameW(previous,pc,32) && !wcscmp(pc,L"#32768") &&
                GetWindowRect(w,&a) && GetWindowRect(previous,&b) && a.left==b.left && a.top==b.top && a.right>b.right && a.bottom>b.bottom){
                ++hidden_shadows;printf("MENU ASSOCIATED SHADOW HIDDEN PASS dpi=%u extension=%ld,%ld\n",GetDpiForWindow(previous),a.right-b.right,a.bottom-b.bottom);
            }
        }
        return TRUE;
    }
    if(!wcscmp(cls,L"SysShadow")){++visible_shadows;
        RECT r;CHECK(GetWindowRect(w,&r));
        printf("MENU SHADOW OBSERVE native=%d rect=%ld,%ld,%ld,%ld previous=%p\n",native_case,r.left,r.top,r.right,r.bottom,GetWindow(w,GW_HWNDPREV));
        if(!native_case && !observe_only)CHECK(FALSE);
        return TRUE;
    }
    if(wcscmp(cls,L"#32768") || native_case)return TRUE;
    CHECK(GetWindowRect(w,&outer) && GetClientRect(w,&client));MapWindowPoints(w,NULL,(POINT *)&client,2);
    OffsetRect(&client,-outer.left,-outer.top);
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    capture_menu(w);
    if(scenario==0 && phase==0){
        NONCLIENTMETRICSW ncm={.cbSize=sizeof(ncm)};CHECK(SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0,GetDpiForWindow(w)));
        HFONT font=CreateFontIndirectW(&ncm.lfMenuFont);HDC metric=CreateCompatibleDC(NULL);CHECK(font && metric);HGDIOBJ old=SelectObject(metric,font);TEXTMETRICW tm;SIZE zero,alphabet;
        CHECK(GetTextMetricsW(metric,&tm) && GetTextExtentPoint32W(metric,L"0",1,&zero) && GetTextExtentPoint32W(metric,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&alphabet));
        printf("MENU HOST METRIC dpi=%u height=%ld average=%ld zero=%ld alphabet=%ld\n",GetDpiForWindow(w),tm.tmHeight,tm.tmAveCharWidth,zero.cx,alphabet.cx);
        SelectObject(metric,old);DeleteObject(font);DeleteDC(metric);
    }
    MENUBARINFO mbi={.cbSize=sizeof(mbi)};MENUITEMINFOW mi={.cbSize=sizeof(mi),.fMask=MIIM_DATA|MIIM_FTYPE};RECT row;
    if(GetMenuBarInfo(w,OBJID_CLIENT,0,&mbi) && mbi.hMenu && GetMenuItemInfoW(mbi.hMenu,0,TRUE,&mi) && (mi.fType&MFT_OWNERDRAW) &&
        GetMenuItemRect(preview_frame,mbi.hMenu,0,&row)){
        MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemData=mi.dwItemData};
        CHECK(SendMessageW(preview_frame,WM_MEASUREITEM,0,(LPARAM)&measure));
        if(scenario==0 && phase==0){int expected=popup_expected_width(GetDpiForWindow(w));
            printf("MENU POPUP LAYOUT dpi=%u actual=%ld natural-System=%d\n",GetDpiForWindow(w),row.right-row.left,expected);
            CHECK(row.right-row.left==expected);
        }
        printf("MENU NATIVE WIDTH dpi=%u request=%u actual=%ld extra=%ld check=%d baseUnit=%u\n",GetDpiForWindow(w),measure.itemWidth,row.right-row.left,
            row.right-row.left-measure.itemWidth,GetSystemMetrics(SM_CXMENUCHECK),(unsigned)LOWORD(GetDialogBaseUnits()));
    }

    HDC dc=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateBitmap(width,height,1,32,NULL);CHECK(dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);
    RECT full={0,0,width,height};SetDCBrushColor(dc,RGB(255,0,255));
    CHECK(FillRect(dc,&full,(HBRUSH)GetStockObject(DC_BRUSH)));
    SendMessageW(w,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));unsigned pixels=0;
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){
        if(x>=client.left && x<client.right && y>=client.top && y<client.bottom)continue;
        COLORREF expected=x<u || y<u || x>=width-u || y>=height-u?RGB(0,0,0):RGB(255,255,255);
        CHECK(GetPixel(dc,x,y)==expected);++pixels;
    }
    /* In addition to the non-client outline, reject the old black block
     * left in the native client corner by incorrect WM_PRINT coordinates. */
    for(int y=client.bottom-2;!outline_only && y<client.bottom;++y)for(int x=client.right-2;x<client.right;++x){
        COLORREF pixel=GetPixel(dc,x,y);
        if(pixel==RGB(0,0,0) || pixel==RGB(255,0,255) || pixel==CLR_INVALID)
            printf("MENU CLIENT CORNER FAIL x=%d y=%d pixel=%06lx\n",x,y,pixel);
        CHECK(pixel!=RGB(0,0,0) && pixel!=RGB(255,0,255) && pixel!=CLR_INVALID);}
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc));++border_checks;
    printf("MENU BORDER PASS dpi=%u outer=%dx%d margins=%ld,%ld,%ld,%ld flat=%d pixels=%u\n",GetDpiForWindow(w),width,height,
        client.left,client.top,width-client.right,height-client.bottom,u,pixels);fflush(stdout);return TRUE;
}
static void key(HWND w,UINT value){CHECK(PostMessageW(w,WM_KEYDOWN,value,1));}
static LRESULT CALLBACK witness(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_ENTERMENULOOP)++entries;
    if(m==WM_EXITMENULOOP)++exits;
    if(m==WM_COMMAND && (LOWORD(a)==1 || LOWORD(a)==2) && !HIWORD(a)){++commands;return 0;}
    if(m==WM_TIMER && a==99){timed_out=TRUE;EndMenu();return 0;}
    if(m==WM_ENTERIDLE && a==MSGF_MENU){
        visible_shadows=0;CHECK(EnumThreadWindows(GetCurrentThreadId(),border_probe,0));
        if(native_case){printf("MENU NATIVE SHADOW OBSERVE visible=%u\n",visible_shadows);EndMenu();return 0;}
        ++phase;
        if(phase==1 && (scenario==0 || scenario==1 || scenario==3 || scenario==4 || scenario==8)){
            CHECK(PostMessageW(w,WM_MOUSEMOVE,0,MAKELPARAM(second.x,second.y)));return 0;
        }
        if(scenario==0 || scenario==6 || scenario==10 || scenario==12){key(w,VK_ESCAPE);return 0;}
        if(scenario==1 || scenario==5 || scenario==11){
            CHECK(PostMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(outside.x,outside.y)));
            CHECK(PostMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(outside.x,outside.y)));return 0;
        }
        if(scenario==2){
            CHECK(PostMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(first.x,first.y)));
            CHECK(PostMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(first.x,first.y)));return 0;
        }
        if(scenario==3){SendMessageW(w,WM_CANCELMODE,0,0);return 0;}
        if(scenario==4){SetActiveWindow(other);SetFocus(other_edit);return 0;}
        if(scenario==7 || scenario==8){key(w,VK_DOWN);key(w,VK_RETURN);return 0;}
        if(scenario==9){
            if(phase==1){key(w,VK_DOWN);key(w,VK_RIGHT);}
            else key(w,VK_ESCAPE);
            return 0;
        }
        CHECK(FALSE);
    }
    return DefWindowProcW(w,m,a,b);
}
static void drain(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}
static void native_popup(void)
{
    HWND w=CreateWindowW(L"MenuCancelProbe",L"Native shadow witness",WS_OVERLAPPEDWINDOW,30,30,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    HMENU menu=CreatePopupMenu();CHECK(menu && AppendMenuW(menu,MF_STRING,3,L"Unstyled native menu"));
    native_case=TRUE;timed_out=FALSE;ShowWindow(w,SW_SHOWNORMAL);CHECK(SetTimer(w,99,2500,NULL));
    TrackPopupMenuEx(menu,TPM_RETURNCMD|TPM_NONOTIFY,40,100,w,NULL);
    CHECK(!timed_out);KillTimer(w,99);native_case=FALSE;CHECK(DestroyMenu(menu) && DestroyWindow(w));drain();
    puts("MENU NATIVE SHADOW PASS unstyled popup executes before/after styled scope");
}
static int matrix(int argc,WCHAR **argv)
{
    CHECK((argc==2 || (argc==3 && (!wcscmp(argv[2],L"--observe") || !wcscmp(argv[2],L"--outline-only") || !wcscmp(argv[2],L"--preview")))) && SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    preview_only=argc==3 && !wcscmp(argv[2],L"--preview");
    observe_only=argc==3 && !wcscmp(argv[2],L"--observe");outline_only=argc==3 && !wcscmp(argv[2],L"--outline-only");
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_WITH_ALTERED_SEARCH_PATH);CHECK(dll);
    BOOL (__cdecl *install)(HWND,const win16_style_palette *)=(void *)GetProcAddress(dll,"win16_style_install");
    BOOL (__cdecl *remove_style)(HWND)=(void *)GetProcAddress(dll,"win16_style_remove");CHECK(install && remove_style);
    WNDCLASSW cls={.lpfnWndProc=witness,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"MenuCancelProbe"};
    CHECK(RegisterClassW(&cls));
    BOOL shadow_setting=FALSE,after_setting=FALSE;WNDCLASSW menu_class,after_class;
    CHECK(SystemParametersInfoW(SPI_GETDROPSHADOW,0,&shadow_setting,0) && GetClassInfoW(NULL,L"#32768",&menu_class));
    for(unsigned context=0;context<2;++context){
    CHECK(SetThreadDpiAwarenessContext(context?DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:DPI_AWARENESS_CONTEXT_UNAWARE));
    native_popup();border_checks=hidden_shadows=0;
    /* Keep an owned active window between cases. Destroying the active window
     * leaves USER to resolve activation against another desktop window and can
     * add seconds to every subsequent case. All case assertions precede the
     * switch, and each tested window is still independently destroyed. */
    HWND anchor=CreateWindowW(cls.lpszClassName,L"Menu test activation anchor",WS_OVERLAPPEDWINDOW,
        10,10,150,100,NULL,NULL,cls.hInstance,NULL);CHECK(anchor);
    ShowWindow(anchor,SW_SHOWNORMAL);
    for(scenario=0;scenario<(preview_only?1u:15u);++scenario){
        HWND w=CreateWindowW(cls.lpszClassName,L"Menu cancellation",WS_OVERLAPPEDWINDOW,
            30,30,650,350,NULL,NULL,cls.hInstance,NULL);CHECK(w);
        HMENU bar=CreateMenu(),a=CreatePopupMenu(),b=CreatePopupMenu(),nested=NULL;
        CHECK(bar && a && b && AppendMenuW(b,MF_STRING,2,L"&Command B"));
        if(scenario==9){nested=CreatePopupMenu();CHECK(nested && AppendMenuW(nested,MF_STRING,1,L"&Nested command") && AppendMenuW(a,MF_POPUP,(UINT_PTR)nested,L"&Child"));}
        else CHECK(AppendMenuW(a,MF_STRING,1,L"&Command A"));
        CHECK(AppendMenuW(bar,MF_POPUP,(UINT_PTR)a,L"&A") && AppendMenuW(bar,MF_POPUP,(UINT_PTR)b,L"&BBBBBBBBBBBBBBBBBBBB") && SetMenu(w,bar));
        if(scenario==0){
            CHECK(ModifyMenuW(bar,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)a,L"&Game") &&
                ModifyMenuW(bar,1,MF_BYPOSITION|MF_POPUP,(UINT_PTR)b,L"&Help"));
            CHECK(ModifyMenuW(a,0,MF_BYPOSITION|MF_STRING,1,L"&Deal\tF2") &&
                AppendMenuW(a,MF_STRING|MF_GRAYED,3,L"&Undo") && AppendMenuW(a,MF_STRING,4,L"Dec&k...") &&
                AppendMenuW(a,MF_STRING|MF_CHECKED,5,L"&Options...") && AppendMenuW(a,MF_SEPARATOR,0,NULL) &&
                AppendMenuW(a,MF_STRING,6,L"E&xit"));
        }
        MENUINFO original_a={.cbSize=sizeof(original_a),.fMask=MIM_STYLE},original_b=original_a;
        CHECK(GetMenuInfo(a,&original_a) && GetMenuInfo(b,&original_b));
        preview_frame=w;
        HWND edit=CreateWindowW(L"EDIT",L"Focus witness",WS_CHILD|WS_VISIBLE|WS_TABSTOP,20,50,200,30,w,NULL,cls.hInstance,NULL);CHECK(edit);
        other=CreateWindowW(cls.lpszClassName,L"Other owned window",WS_OVERLAPPEDWINDOW,700,30,200,150,NULL,NULL,cls.hInstance,NULL);CHECK(other);
        other_edit=CreateWindowW(L"EDIT",L"Other focus",WS_CHILD|WS_VISIBLE,0,0,100,30,other,NULL,cls.hInstance,NULL);CHECK(other_edit);
        ShowWindow(w,SW_SHOWNORMAL);ShowWindow(other,SW_SHOWNOACTIVATE);CHECK(install(w,NULL));
        SetActiveWindow(w);SetFocus(edit);drain();
        UINT dpi=GetDpiForWindow(w);POINT origin={0,0};CHECK(ClientToScreen(w,&origin));
        first.x=origin.x+MulDiv(8,(int)dpi,96);first.y=origin.y-MulDiv(8,(int)dpi,96);
        second.x=origin.x+MulDiv(scenario==0?70:100,(int)dpi,96);second.y=first.y;
        RECT client_bounds;CHECK(GetClientRect(w,&client_bounds));
        outside.x=origin.x+client_bounds.right-20;outside.y=origin.y+client_bounds.bottom-20;
        phase=entries=exits=commands=0;timed_out=FALSE;CHECK(SetTimer(w,99,2500,NULL));
        if(scenario==13 || scenario==14){
            SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,0);CHECK(GetFocus()==w);
            if(scenario==13)SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(40,90));
            else {RECT client;CHECK(GetClientRect(w,&client));
                SendMessageW(w,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM(origin.x+client.right-10,first.y));}
        }else if((scenario>=5 && scenario<=7) || scenario==10 || scenario==11){
            SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,0);CHECK(GetFocus()==w);
            if(scenario==10 || scenario==11)SendMessageW(w,WM_KEYDOWN,VK_LEFT,1);
            SendMessageW(w,WM_KEYDOWN,VK_DOWN,1);
        }else if(scenario==12)SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L'a');
        else SendMessageW(w,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM(first.x,first.y));
        drain();
        if(scenario==6 || scenario==10 || scenario==12){CHECK(GetFocus()==w);SendMessageW(w,WM_KEYDOWN,VK_ESCAPE,1);}
        printf("MENU CANCEL OBSERVE case=%u entries=%u exits=%u phase=%u commands=%u focusRestored=%d timeout=%d\n",scenario,entries,exits,phase,commands,GetFocus()==(scenario==4?other_edit:edit),timed_out);fflush(stdout);
        CHECK(!timed_out && entries==exits && (scenario>=13?entries==0:entries>=1) && GetCapture()==NULL);
        CHECK(GetFocus()==(scenario==4?other_edit:edit));
        unsigned before=entries;
        SendMessageW(w,WM_NCMOUSEMOVE,HTBORDER,MAKELPARAM(second.x,second.y));drain();
        CHECK(!timed_out && entries==before); /* cancellation must disable hover-open */
        CHECK(commands==((scenario==7 || scenario==8)?1u:0u));
        MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE};
        CHECK(GetMenuItemInfoW(a,0,TRUE,&item) && !(item.fType&MFT_OWNERDRAW));
        CHECK(GetMenuItemInfoW(b,0,TRUE,&item) && !(item.fType&MFT_OWNERDRAW));
        MENUINFO restored_a={.cbSize=sizeof(restored_a),.fMask=MIM_STYLE},restored_b=restored_a;
        CHECK(GetMenuInfo(a,&restored_a) && GetMenuInfo(b,&restored_b) && restored_a.dwStyle==original_a.dwStyle && restored_b.dwStyle==original_b.dwStyle);
        KillTimer(w,99);SetActiveWindow(anchor);SetFocus(anchor);CHECK(GetActiveWindow()==anchor && GetFocus()==anchor);
        CHECK(remove_style(w) && DestroyWindow(w) && DestroyWindow(other));drain();
        printf("MENU CANCEL CASE PASS %u native loop/focus/capture/hover/item restoration\n",scenario);
    }
    CHECK(DestroyWindow(anchor));CHECK(border_checks && hidden_shadows);native_popup();
    printf("MENU SHADOW SCOPE PASS aware=%u border-checks=%u hidden-associated=%u\n",context,border_checks,hidden_shadows);
    }
    CHECK(SystemParametersInfoW(SPI_GETDROPSHADOW,0,&after_setting,0) && GetClassInfoW(NULL,L"#32768",&after_class));
    CHECK(shadow_setting==after_setting && menu_class.style==after_class.style);
    CHECK(FreeLibrary(dll));if(preview_only){puts("MENU PREVIEW ONLY: actual native open menu, not the full cancellation matrix");return 0;}puts("MENU CANCEL PASS 15 scenarios per DPI context: mouse switch/cancel, second click, cancel mode, deactivate, keyboard cancel/escape/command, nested escape, system menu, mnemonic and idle bar clicks");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,matrix);}
