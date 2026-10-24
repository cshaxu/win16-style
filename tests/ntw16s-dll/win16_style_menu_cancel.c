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
static BOOL native_case,observe_only,outline_only;
static unsigned visible_shadows,border_checks;
static BOOL CALLBACK border_probe(HWND w,LPARAM unused)
{
    WCHAR cls[32];RECT outer,client;(void)unused;
    if(!IsWindowVisible(w) || !GetClassNameW(w,cls,32))return TRUE;
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
    CHECK((argc==2 || (argc==3 && (!wcscmp(argv[2],L"--observe") || !wcscmp(argv[2],L"--outline-only")))) && SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
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
    native_popup();border_checks=0;
    for(scenario=0;scenario<15;++scenario){
        HWND w=CreateWindowW(cls.lpszClassName,L"Menu cancellation",WS_OVERLAPPEDWINDOW,
            30,30,650,350,NULL,NULL,cls.hInstance,NULL);CHECK(w);
        HMENU bar=CreateMenu(),a=CreatePopupMenu(),b=CreatePopupMenu(),nested=NULL;
        CHECK(bar && a && b && AppendMenuW(b,MF_STRING,2,L"&Command B"));
        if(scenario==9){nested=CreatePopupMenu();CHECK(nested && AppendMenuW(nested,MF_STRING,1,L"&Nested command") && AppendMenuW(a,MF_POPUP,(UINT_PTR)nested,L"&Child"));}
        else CHECK(AppendMenuW(a,MF_STRING,1,L"&Command A"));
        CHECK(AppendMenuW(bar,MF_POPUP,(UINT_PTR)a,L"&A") && AppendMenuW(bar,MF_POPUP,(UINT_PTR)b,L"&BBBBBBBBBBBBBBBBBBBB") && SetMenu(w,bar));
        HWND edit=CreateWindowW(L"EDIT",L"Focus witness",WS_CHILD|WS_VISIBLE|WS_TABSTOP,20,50,200,30,w,NULL,cls.hInstance,NULL);CHECK(edit);
        other=CreateWindowW(cls.lpszClassName,L"Other owned window",WS_OVERLAPPEDWINDOW,700,30,200,150,NULL,NULL,cls.hInstance,NULL);CHECK(other);
        other_edit=CreateWindowW(L"EDIT",L"Other focus",WS_CHILD|WS_VISIBLE,0,0,100,30,other,NULL,cls.hInstance,NULL);CHECK(other_edit);
        ShowWindow(w,SW_SHOWNORMAL);ShowWindow(other,SW_SHOWNOACTIVATE);CHECK(install(w,NULL));
        SetActiveWindow(w);SetFocus(edit);drain();
        UINT dpi=GetDpiForWindow(w);POINT origin={0,0};CHECK(ClientToScreen(w,&origin));
        first.x=origin.x+MulDiv(8,(int)dpi,96);first.y=origin.y-MulDiv(8,(int)dpi,96);
        second.x=origin.x+MulDiv(100,(int)dpi,96);second.y=first.y;
        outside.x=origin.x+40;outside.y=origin.y+90;
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
        KillTimer(w,99);CHECK(remove_style(w) && DestroyWindow(w) && DestroyWindow(other));drain();
        printf("MENU CANCEL CASE PASS %u native loop/focus/capture/hover/item restoration\n",scenario);
    }
    CHECK(border_checks);native_popup();
    printf("MENU SHADOW SCOPE PASS aware=%u border-checks=%u\n",context,border_checks);
    }
    CHECK(SystemParametersInfoW(SPI_GETDROPSHADOW,0,&after_setting,0) && GetClassInfoW(NULL,L"#32768",&after_class));
    CHECK(shadow_setting==after_setting && menu_class.style==after_class.style);
    CHECK(FreeLibrary(dll));puts("MENU CANCEL PASS 15 scenarios per DPI context: mouse switch/cancel, second click, cancel mode, deactivate, keyboard cancel/escape/command, nested escape, system menu, mnemonic and idle bar clicks");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,matrix);}
