#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <oleacc.h>
#include <objbase.h>
#include <shellscalingapi.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

static unsigned menu_scenario,menu_entries,menu_exits,menu_commands,menu_idle,menu_popup_inits;
static volatile LONG menu_frame_paints;
static BOOL menu_timeout;
static unsigned help_commands;
static BOOL custom_hit_enabled;
static LRESULT custom_hit_value;
static BOOL startup_probe, startup_ready;
static unsigned startup_early_size, startup_late_size;
static POINT menu_hover_point;
static DWORD app_pid;
static HWND app_window;
static volatile LONG app_popup_seen;
static int expected_physical_border(HWND window,int pixels,BOOL horizontal)
{
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    UINT x=dpi,y=dpi;
    DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if(prior){
        if(FAILED(GetDpiForMonitor(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),MDT_EFFECTIVE_DPI,&x,&y)) || !x || !y)x=y=dpi;
        SetThreadDpiAwarenessContext(prior);
    }
    return max(1,MulDiv(pixels,(int)dpi,(int)(horizontal?x:y)));
}
static unsigned diagnostic_down,diagnostic_double,diagnostic_up;
static LRESULT CALLBACK diagnostic_hook(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp){const MSG *m=(const MSG *)lp;
        if(m->message==WM_NCLBUTTONDOWN)++diagnostic_down;
        else if(m->message==WM_NCLBUTTONDBLCLK)++diagnostic_double;
        else if(m->message==WM_LBUTTONUP)++diagnostic_up;}
    return CallNextHookEx(NULL,code,wp,lp);
}
static BOOL CALLBACK find_app_window(HWND window,LPARAM ignored)
{
    (void)ignored;DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    if(pid==app_pid && IsWindowVisible(window) && !(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)){
        app_window=window;return FALSE;
    }
    return TRUE;
}
static BOOL CALLBACK find_app_popup(HWND window,LPARAM ignored)
{
    (void)ignored;DWORD pid=0;WCHAR name[32]={0};GetWindowThreadProcessId(window,&pid);
    if(pid==app_pid && IsWindowVisible(window) && GetClassNameW(window,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"#32768")){
        InterlockedExchange(&app_popup_seen,1);return FALSE;
    }
    return TRUE;
}
static DWORD WINAPI cancel_app_menu(void *ignored)
{
    (void)ignored;ULONGLONG deadline=GetTickCount64()+2000;
    do{EnumWindows(find_app_popup,0);if(InterlockedCompareExchange(&app_popup_seen,0,0))break;Sleep(50);}while(GetTickCount64()<deadline);
    PostMessageW(app_window,WM_CANCELMODE,0,0);PostMessageW(app_window,WM_KEYDOWN,VK_ESCAPE,1);return 0;
}
static LRESULT CALLBACK procedure(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(startup_probe && message==WM_SIZE){if(startup_ready)++startup_late_size;else ++startup_early_size;}
    if(custom_hit_enabled && message==WM_NCHITTEST)return custom_hit_value;
    if(message==WM_SYSCOMMAND && (wp&0xfff0)==SC_CONTEXTHELP){++help_commands;return 0;}
    if(menu_scenario){
        if(message==WM_ENTERMENULOOP)++menu_entries;
        if(message==WM_EXITMENULOOP)++menu_exits;
        if(message==WM_INITMENUPOPUP){++menu_popup_inits;if(menu_scenario==6)menu_idle=0;}
        if(message==WM_NCPAINT)InterlockedIncrement(&menu_frame_paints);
        if(message==WM_COMMAND && LOWORD(wp)==101){++menu_commands;return 0;}
        if(message==WM_TIMER && wp==77){menu_timeout=TRUE;EndMenu();return 0;}
        if(message==WM_ENTERIDLE && wp==MSGF_MENU && menu_scenario==5)return 0;
        if(message==WM_ENTERIDLE && wp==MSGF_MENU && !menu_idle++){
            if(menu_scenario==1){PostMessageW(w,WM_KEYDOWN,VK_DOWN,1);PostMessageW(w,WM_KEYDOWN,VK_RETURN,1);}
            else if(menu_scenario==3 && menu_entries==1){
                PostMessageW(w,WM_MOUSEMOVE,0,MAKELPARAM(menu_hover_point.x,menu_hover_point.y));
            }
            else if(menu_scenario==4 && menu_entries==1)PostMessageW(w,WM_KEYDOWN,VK_RIGHT,1);
            else if(menu_scenario==6 && menu_popup_inits==1){PostMessageW(w,WM_KEYDOWN,VK_DOWN,1);PostMessageW(w,WM_KEYDOWN,VK_RIGHT,1);}
            else if(menu_scenario==6){PostMessageW(w,WM_KEYDOWN,VK_ESCAPE,1);PostMessageW(w,WM_KEYDOWN,VK_ESCAPE,1);}
            else PostMessageW(w,WM_KEYDOWN,VK_ESCAPE,1);
            return 0;
        }
        if(message==WM_ENTERMENULOOP && (menu_scenario==3 || menu_scenario==4))menu_idle=0;
    }
    return DefWindowProcW(w,message,wp,lp);
}

static BOOL dialog_frame_seen;
static INT_PTR CALLBACK frame_dialog_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    (void)wp;(void)lp;
    if(message==WM_INITDIALOG){SetTimer(window,1,50,NULL);return TRUE;}
    if(message==WM_TIMER){
        TITLEBARINFOEX info={.cbSize=sizeof(info)};SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        dialog_frame_seen=GetPropW(window,L"Win2kStyle.Frame") && info.rcTitleBar.bottom-info.rcTitleBar.top==MulDiv(19,(int)dpi,96);
        KillTimer(window,1);EndDialog(window,7);return TRUE;
    }
    return FALSE;
}

static volatile LONG message_seen,message_attached,message_accept;
static BOOL CALLBACK find_message_window(HWND window,LPARAM unused)
{
    (void)unused;DWORD pid=0;WCHAR title[64]={0};GetWindowThreadProcessId(window,&pid);
    if(pid!=GetCurrentProcessId() || !IsWindowVisible(window) || !GetWindowTextW(window,title,ARRAYSIZE(title)) || lstrcmpW(title,L"Win2kMessageFixture"))return TRUE;
    TITLEBARINFOEX info={.cbSize=sizeof(info)};DWORD_PTR response=0;
    BOOL sent=SendMessageTimeoutW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info,SMTO_ABORTIFHUNG,1000,&response)!=0;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    if(sent && GetPropW(window,L"Win2kStyle.Frame") && info.rcTitleBar.bottom-info.rcTitleBar.top==MulDiv(19,(int)dpi,96))InterlockedExchange(&message_attached,1);
    InterlockedExchange(&message_seen,1);
    if(InterlockedCompareExchange(&message_accept,0,0)){
        HWND button=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)button);
        ULONGLONG deadline=GetTickCount64()+1500;
        while(IsWindow(window) && GetTickCount64()<deadline)Sleep(25);
        if(IsWindow(window)){PostMessageW(window,WM_CLOSE,0,0);Sleep(1000);if(IsWindow(window))ExitProcess(4);}
    }else PostMessageW(window,WM_CLOSE,0,0);
    return FALSE;
}
static DWORD WINAPI close_message(void *unused)
{
    (void)unused;ULONGLONG deadline=GetTickCount64()+3000;
    do{EnumWindows(find_message_window,0);if(InterlockedCompareExchange(&message_seen,0,0))return 0;Sleep(25);}while(GetTickCount64()<deadline);
    ExitProcess(4);
    return 1;
}

static HWND common_owner;
static volatile LONG common_done;
static BOOL CALLBACK find_common_window(HWND window,LPARAM unused)
{
    (void)unused;DWORD pid=0;WCHAR cls[32]={0};GetWindowThreadProcessId(window,&pid);
    if((pid!=GetCurrentProcessId() && (!common_owner || GetWindow(window,GW_OWNER)!=common_owner)) || !IsWindowVisible(window) || !GetClassNameW(window,cls,ARRAYSIZE(cls)) || lstrcmpW(cls,L"#32770"))return TRUE;
    TITLEBARINFOEX info={.cbSize=sizeof(info)};DWORD_PTR response=0;
    BOOL sent=SendMessageTimeoutW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info,SMTO_ABORTIFHUNG,1000,&response)!=0;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    if(sent && GetPropW(window,L"Win2kStyle.Frame") && info.rcTitleBar.bottom-info.rcTitleBar.top==MulDiv(19,(int)dpi,96))InterlockedExchange(&message_attached,1);
    InterlockedExchange(&message_seen,1);
    if(InterlockedCompareExchange(&message_accept,0,0)){
        HWND button=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)button);
        ULONGLONG deadline=GetTickCount64()+1500;
        while(IsWindow(window) && GetTickCount64()<deadline)Sleep(25);
        if(IsWindow(window)){PostMessageW(window,WM_CLOSE,0,0);Sleep(1000);if(IsWindow(window))ExitProcess(4);}
    }else PostMessageW(window,WM_CLOSE,0,0);
    return FALSE;
}
static DWORD WINAPI close_common(void *unused)
{
    (void)unused;ULONGLONG deadline=GetTickCount64()+3000;
    do{EnumWindows(find_common_window,0);if(InterlockedCompareExchange(&message_seen,0,0) || InterlockedCompareExchange(&common_done,0,0))return 0;Sleep(25);}while(GetTickCount64()<deadline);
    ExitProcess(4);return 1;
}

static UINT_PTR CALLBACK file_classic_hook(HWND window,UINT message,WPARAM wp,LPARAM lp)
{(void)window;(void)message;(void)wp;(void)lp;return 0;}

static UINT find_message;static unsigned find_terms;
static LRESULT CALLBACK find_owner_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==find_message){FINDREPLACEW *record=(FINDREPLACEW *)lp;if(record && (record->Flags&FR_DIALOGTERM))++find_terms;return 0;}
    return DefWindowProcW(window,message,wp,lp);
}

static COLORREF caption_pixel(HWND w)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(!GetWindowRect(w,&outer))return CLR_INVALID;
    HDC dc=GetWindowDC(w);if(!dc)return CLR_INVALID;
    COLORREF pixel=GetPixel(dc,info.rcTitleBar.left-outer.left+40,info.rcTitleBar.top-outer.top+2);
    ReleaseDC(w,dc);return pixel;
}
static BOOL classic_caption_pixel(COLORREF pixel)
{
    if(pixel==CLR_INVALID)return FALSE;
    BYTE red=GetRValue(pixel),green=GetGValue(pixel),blue=GetBValue(pixel);
    return (red==green && green==blue && red>=128 && red<=181) || (blue>red && blue>=90);
}
static BOOL blue_caption(HWND w)
{
    COLORREF pixel=caption_pixel(w);
    return pixel!=CLR_INVALID && GetBValue(pixel)>GetRValue(pixel);
}

static BOOL adjusted_menu_rect(HWND w,RECT *rect)
{
    MENUBARINFO bar={.cbSize=sizeof(bar)};TITLEBARINFOEX title={.cbSize=sizeof(title)};RECT outer;
    if(!GetMenuBarInfo(w,OBJID_MENU,0,&bar) || !GetWindowRect(w,&outer))return FALSE;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
    int border=(int)(title.rcTitleBar.top-outer.top);
    OffsetRect(rect,(int)(outer.left+border-bar.rcBar.left),(int)(title.rcTitleBar.bottom-bar.rcBar.top));return TRUE;
}

static void pump_messages(void)
{
    MSG message;
    while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
}

static BOOL click_caption(HWND w,unsigned index,WPARAM hit)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    POINT point={(info.rgrect[index].left+info.rgrect[index].right)/2,
                 (info.rgrect[index].top+info.rgrect[index].bottom)/2};
    SendMessageW(w,WM_NCLBUTTONDOWN,hit,MAKELPARAM(point.x,point.y));
    if(GetCapture()!=w)return FALSE;
    ScreenToClient(w,&point);
    SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y));
    pump_messages();return TRUE;
}

static uint64_t button_signature(HWND w,unsigned index)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(index>CCHILDREN_TITLEBAR || !GetWindowRect(w,&outer))return 0;
    RECT r=info.rgrect[index];OffsetRect(&r,-outer.left,-outer.top);
    HDC dc=GetWindowDC(w);if(!dc || r.right<=r.left || r.bottom<=r.top){if(dc)ReleaseDC(w,dc);return 0;}
    uint64_t value=14695981039346656037ULL;
    for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x){value^=GetPixel(dc,x,y);value*=1099511628211ULL;}
    ReleaseDC(w,dc);return value;
}

typedef struct { HWND window;BOOL foreground,input,pressed,cancelled,maximized,restored;unsigned gray,native;LRESULT hit;uintptr_t tracked;BOOL capture; } physical_probe;
static BOOL physical_click(POINT point)
{
    INPUT event={.type=INPUT_MOUSE};
    if(!SetCursorPos(point.x,point.y))return FALSE;
    event.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;BOOL down=SendInput(1,&event,sizeof(event))==1;
    Sleep(100);event.mi.dwFlags=MOUSEEVENTF_LEFTUP;BOOL up=SendInput(1,&event,sizeof(event))==1;
    return down && up;
}
typedef struct { HWND window;POINT first,second;BOOL input,toggle,closed_before_escape;LONG first_paints,second_paints; } menu_physical_probe;
static DWORD WINAPI menu_physical_thread(void *parameter)
{
    menu_physical_probe *probe=parameter;
    for(unsigned i=0;i<20 && GetForegroundWindow()!=probe->window;++i)Sleep(50);
    if(GetForegroundWindow()==probe->window){
        probe->input=physical_click(probe->first);Sleep(350);
        probe->first_paints=InterlockedCompareExchange(&menu_frame_paints,0,0);
        if(probe->toggle)probe->input=physical_click(probe->second) && probe->input;
        else probe->input=SetCursorPos(probe->second.x,probe->second.y) && probe->input;
        Sleep(350);
        probe->second_paints=InterlockedCompareExchange(&menu_frame_paints,0,0);
        probe->closed_before_escape=GetPropW(probe->window,L"Win2kStyle.Menu")==NULL;
        PostMessageW(probe->window,WM_KEYDOWN,VK_ESCAPE,1);Sleep(200);
    }
    PostMessageW(probe->window,WM_APP+1,0,0);return 0;
}
static DWORD WINAPI physical_thread(void *parameter)
{
    physical_probe *probe=parameter;HWND w=probe->window;
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(w,&outer);
    RECT button=info.rgrect[3];POINT center={(button.left+button.right)/2,(button.top+button.bottom)/2};
    probe->hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(center.x,center.y));
    INPUT event={.type=INPUT_MOUSE};event.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
    for(unsigned i=0;i<20 && GetForegroundWindow()!=w;++i)Sleep(50);
    probe->foreground=GetForegroundWindow()==w;
    if(!probe->foreground){PostMessageW(w,WM_APP+1,0,0);return 0;}
    BOOL positioned=probe->foreground && SetCursorPos(center.x,center.y);Sleep(100);
    probe->input=positioned && SendInput(1,&event,sizeof(event))==1;
    for(unsigned i=0;i<10 && (uintptr_t)GetPropW(w,L"Win2kStyle.CaptionButton")!=7;++i)Sleep(50);
    GUITHREADINFO thread={.cbSize=sizeof(thread)};
    probe->capture=GetGUIThreadInfo(GetWindowThreadProcessId(w,NULL),&thread) && thread.hwndCapture==w;
    probe->tracked=(uintptr_t)GetPropW(w,L"Win2kStyle.CaptionButton");
    probe->pressed=probe->capture;
    TITLEBARINFOEX down={.cbSize=sizeof(down)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&down);
    probe->pressed=probe->pressed && (down.rgstate[3]&STATE_SYSTEM_PRESSED)!=0;
    HDC dc=GetWindowDC(w);
    if(dc){OffsetRect(&button,-outer.left,-outer.top);
        for(int y=button.top;y<button.bottom;++y)for(int x=button.left;x<button.right;++x){
            COLORREF pixel=GetPixel(dc,x,y);
            if(pixel==RGB(212,208,200))++probe->gray;
            if(pixel==GetSysColor(COLOR_3DFACE))++probe->native;
        }
        ReleaseDC(w,dc);}
    SetCursorPos(outer.left+30,outer.top+100);Sleep(150);
    event.mi.dwFlags=MOUSEEVENTF_LEFTUP;
    BOOL released=SendInput(1,&event,sizeof(event))==1;
    probe->input=probe->input && released;
    Sleep(200);probe->cancelled=IsWindow(w) && !IsZoomed(w);
    probe->input=physical_click(center) && probe->input;Sleep(200);
    probe->maximized=IsWindow(w) && IsZoomed(w);
    if(probe->maximized){
        TITLEBARINFOEX zoom={.cbSize=sizeof(zoom)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&zoom);
        POINT restore={(zoom.rgrect[3].left+zoom.rgrect[3].right)/2,(zoom.rgrect[3].top+zoom.rgrect[3].bottom)/2};
        probe->input=physical_click(restore) && probe->input;Sleep(200);
        probe->restored=IsWindow(w) && !IsZoomed(w);
    }
    PostMessageW(w,WM_APP+1,0,0);return 0;
}

int wmain(int argc,WCHAR **argv)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    if(argc==2 && !lstrcmpW(argv[1],L"--dialog-frame")){
        struct { DLGTEMPLATE header; WORD menu,klass,title; } templ={0};
        templ.header.style=WS_POPUP|WS_CAPTION|WS_SYSMENU;templ.header.cx=160;templ.header.cy=80;
        BOOL modeless[2]={0},modal[2]={0},no_proc[2]={0},named[2]={0},named_modal[2]={0};
        for(int ansi=0;ansi<2;++ansi){
            HWND dialog=ansi?CreateDialogIndirectParamA(instance,(LPCDLGTEMPLATEA)&templ,NULL,frame_dialog_proc,0):
                CreateDialogIndirectParamW(instance,(LPCDLGTEMPLATEW)&templ,NULL,frame_dialog_proc,0);
            if(dialog){modeless[ansi]=GetPropW(dialog,L"Win2kStyle.Frame")!=NULL;DestroyWindow(dialog);}
            dialog=ansi?CreateDialogIndirectParamA(instance,(LPCDLGTEMPLATEA)&templ,NULL,NULL,0):
                CreateDialogIndirectParamW(instance,(LPCDLGTEMPLATEW)&templ,NULL,NULL,0);
            if(dialog){no_proc[ansi]=GetPropW(dialog,L"Win2kStyle.Frame")!=NULL;DestroyWindow(dialog);}
            dialog=ansi?CreateDialogParamA(instance,MAKEINTRESOURCEA(101),NULL,NULL,0):
                CreateDialogParamW(instance,MAKEINTRESOURCEW(101),NULL,NULL,0);
            if(dialog){named[ansi]=GetPropW(dialog,L"Win2kStyle.Frame")!=NULL;DestroyWindow(dialog);}
            dialog_frame_seen=FALSE;
            INT_PTR named_result=ansi?DialogBoxParamA(instance,MAKEINTRESOURCEA(101),NULL,frame_dialog_proc,0):
                DialogBoxParamW(instance,MAKEINTRESOURCEW(101),NULL,frame_dialog_proc,0);
            named_modal[ansi]=named_result==7 && dialog_frame_seen;
            dialog_frame_seen=FALSE;
            INT_PTR result=ansi?DialogBoxIndirectParamA(instance,(LPCDLGTEMPLATEA)&templ,NULL,frame_dialog_proc,0):
                DialogBoxIndirectParamW(instance,(LPCDLGTEMPLATEW)&templ,NULL,frame_dialog_proc,0);
            modal[ansi]=result==7 && dialog_frame_seen;
        }
        printf("WIN2K DIALOG FRAME bits=%u modeless=%d,%d modal=%d,%d no_proc=%d,%d named=%d,%d named_modal=%d,%d\n",(unsigned)(sizeof(void*)*8),modeless[0],modeless[1],modal[0],modal[1],no_proc[0],no_proc[1],named[0],named[1],named_modal[0],named_modal[1]);
        return modeless[0] && modeless[1] && modal[0] && modal[1] && no_proc[0] && no_proc[1] && named[0] && named[1] && named_modal[0] && named_modal[1]?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--message-frame")){
        BOOL all=TRUE;
        for(int i=0;i<6;++i){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_message,NULL,0,NULL);if(!thread)return 2;
            int result=0;MSGBOXPARAMSA a={.cbSize=sizeof(a),.hwndOwner=NULL,.lpszText="Text",.lpszCaption="Win2kMessageFixture",.dwStyle=MB_OK};
            MSGBOXPARAMSW w={.cbSize=sizeof(w),.hwndOwner=NULL,.lpszText=L"Text",.lpszCaption=L"Win2kMessageFixture",.dwStyle=MB_OK};
            switch(i){case 0:result=MessageBoxA(NULL,"Text","Win2kMessageFixture",MB_OK);break;
                case 1:result=MessageBoxW(NULL,L"Text",L"Win2kMessageFixture",MB_OK);break;
                case 2:result=MessageBoxExA(NULL,"Text","Win2kMessageFixture",MB_OK,0);break;
                case 3:result=MessageBoxExW(NULL,L"Text",L"Win2kMessageFixture",MB_OK,0);break;
                case 4:result=MessageBoxIndirectA(&a);break;default:result=MessageBoxIndirectW(&w);break;}
            DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL pass=wait==WAIT_OBJECT_0 && result==IDOK && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K MESSAGE bits=%u api=%d attached=%ld result=%d\n",(unsigned)(sizeof(void*)*8),i,(long)InterlockedCompareExchange(&message_attached,0,0),result);
            if(!pass)all=FALSE;
        }
        return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--print-ex-frame")){
        PRINTDLGW defaults={.lStructSize=sizeof(defaults),.Flags=PD_RETURNDEFAULT};
        BOOL available=PrintDlgW(&defaults);DWORD printer_error=CommDlgExtendedError();
        if(defaults.hDevMode)GlobalFree(defaults.hDevMode);
        if(defaults.hDevNames)GlobalFree(defaults.hDevNames);
        if(!available){
            if(printer_error==PDERR_NODEFAULTPRN || printer_error==PDERR_NODEVICES){puts("WIN2K PRINTEX SKIP no_default_printer");return 77;}
            printf("WIN2K PRINTEX ERROR default=%lu\n",(unsigned long)printer_error);return 3;
        }
        HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 3;
        WNDCLASSW owner_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Win2kPrintExFixture"};
        if(!RegisterClassW(&owner_class)){CoUninitialize();return 2;}
        common_owner=CreateWindowW(owner_class.lpszClassName,L"Print owner",WS_OVERLAPPEDWINDOW,0,0,320,200,NULL,NULL,instance,NULL);
        if(!common_owner){UnregisterClassW(owner_class.lpszClassName,instance);CoUninitialize();return 2;}
        BOOL all=TRUE;
        for(int ansi=0;ansi<2;++ansi){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);InterlockedExchange(&common_done,0);
            HANDLE thread=CreateThread(NULL,0,close_common,NULL,0,NULL);if(!thread){all=FALSE;break;}
            PRINTDLGEXA a={.lStructSize=sizeof(a),.hwndOwner=common_owner,.Flags=PD_NOPAGENUMS,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
            PRINTDLGEXW w={.lStructSize=sizeof(w),.hwndOwner=common_owner,.Flags=PD_NOPAGENUMS,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
            HRESULT result=ansi?PrintDlgExA(&a):PrintDlgExW(&w);DWORD action=ansi?a.dwResultAction:w.dwResultAction;
            InterlockedExchange(&common_done,1);DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL pass=wait==WAIT_OBJECT_0 && result==S_OK && action==PD_RESULT_CANCEL && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K PRINTEX bits=%u ansi=%d attached=%ld result=%08lx action=%lu\n",(unsigned)(sizeof(void*)*8),ansi,(long)InterlockedCompareExchange(&message_attached,0,0),(unsigned long)result,(unsigned long)action);
            if(a.hDevMode)GlobalFree(a.hDevMode);
            if(a.hDevNames)GlobalFree(a.hDevNames);
            if(a.hDC)DeleteDC(a.hDC);
            if(w.hDevMode)GlobalFree(w.hDevMode);
            if(w.hDevNames)GlobalFree(w.hDevNames);
            if(w.hDC)DeleteDC(w.hDC);
            if(!pass)all=FALSE;
        }
        DestroyWindow(common_owner);common_owner=NULL;UnregisterClassW(owner_class.lpszClassName,instance);CoUninitialize();return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--print-frame")){
        PRINTDLGW defaults={.lStructSize=sizeof(defaults),.Flags=PD_RETURNDEFAULT};
        BOOL available=PrintDlgW(&defaults);DWORD printer_error=CommDlgExtendedError();
        if(defaults.hDevMode)GlobalFree(defaults.hDevMode);
        if(defaults.hDevNames)GlobalFree(defaults.hDevNames);
        if(!available){
            if(printer_error==PDERR_NODEFAULTPRN || printer_error==PDERR_NODEVICES){puts("WIN2K PRINT SKIP no_default_printer");return 77;}
            printf("WIN2K PRINT ERROR default=%lu\n",(unsigned long)printer_error);return 3;
        }
        WNDCLASSW owner_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Win2kPrintFixture"};
        if(!RegisterClassW(&owner_class))return 2;
        common_owner=CreateWindowW(owner_class.lpszClassName,L"Print owner",WS_OVERLAPPEDWINDOW,0,0,320,200,NULL,NULL,instance,NULL);
        if(!common_owner)return 2;
        BOOL all=TRUE;
        for(int ansi=0;ansi<2;++ansi)for(int setup=0;setup<2;++setup){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_common,NULL,0,NULL);if(!thread)return 2;
            PRINTDLGA pa={.lStructSize=sizeof(pa),.hwndOwner=common_owner};PRINTDLGW pw={.lStructSize=sizeof(pw),.hwndOwner=common_owner};
            PAGESETUPDLGA sa={.lStructSize=sizeof(sa),.hwndOwner=common_owner};PAGESETUPDLGW sw={.lStructSize=sizeof(sw),.hwndOwner=common_owner};
            BOOL result=ansi?(setup?PageSetupDlgA(&sa):PrintDlgA(&pa)):(setup?PageSetupDlgW(&sw):PrintDlgW(&pw));
            DWORD error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL pass=wait==WAIT_OBJECT_0 && !result && error==0 && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K PRINT bits=%u ansi=%d setup=%d attached=%ld cancelled=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,setup,(long)InterlockedCompareExchange(&message_attached,0,0),!result,(unsigned long)error);
            if(pa.hDevMode)GlobalFree(pa.hDevMode);
            if(pa.hDevNames)GlobalFree(pa.hDevNames);
            if(pw.hDevMode)GlobalFree(pw.hDevMode);
            if(pw.hDevNames)GlobalFree(pw.hDevNames);
            if(sa.hDevMode)GlobalFree(sa.hDevMode);
            if(sa.hDevNames)GlobalFree(sa.hDevNames);
            if(sw.hDevMode)GlobalFree(sw.hDevMode);
            if(sw.hDevNames)GlobalFree(sw.hDevNames);
            if(!pass)all=FALSE;
        }
        DestroyWindow(common_owner);common_owner=NULL;UnregisterClassW(owner_class.lpszClassName,instance);return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--color-accept")){
        BOOL all=TRUE;InterlockedExchange(&message_accept,1);
        for(int ansi=0;ansi<2;++ansi){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_common,NULL,0,NULL);if(!thread)return 2;
            COLORREF colors[16]={0},expected=RGB(10,20,30);
            CHOOSECOLORA a={.lStructSize=sizeof(a),.rgbResult=expected,.lpCustColors=colors,.Flags=CC_RGBINIT};
            CHOOSECOLORW w={.lStructSize=sizeof(w),.rgbResult=expected,.lpCustColors=colors,.Flags=CC_RGBINIT};
            BOOL result=ansi?ChooseColorA(&a):ChooseColorW(&w);DWORD error=CommDlgExtendedError();
            DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);BOOL same=(ansi?a.rgbResult:w.rgbResult)==expected;
            BOOL pass=wait==WAIT_OBJECT_0 && result && !error && same && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K COLOR bits=%u ansi=%d attached=%ld result=%d same=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,(long)InterlockedCompareExchange(&message_attached,0,0),result,same,(unsigned long)error);
            if(!pass)all=FALSE;
        }
        InterlockedExchange(&message_accept,0);return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--color-font-frame")){
        BOOL all=TRUE;
        for(int ansi=0;ansi<2;++ansi)for(int font=0;font<2;++font){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_common,NULL,0,NULL);if(!thread)return 2;
            COLORREF colors[16]={0};LOGFONTA log_a={0};LOGFONTW log_w={0};
            CHOOSECOLORA ca={.lStructSize=sizeof(ca),.lpCustColors=colors,.Flags=CC_RGBINIT};
            CHOOSECOLORW cw={.lStructSize=sizeof(cw),.lpCustColors=colors,.Flags=CC_RGBINIT};
            CHOOSEFONTA fa={.lStructSize=sizeof(fa),.lpLogFont=&log_a,.Flags=CF_SCREENFONTS};
            CHOOSEFONTW fw={.lStructSize=sizeof(fw),.lpLogFont=&log_w,.Flags=CF_SCREENFONTS};
            BOOL result=ansi?(font?ChooseFontA(&fa):ChooseColorA(&ca)):(font?ChooseFontW(&fw):ChooseColorW(&cw));
            DWORD error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL pass=wait==WAIT_OBJECT_0 && !result && error==0 && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K COMMON bits=%u ansi=%d font=%d attached=%ld cancelled=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,font,(long)InterlockedCompareExchange(&message_attached,0,0),!result,(unsigned long)error);
            if(!pass)all=FALSE;
        }
        return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--file-select")){
        char path_a[260];WCHAR path_w[260];
        if(!GetModuleFileNameA(NULL,path_a,ARRAYSIZE(path_a)) || !GetModuleFileNameW(NULL,path_w,ARRAYSIZE(path_w)))return 2;
        BOOL all=TRUE;InterlockedExchange(&message_accept,1);
        for(int ansi=0;ansi<2;++ansi)for(int save=0;save<2;++save)for(int explorer=0;explorer<2;++explorer){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_message,NULL,0,NULL);if(!thread)return 2;
            char file_a[260];WCHAR file_w[260];lstrcpynA(file_a,path_a,ARRAYSIZE(file_a));lstrcpynW(file_w,path_w,ARRAYSIZE(file_w));
            DWORD flags=OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST|OFN_DONTADDTORECENT|(explorer?OFN_EXPLORER:OFN_ENABLEHOOK);
            OPENFILENAMEA a={.lStructSize=sizeof(a),.lpstrFilter="All\0*.*\0\0",.lpstrFile=file_a,.nMaxFile=ARRAYSIZE(file_a),.lpstrTitle="Win2kMessageFixture",.Flags=flags,.lpfnHook=file_classic_hook};
            OPENFILENAMEW w={.lStructSize=sizeof(w),.lpstrFilter=L"All\0*.*\0\0",.lpstrFile=file_w,.nMaxFile=ARRAYSIZE(file_w),.lpstrTitle=L"Win2kMessageFixture",.Flags=flags,.lpfnHook=file_classic_hook};
            BOOL result=ansi?(save?GetSaveFileNameA(&a):GetOpenFileNameA(&a)):(save?GetSaveFileNameW(&w):GetOpenFileNameW(&w));
            DWORD error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL same=ansi?!lstrcmpiA(file_a,path_a):!lstrcmpiW(file_w,path_w);
            BOOL pass=wait==WAIT_OBJECT_0 && result && error==0 && same && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K SELECT bits=%u ansi=%d save=%d explorer=%d attached=%ld result=%d same=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,save,explorer,(long)InterlockedCompareExchange(&message_attached,0,0),result,same,(unsigned long)error);
            if(!pass)all=FALSE;
        }
        InterlockedExchange(&message_accept,0);return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--file-frame")){
        BOOL all=TRUE;
        for(int ansi=0;ansi<2;++ansi)for(int save=0;save<2;++save)for(int explorer=0;explorer<2;++explorer){
            InterlockedExchange(&message_seen,0);InterlockedExchange(&message_attached,0);
            HANDLE thread=CreateThread(NULL,0,close_message,NULL,0,NULL);if(!thread)return 2;
            char file_a[260]={0};WCHAR file_w[260]={0};DWORD flags=OFN_NOCHANGEDIR|(explorer?OFN_EXPLORER:OFN_ENABLEHOOK);
            OPENFILENAMEA a={.lStructSize=sizeof(a),.lpstrFilter="All\0*.*\0\0",.lpstrFile=file_a,.nMaxFile=260,.lpstrTitle="Win2kMessageFixture",.Flags=flags,.lpfnHook=file_classic_hook};
            OPENFILENAMEW w={.lStructSize=sizeof(w),.lpstrFilter=L"All\0*.*\0\0",.lpstrFile=file_w,.nMaxFile=260,.lpstrTitle=L"Win2kMessageFixture",.Flags=flags,.lpfnHook=file_classic_hook};
            BOOL result=ansi?(save?GetSaveFileNameA(&a):GetOpenFileNameA(&a)):(save?GetSaveFileNameW(&w):GetOpenFileNameW(&w));
            DWORD error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(thread,1000);CloseHandle(thread);
            BOOL pass=wait==WAIT_OBJECT_0 && !result && error==0 && InterlockedCompareExchange(&message_seen,0,0) && InterlockedCompareExchange(&message_attached,0,0);
            printf("WIN2K FILE bits=%u ansi=%d save=%d explorer=%d attached=%ld cancelled=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,save,explorer,(long)InterlockedCompareExchange(&message_attached,0,0),!result,(unsigned long)error);
            if(!pass)all=FALSE;
        }
        return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--find-frame")){
        find_message=RegisterWindowMessageW(FINDMSGSTRINGW);
        WNDCLASSW owner_class={.lpfnWndProc=find_owner_proc,.hInstance=instance,.lpszClassName=L"Win2kFindFixture"};
        if(!find_message || !RegisterClassW(&owner_class))return 2;
        HWND owner=CreateWindowW(owner_class.lpszClassName,L"Find owner",WS_OVERLAPPEDWINDOW,0,0,320,200,NULL,NULL,instance,NULL);
        if(!owner)return 2;
        BOOL all=TRUE;
        for(int ansi=0;ansi<2;++ansi)for(int replace=0;replace<2;++replace){
            char find_a[64]="needle",replacement_a[64]="replacement";WCHAR find_w[64]=L"needle",replacement_w[64]=L"replacement";
            FINDREPLACEA a={.lStructSize=sizeof(a),.hwndOwner=owner,.lpstrFindWhat=find_a,.wFindWhatLen=64,.lpstrReplaceWith=replacement_a,.wReplaceWithLen=64,.Flags=FR_DOWN};
            FINDREPLACEW w={.lStructSize=sizeof(w),.hwndOwner=owner,.lpstrFindWhat=find_w,.wFindWhatLen=64,.lpstrReplaceWith=replacement_w,.wReplaceWithLen=64,.Flags=FR_DOWN};
            find_terms=0;HWND dialog=ansi?(replace?ReplaceTextA(&a):FindTextA(&a)):(replace?ReplaceTextW(&w):FindTextW(&w));
            BOOL attached=dialog && GetPropW(dialog,L"Win2kStyle.Frame")!=NULL;
            if(dialog){SendMessageW(dialog,WM_CLOSE,0,0);pump_messages();}
            BOOL gone=dialog && !IsWindow(dialog);BOOL pass=attached && gone && find_terms==1;
            printf("WIN2K FIND bits=%u ansi=%d replace=%d attached=%d gone=%d terms=%u\n",(unsigned)(sizeof(void*)*8),ansi,replace,attached,gone,find_terms);
            if(!pass)all=FALSE;
        }
        DestroyWindow(owner);UnregisterClassW(owner_class.lpszClassName,instance);return all?0:3;
    }
    BOOL no_icon=argc==2 && !lstrcmpW(argv[1],L"--no-icon");
    WNDCLASSW wc={.lpfnWndProc=procedure,.hInstance=instance,.hIcon=no_icon?NULL:LoadIconW(NULL,MAKEINTRESOURCEW(32515)),
        .hbrBackground=(HBRUSH)(COLOR_WINDOW+1),.lpszClassName=L"Win2kFrameFixture"};
    if(!RegisterClassW(&wc))return 1;
    if(argc==2 && !lstrcmpW(argv[1],L"--startup-size")){
        startup_probe=TRUE;
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Startup size",WS_OVERLAPPEDWINDOW,
                               80,80,420,260,NULL,NULL,instance,NULL);
        startup_ready=TRUE;
        if(!w)return 2;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE))DispatchMessageW(&message);
        BOOL ok=startup_early_size==0 && startup_late_size>0;
        printf("WIN2K STARTUP SIZE bits=%u early=%u late=%u\n",(unsigned)(sizeof(void*)*8),startup_early_size,startup_late_size);
        DestroyWindow(w);return ok?0:3;
    }
    if(argc==2 && (!lstrcmpW(argv[1],L"--menu-physical") || !lstrcmpW(argv[1],L"--menu-physical-toggle"))){
        BOOL toggle=!lstrcmpW(argv[1],L"--menu-physical-toggle");
        HMENU menu=CreateMenu(),first=CreatePopupMenu(),second=CreatePopupMenu();if(!menu || !first || !second)return 2;
        AppendMenuW(first,MF_STRING,101,L"First item");AppendMenuW(second,MF_STRING,102,L"Second item");
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)first,L"File");AppendMenuW(menu,MF_POPUP,(UINT_PTR)second,L"Help");
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Menu physical",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,menu,instance,NULL);
        if(!w){DestroyMenu(menu);return 3;}
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SetForegroundWindow(w);
        RECT a,b;BOOL measured=GetMenuItemRect(w,menu,0,&a) && GetMenuItemRect(w,menu,1,&b);
        adjusted_menu_rect(w,&a);adjusted_menu_rect(w,&b);
        menu_physical_probe probe={.window=w,.first={(a.left+a.right)/2,(a.top+a.bottom)/2},.second={(b.left+b.right)/2,(b.top+b.bottom)/2},.toggle=toggle};
        if(toggle)probe.second=probe.first;
        menu_scenario=5;HANDLE worker=CreateThread(NULL,0,menu_physical_thread,&probe,0,NULL);
        if(!worker){DestroyWindow(w);DestroyMenu(menu);return 3;}
        MSG message;while(GetMessageW(&message,NULL,0,0)>0){if(message.message==WM_APP+1)break;TranslateMessage(&message);DispatchMessageW(&message);}
        WaitForSingleObject(worker,3000);CloseHandle(worker);menu_scenario=0;
        BOOL ok=measured && probe.input && menu_entries==(toggle?1u:2u) && menu_exits==menu_entries &&
                (!toggle || probe.closed_before_escape) &&
                probe.first_paints==probe.second_paints && GetCapture()==NULL;
        printf("WIN2K MENU PHYSICAL bits=%u toggle=%d input=%d entries=%u exits=%u frame_paints=%ld,%ld,%ld\n",
               (unsigned)(sizeof(void*)*8),toggle,probe.input,menu_entries,menu_exits,probe.first_paints,probe.second_paints,
               InterlockedCompareExchange(&menu_frame_paints,0,0));
        DestroyWindow(w);DestroyMenu(menu);return ok?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--menu-special")){
        HMENU menu=CreateMenu(),first=CreatePopupMenu(),second=CreatePopupMenu();if(!menu || !first || !second)return 2;
        AppendMenuW(first,MF_STRING,101,L"First item");AppendMenuW(second,MF_STRING,102,L"Second item");
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)first,L"File");AppendMenuW(menu,MF_POPUP|MF_MENUBREAK,(UINT_PTR)second,L"Help");
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Multiple rows",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,menu,instance,NULL);
        if(!w){DestroyMenu(menu);return 3;}
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        TITLEBARINFOEX title={.cbSize=sizeof(title)};MENUBARINFO bar={.cbSize=sizeof(bar)};
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);BOOL got=GetMenuBarInfo(w,OBJID_MENU,0,&bar);
        POINT client={0,0};ClientToScreen(w,&client);UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
        BOOL ok=got && title.rcTitleBar.bottom-title.rcTitleBar.top==GetSystemMetricsForDpi(SM_CYCAPTION,dpi) &&
            bar.rcBar.top>=title.rcTitleBar.bottom && bar.rcBar.bottom<=client.y;
        printf("WIN2K SPECIAL MENU bits=%u native=%d title=%ld\n",(unsigned)(sizeof(void*)*8),ok,
               title.rcTitleBar.bottom-title.rcTitleBar.top);
        DestroyWindow(w);DestroyMenu(menu);return ok?0:3;
    }
    if(argc==3 && !lstrcmpW(argv[1],L"--real-app")){
        WCHAR command[2048];if(swprintf(command,ARRAYSIZE(command),L"\"%ls\"",argv[2])<0)return 2;
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
        if(!CreateProcessW(argv[2],command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&process))return 3;
        app_pid=process.dwProcessId;ULONGLONG deadline=GetTickCount64()+5000;
        do{EnumWindows(find_app_window,0);if(app_window)break;Sleep(50);}while(GetTickCount64()<deadline);
        BOOL ok=FALSE;
        if(app_window){
            TITLEBARINFOEX title={.cbSize=sizeof(title)};RECT outer,item;
            SendMessageW(app_window,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);GetWindowRect(app_window,&outer);
            printf("WIN2K REAL BUTTONS title=%ld,%ld,%ld,%ld min=%ld,%ld,%ld,%ld max=%ld,%ld,%ld,%ld close=%ld,%ld,%ld,%ld\n",
                   title.rcTitleBar.left-outer.left,title.rcTitleBar.top-outer.top,title.rcTitleBar.right-outer.left,title.rcTitleBar.bottom-outer.top,
                   title.rgrect[2].left-outer.left,title.rgrect[2].top-outer.top,title.rgrect[2].right-outer.left,title.rgrect[2].bottom-outer.top,
                   title.rgrect[3].left-outer.left,title.rgrect[3].top-outer.top,title.rgrect[3].right-outer.left,title.rgrect[3].bottom-outer.top,
                   title.rgrect[5].left-outer.left,title.rgrect[5].top-outer.top,title.rgrect[5].right-outer.left,title.rgrect[5].bottom-outer.top);
            HMENU menu=GetMenu(app_window);UINT dpi=GetDpiForWindow(app_window);if(!dpi)dpi=96;
            BOOL measured=menu && GetMenuItemRect(app_window,menu,0,&item);
            if(measured)measured=adjusted_menu_rect(app_window,&item);
            LRESULT hit=measured?SendMessageW(app_window,WM_NCHITTEST,0,MAKELPARAM((item.left+item.right)/2,(item.top+item.bottom)/2)):0;
            BOOL opened=FALSE;
            if(measured && hit==HTBORDER){
                HANDLE cancel=CreateThread(NULL,0,cancel_app_menu,NULL,0,NULL);
                DWORD_PTR reply=0;
                BOOL sent=cancel && SendMessageTimeoutW(app_window,WM_NCLBUTTONDOWN,HTBORDER,
                    MAKELPARAM((item.left+item.right)/2,(item.top+item.bottom)/2),SMTO_ABORTIFHUNG,4000,&reply);
                if(cancel){WaitForSingleObject(cancel,3000);CloseHandle(cancel);}
                opened=sent && InterlockedCompareExchange(&app_popup_seen,0,0)!=0;
            }
            ok=measured && title.rcTitleBar.bottom-title.rcTitleBar.top==MulDiv(19,(int)dpi,96) && hit==HTBORDER && opened;
            printf("WIN2K REAL APP bits=%u title=%ld menu=%d hit=%lld opened=%d\n",(unsigned)(sizeof(void*)*8),
                   title.rcTitleBar.bottom-title.rcTitleBar.top,measured,(long long)hit,opened);
            DWORD_PTR activation_reply=0;
            BOOL active_sent=SendMessageTimeoutW(app_window,WM_NCACTIVATE,TRUE,0,SMTO_ABORTIFHUNG,2000,&activation_reply)!=0;
            COLORREF active_pixel=active_sent?caption_pixel(app_window):CLR_INVALID;
            BOOL inactive_sent=SendMessageTimeoutW(app_window,WM_NCACTIVATE,FALSE,0,SMTO_ABORTIFHUNG,2000,&activation_reply)!=0;
            COLORREF inactive_pixel=inactive_sent?caption_pixel(app_window):CLR_INVALID;
            BOOL active_color=active_pixel!=CLR_INVALID && GetBValue(active_pixel)>GetRValue(active_pixel);
            BOOL inactive_color=inactive_pixel!=CLR_INVALID && GetRValue(inactive_pixel)==GetGValue(inactive_pixel) &&
                                GetGValue(inactive_pixel)==GetBValue(inactive_pixel) && GetRValue(inactive_pixel)>=128 && GetRValue(inactive_pixel)<=181;
            printf("WIN2K REAL ACTIVATION active=%d inactive=%d pixels=%06lx,%06lx\n",active_color,inactive_color,
                   (unsigned long)active_pixel,(unsigned long)inactive_pixel);
            ok=ok && active_color && inactive_color;
            BOOL supports_max=(GetWindowLongPtrW(app_window,GWL_STYLE)&WS_MAXIMIZEBOX)!=0;
            BOOL maximized=FALSE,restored=FALSE,caption=FALSE;COLORREF max_pixel=CLR_INVALID,restore_pixel=CLR_INVALID;
            if(supports_max){DWORD_PTR ignored=0;
                BOOL sent=SendMessageTimeoutW(app_window,WM_SYSCOMMAND,SC_MAXIMIZE,0,SMTO_ABORTIFHUNG,2000,&ignored)!=0;
                maximized=sent && IsZoomed(app_window);
                TITLEBARINFOEX zoom={.cbSize=sizeof(zoom)};
                if(maximized && SendMessageTimeoutW(app_window,WM_GETTITLEBARINFOEX,0,(LPARAM)&zoom,SMTO_ABORTIFHUNG,2000,&ignored))
                    caption=zoom.rcTitleBar.bottom>zoom.rcTitleBar.top && GetPropW(app_window,L"Win2kStyle.Frame")!=NULL;
                if(maximized)max_pixel=caption_pixel(app_window);
                sent=SendMessageTimeoutW(app_window,WM_SYSCOMMAND,SC_RESTORE,0,SMTO_ABORTIFHUNG,2000,&ignored)!=0;
                restored=sent && !IsZoomed(app_window);if(restored)restore_pixel=caption_pixel(app_window);
                ok=ok && maximized && restored && caption && classic_caption_pixel(max_pixel) && classic_caption_pixel(restore_pixel);
            }
            printf("WIN2K REAL GEOMETRY supported=%d maximized=%d restored=%d caption=%d pixels=%06lx,%06lx\n",supports_max,maximized,restored,caption,(unsigned long)max_pixel,(unsigned long)restore_pixel);
            RECT before,after;BOOL sized=(GetWindowLongPtrW(app_window,GWL_STYLE)&WS_THICKFRAME)!=0;
            BOOL read=GetWindowRect(app_window,&before),moved=FALSE,resized=FALSE,drawn=FALSE;
            if(read){int width=before.right-before.left,height=before.bottom-before.top;
                UINT flags=SWP_NOZORDER|SWP_NOACTIVATE|(sized?0:SWP_NOSIZE);
                BOOL set=SetWindowPos(app_window,NULL,before.left+22,before.top+16,width+24,height+18,flags);
                if(set && GetWindowRect(app_window,&after)){
                    moved=after.left!=before.left || after.top!=before.top;
                    resized=(after.right-after.left!=width || after.bottom-after.top!=height);
                    drawn=classic_caption_pixel(caption_pixel(app_window));
                }
            }
            ok=ok && moved && drawn && (!sized || resized);
            printf("WIN2K REAL MOVE moved=%d size_supported=%d resized=%d caption=%d\n",moved,sized,resized,drawn);
            PostMessageW(app_window,WM_CLOSE,0,0);
        }
        if(WaitForSingleObject(process.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,1);ok=FALSE;}
        CloseHandle(process.hThread);CloseHandle(process.hProcess);return ok?0:4;
    }
    if(argc==2 && (!lstrcmpW(argv[1],L"--menu-hover") || !lstrcmpW(argv[1],L"--menu-arrow") || !lstrcmpW(argv[1],L"--menu-nested"))){
        BOOL arrow=!lstrcmpW(argv[1],L"--menu-arrow");
        BOOL nested=!lstrcmpW(argv[1],L"--menu-nested");
        HMENU menu=CreateMenu(),first=CreatePopupMenu(),second=CreatePopupMenu();if(!menu || !first || !second)return 2;
        if(nested){HMENU child=CreatePopupMenu();if(!child)return 2;AppendMenuW(child,MF_STRING,101,L"Nested item");AppendMenuW(first,MF_POPUP,(UINT_PTR)child,L"Nested");}
        else AppendMenuW(first,MF_STRING,101,L"First item");
        AppendMenuW(second,MF_STRING,102,L"Second item");
        AppendMenuW(menu,MF_POPUP,(UINT_PTR)first,L"File");AppendMenuW(menu,MF_POPUP,(UINT_PTR)second,L"Help");
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Menu hover",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,menu,instance,NULL);
        if(!w){DestroyMenu(menu);return 3;}
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SetForegroundWindow(w);
        RECT a,b;BOOL measured=GetMenuItemRect(w,menu,0,&a) && GetMenuItemRect(w,menu,1,&b);
        adjusted_menu_rect(w,&a);adjusted_menu_rect(w,&b);
        POINT click={(a.left+a.right)/2,(a.top+a.bottom)/2};menu_hover_point=(POINT){(b.left+b.right)/2,(b.top+b.bottom)/2};
        menu_scenario=nested?6:arrow?4:3;menu_idle=0;SetTimer(w,77,2500,NULL);
        SendMessageW(w,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM(click.x,click.y));
        KillTimer(w,77);pump_messages();menu_scenario=0;
        BOOL ok=measured && !menu_timeout && menu_entries==(nested?1u:2u) && menu_exits==menu_entries &&
            (!nested || menu_popup_inits>=2) && GetCapture()==NULL;
        printf("WIN2K MENU HOVER bits=%u arrow=%d nested=%d entries=%u exits=%u popups=%u timeout=%d\n",
               (unsigned)(sizeof(void*)*8),arrow,nested,menu_entries,menu_exits,menu_popup_inits,menu_timeout);
        DestroyWindow(w);DestroyMenu(menu);return ok?0:3;
    }
    if(argc==2 && (!lstrcmpW(argv[1],L"--menu-input") || !lstrcmpW(argv[1],L"--menu-key") || !lstrcmpW(argv[1],L"--menu-f10") || !lstrcmpW(argv[1],L"--menu-f10-select"))){
        BOOL keyboard=!lstrcmpW(argv[1],L"--menu-key");
        BOOL select=!lstrcmpW(argv[1],L"--menu-f10-select");
        BOOL f10=select || !lstrcmpW(argv[1],L"--menu-f10");
        HMENU menu=CreateMenu(),popup=CreatePopupMenu();if(!menu || !popup)return 2;
        AppendMenuW(popup,MF_STRING,101,L"Item");AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"&File");
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Menu input",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,menu,instance,NULL);
        if(!w){DestroyMenu(menu);return 3;}
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SetForegroundWindow(w);
        HWND edit=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP,10,10,120,20,w,NULL,instance,NULL);
        if(edit)SetFocus(edit);
        RECT item={0};BOOL measured=GetMenuItemRect(w,menu,0,&item);
        POINT native={(item.left+item.right)/2,(item.top+item.bottom)/2};
        BOOL native_neutral=measured && SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(native.x,native.y))!=HTMENU;
        adjusted_menu_rect(w,&item);POINT click={(item.left+item.right)/2,(item.top+item.bottom)/2};
        BOOL hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(click.x,click.y))==HTBORDER;
        BOOL focus_ok=TRUE;
        for(menu_scenario=select?1:(keyboard||f10)?2:1;menu_scenario<=(select?1u:2u);++menu_scenario){
            menu_idle=0;SetTimer(w,77,2500,NULL);
            if(keyboard)SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L'f');
            else if(f10){
                SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,0);focus_ok=edit && GetFocus()==w;
                SendMessageW(w,WM_KEYDOWN,VK_DOWN,1);
                pump_messages();
                if(!select)SendMessageW(w,WM_KEYDOWN,VK_ESCAPE,1);
                focus_ok=focus_ok && GetFocus()==edit;
            }
            else SendMessageW(w,WM_NCLBUTTONDOWN,HTBORDER,MAKELPARAM(click.x,click.y));
            KillTimer(w,77);pump_messages();
        }
        menu_scenario=0;
        BOOL ok=measured && native_neutral && hit && focus_ok && !menu_timeout && menu_entries==((keyboard||f10)?1u:2u) && menu_exits==menu_entries && menu_commands==(select?1u:(keyboard||f10)?0u:1u) && GetCapture()==NULL;
        printf("WIN2K MENU INPUT bits=%u keyboard=%d f10=%d focus=%d hit=%d entries=%u exits=%u commands=%u timeout=%d native_neutral=%d\n",
               (unsigned)(sizeof(void*)*8),keyboard,f10,focus_ok,hit,menu_entries,menu_exits,menu_commands,menu_timeout,native_neutral);
        DestroyWindow(w);DestroyMenu(menu);return ok?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--scroll-geometry")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Scroll geometry",WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        SCROLLINFO range={.cbSize=sizeof(range),.fMask=SIF_RANGE|SIF_PAGE,.nMin=0,.nMax=100,.nPage=10};
        SetScrollInfo(w,SB_HORZ,&range,FALSE);SetScrollInfo(w,SB_VERT,&range,FALSE);
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        RECT outer,client;SCROLLBARINFO vert={.cbSize=sizeof(vert)},horz={.cbSize=sizeof(horz)};
        BOOL got=GetWindowRect(w,&outer) && GetClientRect(w,&client) &&
            GetScrollBarInfo(w,OBJID_VSCROLL,&vert) && GetScrollBarInfo(w,OBJID_HSCROLL,&horz);
        MapWindowPoints(w,NULL,(POINT *)&client,2);
        BOOL separate=got && vert.rcScrollBar.left>=client.right && horz.rcScrollBar.top>=client.bottom &&
            vert.rcScrollBar.right<=outer.right && horz.rcScrollBar.bottom<=outer.bottom;
        POINT vp={(vert.rcScrollBar.left+vert.rcScrollBar.right)/2,vert.rcScrollBar.top+4};
        POINT hp={horz.rcScrollBar.left+4,(horz.rcScrollBar.top+horz.rcScrollBar.bottom)/2};
        BOOL hits=got && SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(vp.x,vp.y))==HTVSCROLL &&
            SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(hp.x,hp.y))==HTHSCROLL;
        printf("WIN2K SCROLL GEOMETRY bits=%u got=%d separate=%d hits=%d client=%ld,%ld,%ld,%ld vertical=%ld,%ld,%ld,%ld horizontal=%ld,%ld,%ld,%ld\n",
               (unsigned)(sizeof(void*)*8),got,separate,hits,client.left-outer.left,client.top-outer.top,client.right-outer.left,client.bottom-outer.top,
               vert.rcScrollBar.left-outer.left,vert.rcScrollBar.top-outer.top,vert.rcScrollBar.right-outer.left,vert.rcScrollBar.bottom-outer.top,
               horz.rcScrollBar.left-outer.left,horz.rcScrollBar.top-outer.top,horz.rcScrollBar.right-outer.left,horz.rcScrollBar.bottom-outer.top);
        DestroyWindow(w);return separate && hits?0:3;
    }
    if(argc==4 && !lstrcmpW(argv[1],L"--frame-bounds")){
        BOOL all=TRUE;
        int resizable=argv[2][0]-L'0',with_menu=argv[3][0]-L'0';
        if((resizable!=0 && resizable!=1) || (with_menu!=0 && with_menu!=1) || argv[2][1] || argv[3][1])return 2;
        {
            HMENU menu=NULL,popup=NULL;
            if(with_menu){menu=CreateMenu();popup=CreatePopupMenu();if(!menu || !popup)return 2;
                AppendMenuW(popup,MF_STRING,101,L"Item");AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"File");}
            DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|(resizable?WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX:0);
            HWND w=CreateWindowExW(0,wc.lpszClassName,L"Border metrics",style,80,80,420,260,NULL,menu,instance,NULL);
            if(!w){if(menu)DestroyMenu(menu);return 2;}
            ShowWindow(w,SW_SHOW);UpdateWindow(w);
            SendMessageW(w,WM_NCACTIVATE,TRUE,0);
            RECT outer,client;POINT origin={0,0};TITLEBARINFOEX title={.cbSize=sizeof(title)};
            BOOL got=GetWindowRect(w,&outer) && GetClientRect(w,&client) && ClientToScreen(w,&origin);
            SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
            int left=origin.x-outer.left,right=outer.right-origin.x-client.right,bottom=outer.bottom-origin.y-client.bottom;
            int top=(int)(title.rcTitleBar.top-outer.top);
            MENUBARINFO system={.cbSize=sizeof(system)},bar={.cbSize=sizeof(bar)};
            BOOL has_system=GetMenuBarInfo(w,OBJID_SYSMENU,0,&system);
            BOOL has_menu=with_menu && GetMenuBarInfo(w,OBJID_MENU,0,&bar);
            LRESULT edge=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(outer.left+1,(outer.top+outer.bottom)/2));
            unsigned menu_spill=0;
            if(with_menu && resizable){
                HDC dc=GetWindowDC(w);
                if(dc){
                    for(int y=(int)(title.rcTitleBar.bottom-outer.top);y<(int)(title.rcTitleBar.bottom-outer.top)+3;++y)
                        for(int x=left;x<left+20;++x)if(GetPixel(dc,x,y)!=RGB(212,208,200))++menu_spill;
                    ReleaseDC(w,dc);
                }else menu_spill=UINT_MAX;
            }
            printf("WIN2K FRAME BOUNDS bits=%u resize=%d menu=%d top=%d left=%d right=%d bottom=%d edge=%lld icon=%ld,%ld bar=%ld,%ld spill=%u\n",
                   (unsigned)(sizeof(void*)*8),resizable,with_menu,top,left,right,bottom,(long long)edge,
                   has_system?system.rcBar.left-outer.left:-1,has_system?system.rcBar.top-outer.top:-1,
                   has_menu?bar.rcBar.left-outer.left:-1,has_menu?bar.rcBar.top-outer.top:-1,menu_spill);
            UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
            int expected_x=expected_physical_border(w,resizable?4:3,TRUE);
            int expected_y=expected_physical_border(w,resizable?4:3,FALSE);
            BOOL corners=TRUE,edge_pixels=TRUE;
            if(resizable){
                const POINT points[]={{outer.left+1,outer.top+1},{outer.right-2,outer.top+1},
                                      {outer.left+1,outer.bottom-2},{outer.right-2,outer.bottom-2}};
                const LRESULT hits[]={HTTOPLEFT,HTTOPRIGHT,HTBOTTOMLEFT,HTBOTTOMRIGHT};
                for(unsigned i=0;i<ARRAYSIZE(points);++i)
                    if(SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(points[i].x,points[i].y))!=hits[i])corners=FALSE;
                if(dpi==96){
                    HDC dc=GetWindowDC(w);int width=outer.right-outer.left,height=outer.bottom-outer.top;
                    if(!dc)edge_pixels=FALSE;
                    else{
                        COLORREF face=RGB(212,208,200),light=RGB(255,255,255),shadow=RGB(128,128,128),dark=RGB(0,0,0);
                        edge_pixels=GetPixel(dc,0,height/2)==face && GetPixel(dc,1,height/2)==light &&
                            GetPixel(dc,2,height/2)==face && GetPixel(dc,width/2,0)==face &&
                            GetPixel(dc,width/2,1)==light && GetPixel(dc,width/2,2)==face &&
                            GetPixel(dc,width-1,height/2)==dark && GetPixel(dc,width-2,height/2)==shadow &&
                            GetPixel(dc,width/2,height-1)==dark && GetPixel(dc,width/2,height-2)==shadow;
                        ReleaseDC(w,dc);
                    }
                }
            }
            all=all && got && GetPropW(w,L"Win2kStyle.Frame")!=NULL && top==expected_y && left==expected_x && right==expected_x && bottom==expected_y &&
                edge==(resizable?HTLEFT:HTBORDER) && corners && edge_pixels && !menu_spill;
            DestroyWindow(w);if(menu)DestroyMenu(menu);
        }
        return all?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--activation")){
        HWND first=CreateWindowExW(0,wc.lpszClassName,L"First active",WS_OVERLAPPEDWINDOW,
                                  80,80,420,260,NULL,NULL,instance,NULL);
        HWND second=CreateWindowExW(0,wc.lpszClassName,L"Second active",WS_OVERLAPPEDWINDOW,
                                   520,80,420,260,NULL,NULL,instance,NULL);
        if(!first || !second){if(first)DestroyWindow(first);if(second)DestroyWindow(second);return 2;}
        ShowWindow(first,SW_SHOW);ShowWindow(second,SW_SHOW);UpdateWindow(first);UpdateWindow(second);
        if(!GetForegroundWindow()){
            puts("WIN2K ACTIVATION SKIP no_foreground");DestroyWindow(second);DestroyWindow(first);return 77;
        }
        SetActiveWindow(first);BOOL first_active=GetActiveWindow()==first;
        COLORREF first_blue=caption_pixel(first),second_gray=caption_pixel(second);
        SetActiveWindow(second);BOOL second_active=GetActiveWindow()==second;
        COLORREF first_gray=caption_pixel(first),second_blue=caption_pixel(second);
        BOOL colors=GetBValue(first_blue)>GetRValue(first_blue) && GetRValue(second_gray)==GetGValue(second_gray) &&
            GetGValue(second_gray)==GetBValue(second_gray) && GetRValue(first_gray)==GetGValue(first_gray) &&
            GetGValue(first_gray)==GetBValue(first_gray) && GetBValue(second_blue)>GetRValue(second_blue) &&
            first_blue!=CLR_INVALID && second_gray!=CLR_INVALID && first_gray!=CLR_INVALID && second_blue!=CLR_INVALID;
        printf("WIN2K ACTIVATION bits=%u first=%d second=%d colors=%d pixels=%06lx,%06lx,%06lx,%06lx\n",(unsigned)(sizeof(void*)*8),first_active,second_active,colors,
               (unsigned long)first_blue,(unsigned long)second_gray,(unsigned long)first_gray,(unsigned long)second_blue);
        DestroyWindow(second);DestroyWindow(first);return first_active && second_active && colors?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--caption-added")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption added",WS_POPUP,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        BOOL initially_unattached=GetPropW(w,L"Win2kStyle.Frame")==NULL;
        SetWindowLongPtrW(w,GWL_STYLE,WS_OVERLAPPEDWINDOW|WS_VISIBLE);
        BOOL changed=SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        BOOL attached=GetPropW(w,L"Win2kStyle.Frame")!=NULL;
        BOOL painted=blue_caption(w);
        printf("WIN2K CAPTION ADDED bits=%u initial=%d changed=%d attached=%d painted=%d\n",
               (unsigned)(sizeof(void*)*8),initially_unattached,changed,attached,painted);
        DestroyWindow(w);return initially_unattached && changed && attached && painted?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--style-transition")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Style transition",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        BOOL initial=GetPropW(w,L"Win2kStyle.Frame") && blue_caption(w);
        LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
        SetWindowLongPtrW(w,GWL_STYLE,style&~WS_CAPTION);
        BOOL changed=SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        RECT client;POINT origin={0,0};BOOL valid=GetClientRect(w,&client) && ClientToScreen(w,&origin);
        BOOL client_hit=valid && SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(origin.x+10,origin.y+10))==HTCLIENT;
        SetWindowLongPtrW(w,GWL_STYLE,style);
        BOOL reset=SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        BOOL restored=GetPropW(w,L"Win2kStyle.Frame") && blue_caption(w);
        printf("WIN2K STYLE TRANSITION bits=%u initial=%d changed=%d client_hit=%d reset=%d restored=%d\n",
               (unsigned)(sizeof(void*)*8),initial,changed,client_hit,reset,restored);
        DestroyWindow(w);return initial && changed && client_hit && reset && restored?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--menu-transition")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Menu transition",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        HMENU menu=CreateMenu(),popup=CreatePopupMenu();
        if(!w || !menu || !popup){if(w)DestroyWindow(w);if(menu)DestroyMenu(menu);if(popup)DestroyMenu(popup);return 2;}
        AppendMenuW(popup,MF_STRING,101,L"Item");AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"File");
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        TITLEBARINFOEX before={.cbSize=sizeof(before)},added={.cbSize=sizeof(added)},removed={.cbSize=sizeof(removed)};
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&before);
        BOOL attached=SetMenu(w,menu);DrawMenuBar(w);
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&added);
        MENUBARINFO bar={.cbSize=sizeof(bar)};BOOL has_bar=GetMenuBarInfo(w,OBJID_MENU,0,&bar);
        if(has_bar)adjusted_menu_rect(w,&bar.rcBar);
        POINT client={0,0};ClientToScreen(w,&client);
        BOOL menu_ok=attached && has_bar && bar.rcBar.top>=added.rcTitleBar.bottom && bar.rcBar.bottom<=client.y;
        BOOL detached=SetMenu(w,NULL);DrawMenuBar(w);
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&removed);client=(POINT){0,0};ClientToScreen(w,&client);
        BOOL bare_ok=detached && removed.rcTitleBar.bottom-removed.rcTitleBar.top==before.rcTitleBar.bottom-before.rcTitleBar.top &&
            removed.rcTitleBar.bottom<=client.y && SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(client.x+20,client.y+1))==HTCLIENT;
        printf("WIN2K MENU TRANSITION bits=%u menu=%d bare=%d before=%ld added=%ld removed=%ld\n",
               (unsigned)(sizeof(void*)*8),menu_ok,bare_ok,before.rcTitleBar.bottom-before.rcTitleBar.top,
               added.rcTitleBar.bottom-added.rcTitleBar.top,removed.rcTitleBar.bottom-removed.rcTitleBar.top);
        DestroyWindow(w);DestroyMenu(menu);return menu_ok && bare_ok?0:3;
    }
    if(argc==2 && (!lstrcmpW(argv[1],L"--geometry") || !lstrcmpW(argv[1],L"--geometry-bare"))){
        BOOL bare=!lstrcmpW(argv[1],L"--geometry-bare");
        HMENU menu=NULL,popup=NULL;
        if(!bare){
            menu=CreateMenu();popup=CreatePopupMenu();if(!menu || !popup)return 2;
            AppendMenuW(popup,MF_STRING,101,L"Item");AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"File");
        }
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Geometry",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,menu,instance,NULL);
        if(!w){if(menu)DestroyMenu(menu);return 3;}
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        RECT outer,client;GetWindowRect(w,&outer);GetClientRect(w,&client);
        POINT origin={0,0};ClientToScreen(w,&origin);
        TITLEBARINFOEX title={.cbSize=sizeof(title)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
        MENUBARINFO bar={.cbSize=sizeof(bar)};BOOL have_menu=GetMenuBarInfo(w,OBJID_MENU,0,&bar);
        UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
        if(have_menu)adjusted_menu_rect(w,&bar.rcBar);
        LRESULT client_hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(origin.x+20,origin.y+1));
        COLORREF menu_pixel=CLR_INVALID;
        HDC dc=GetWindowDC(w);if(dc && have_menu){menu_pixel=GetPixel(dc,bar.rcBar.right-outer.left-10,(bar.rcBar.top+bar.rcBar.bottom)/2-outer.top);ReleaseDC(w,dc);}
        else if(dc)ReleaseDC(w,dc);
        LRESULT blank_hit=have_menu?SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(bar.rcBar.right-10,(bar.rcBar.top+bar.rcBar.bottom)/2)):0;
        printf("WIN2K GEOMETRY bits=%u title=%ld menu=%ld,%ld client=%ld menu_ok=%d bare=%d client_hit=%lld menu_pixel=%06lx\n",
               (unsigned)(sizeof(void*)*8),title.rcTitleBar.bottom-title.rcTitleBar.top,
               bar.rcBar.top-outer.top,bar.rcBar.bottom-outer.top,origin.y-outer.top,have_menu,bare,(long long)client_hit,(unsigned long)menu_pixel);
        BOOL good=bare?(title.rcTitleBar.bottom-title.rcTitleBar.top==MulDiv(19,(int)dpi,96) &&
                        title.rcTitleBar.bottom<=origin.y && client_hit==HTCLIENT):
                       (have_menu && title.rcTitleBar.bottom-title.rcTitleBar.top==MulDiv(19,(int)dpi,96) &&
                        bar.rcBar.top>=title.rcTitleBar.bottom && bar.rcBar.bottom<=origin.y && client_hit==HTCLIENT && blank_hit==HTBORDER && menu_pixel==RGB(212,208,200));
        DestroyWindow(w);if(menu)DestroyMenu(menu);return good?0:4;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--hit-transparent")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Transparent hit",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);TITLEBARINFOEX info={.cbSize=sizeof(info)};
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        RECT button=info.rgrect[3];POINT caption={info.rcTitleBar.left+30,(info.rcTitleBar.top+info.rcTitleBar.bottom)/2};
        POINT maximize={(button.left+button.right)/2,(button.top+button.bottom)/2};POINT client={20,20};ClientToScreen(w,&client);
        const LRESULT values[]={HTTRANSPARENT,HTERROR,HTNOWHERE,HTCLIENT};BOOL passed=TRUE;custom_hit_enabled=TRUE;
        for(unsigned i=0;i<ARRAYSIZE(values);++i){custom_hit_value=values[i];
            LRESULT a=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(caption.x,caption.y));
            LRESULT b=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(maximize.x,maximize.y));
            LRESULT c=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(client.x,client.y));
            if(a!=values[i] || b!=values[i] || c!=values[i])passed=FALSE;
        }
        custom_hit_enabled=FALSE;
        printf("WIN2K CUSTOM HIT bits=%u preserved=%d\n",(unsigned)(sizeof(void*)*8),passed);
        DestroyWindow(w);return passed?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--system-icon")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"System icon",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        RECT outer;MENUBARINFO system={.cbSize=sizeof(system)};
        if(!GetWindowRect(w,&outer) || !GetMenuBarInfo(w,OBJID_SYSMENU,0,&system)){DestroyWindow(w);return 2;}
        RECT icon=system.rcBar;int border=(int)(info.rcTitleBar.top-outer.top);
        OffsetRect(&icon,(int)(outer.left+border-icon.left),(int)(outer.top+border-icon.top));
        POINT point={(icon.left+icon.right)/2,(icon.top+icon.bottom)/2};
        LRESULT hit_result=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y));
        LRESULT edge_hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(icon.left+1,icon.top+1));BOOL hit=hit_result==HTSYSMENU && edge_hit==HTSYSMENU;
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);HDC dc=GetWindowDC(w);
        COLORREF background=dc?GetPixel(dc,icon.left-outer.left,icon.top-outer.top):CLR_INVALID;
        if(dc)ReleaseDC(w,dc);
        BOOL gradient=background!=CLR_INVALID && GetBValue(background)>GetRValue(background);
        printf("WIN2K SYSTEM ICON bits=%u hit=%d result=%lld edge=%lld gradient=%d\n",(unsigned)(sizeof(void*)*8),hit,(long long)hit_result,(long long)edge_hit,gradient);
        DestroyWindow(w);return hit && gradient?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--help-button")){
        HWND w=CreateWindowExW(WS_EX_CONTEXTHELP,wc.lpszClassName,L"Context help",
                                WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        RECT help=info.rgrect[4];BOOL visible=!(info.rgstate[4]&STATE_SYSTEM_INVISIBLE) && help.right>help.left && help.bottom>help.top;
        BOOL hit=visible && SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM((help.left+help.right)/2,(help.top+help.bottom)/2))==HTHELP;
        BOOL clicked=hit && click_caption(w,4,HTHELP) && help_commands==1;
        printf("WIN2K HELP bits=%u visible=%d hit=%d clicked=%d commands=%u\n",(unsigned)(sizeof(void*)*8),visible,hit,clicked,help_commands);
        DestroyWindow(w);return visible && hit && clicked?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--button-cancel")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption cancellation",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        RECT button=info.rgrect[3];POINT center={(button.left+button.right)/2,(button.top+button.bottom)/2};
        SendMessageW(w,WM_NCLBUTTONDOWN,HTMAXBUTTON,MAKELPARAM(center.x,center.y));
        BOOL captured=GetCapture()==w;POINT outside={10,100};
        SendMessageW(w,WM_MOUSEMOVE,0,MAKELPARAM(outside.x,outside.y));
        SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(outside.x,outside.y));pump_messages();
        BOOL cancelled=captured && GetCapture()!=w && !IsZoomed(w);
        SendMessageW(w,WM_NCLBUTTONDBLCLK,HTMAXBUTTON,MAKELPARAM(center.x,center.y));
        BOOL double_captured=GetCapture()==w;POINT inside=center;ScreenToClient(w,&inside);
        SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(inside.x,inside.y));pump_messages();
        BOOL maximized=cancelled && double_captured && IsZoomed(w);
        BOOL restored=maximized && click_caption(w,3,HTMAXBUTTON) && !IsZoomed(w);
        printf("WIN2K BUTTON CANCEL bits=%u captured=%d cancelled=%d max=%d restore=%d\n",
               (unsigned)(sizeof(void*)*8),captured,cancelled,maximized,restored);
        DestroyWindow(w);return captured && cancelled && maximized && restored?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--buttons")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption commands",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        BOOL max=click_caption(w,3,HTMAXBUTTON) && IsZoomed(w);
        BOOL restore=max && click_caption(w,3,HTMAXBUTTON) && !IsZoomed(w);
        BOOL min=restore && click_caption(w,2,HTMINBUTTON) && IsIconic(w);
        if(min){ShowWindow(w,SW_RESTORE);pump_messages();}
        BOOL close=min && !IsIconic(w) && click_caption(w,5,HTCLOSE) && !IsWindow(w);
        if(IsWindow(w))DestroyWindow(w);
        printf("WIN2K COMMANDS bits=%u max=%d restore=%d min=%d close=%d\n",
               (unsigned)(sizeof(void*)*8),max,restore,min,close);
        return max && restore && min && close?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--physical")){
        HHOOK diagnostic=SetWindowsHookExW(WH_GETMESSAGE,diagnostic_hook,NULL,GetCurrentThreadId());
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption mouse test",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SetForegroundWindow(w);
        physical_probe probe={.window=w};HANDLE worker=CreateThread(NULL,0,physical_thread,&probe,0,NULL);
        if(!worker){DestroyWindow(w);return 3;}
        MSG message;
        while(GetMessageW(&message,NULL,0,0)>0){
            if(message.message==WM_APP+1)break;
            TranslateMessage(&message);DispatchMessageW(&message);
        }
        WaitForSingleObject(worker,3000);CloseHandle(worker);DestroyWindow(w);
        printf("WIN2K PHYSICAL bits=%u foreground=%d input=%d pressed=%d capture=%d tracked=%llu hit=%lld gray=%u native=%u cancelled=%d max=%d restore=%d\n",
               (unsigned)(sizeof(void*)*8),probe.foreground,probe.input,probe.pressed,probe.capture,(unsigned long long)probe.tracked,(long long)probe.hit,probe.gray,probe.native,probe.cancelled,probe.maximized,probe.restored);
        printf("WIN2K PHYSICAL TRACE down=%u double=%u up=%u\n",diagnostic_down,diagnostic_double,diagnostic_up);if(diagnostic)UnhookWindowsHookEx(diagnostic);
        return probe.foreground && probe.input && probe.pressed && probe.gray>80 && probe.native<20 && probe.cancelled && probe.maximized && probe.restored?0:3;
    }
    if(no_icon){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"No icon",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(w,&outer);
        HDC dc=GetWindowDC(w);COLORREF before[16*16],title_before[16*16];unsigned drawn=0,changed=0,title_changed=0;
        int left=info.rcTitleBar.left-outer.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+3;
        int top=info.rcTitleBar.top-outer.top+3;
        int title_left=info.rcTitleBar.left-outer.left+2;
        if(dc){
            COLORREF background=GetPixel(dc,left,top);
            for(int y=0;y<16;++y)for(int x=0;x<16;++x){
                COLORREF pixel=GetPixel(dc,left+x,top+y);
                before[y*16+x]=pixel;
                title_before[y*16+x]=GetPixel(dc,title_left+x,top+y);
                if(pixel!=CLR_INVALID && pixel!=background)++drawn;
            }
            SendMessageW(w,WM_SETICON,ICON_BIG,(LPARAM)LoadIconW(NULL,MAKEINTRESOURCEW(32513)));
            for(int y=0;y<16;++y)for(int x=0;x<16;++x){
                if(before[y*16+x]!=GetPixel(dc,left+x,top+y))++changed;
                if(title_before[y*16+x]!=GetPixel(dc,title_left+x,top+y))++title_changed;
            }
            ReleaseDC(w,dc);
        }
        DestroyWindow(w);
        printf("WIN2K DEFAULT ICON bits=%u drawn=%u changed=%u title_changed=%u\n",(unsigned)(sizeof(void*)*8),drawn,changed,title_changed);
        return drawn>0 && changed>0 && title_changed==0?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--tool-resize")){
        HWND tool=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Tool resize",
                                  WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME,100,100,320,180,NULL,NULL,instance,NULL);
        if(!tool)return 2;
        ShowWindow(tool,SW_SHOW);UpdateWindow(tool);SendMessageW(tool,WM_NCACTIVATE,TRUE,0);
        RECT outer;GetWindowRect(tool,&outer);TITLEBARINFOEX info={.cbSize=sizeof(info)};
        SendMessageW(tool,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        MENUBARINFO system={.cbSize=sizeof(system)};BOOL got=GetMenuBarInfo(tool,OBJID_SYSMENU,0,&system);
        UINT dpi=GetDpiForWindow(tool);if(!dpi)dpi=96;
        int border_x=expected_physical_border(tool,4,TRUE),border_y=expected_physical_border(tool,4,FALSE);
        POINT caption={info.rcTitleBar.left+5,(info.rcTitleBar.top+info.rcTitleBar.bottom)/2};
        BOOL no_icon=got && system.rcBar.right<=system.rcBar.left && system.rcBar.bottom<=system.rcBar.top;
        BOOL geometry=info.rcTitleBar.left-outer.left==border_x && info.rcTitleBar.top-outer.top==border_y &&
            info.rcTitleBar.bottom-info.rcTitleBar.top==MulDiv(16,(int)dpi,96);
        BOOL hit=SendMessageW(tool,WM_NCHITTEST,0,MAKELPARAM(caption.x,caption.y))==HTCAPTION;
        printf("WIN2K TOOL RESIZE bits=%u no_icon=%d geometry=%d hit=%d\n",
               (unsigned)(sizeof(void*)*8),no_icon,geometry,hit);
        DestroyWindow(tool);return no_icon && geometry && hit?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--tool")){
        HWND tool=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Tool window",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
                                  100,100,320,180,NULL,NULL,instance,NULL);
        BOOL tool_ok=FALSE;
        int text_top=INT_MAX,text_bottom=-1;
        if(tool){
            ShowWindow(tool,SW_SHOW);UpdateWindow(tool);SendMessageW(tool,WM_NCACTIVATE,TRUE,0);
            tool_ok=GetPropW(tool,L"Win2kStyle.Frame")!=NULL && blue_caption(tool);
            TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
            SendMessageW(tool,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(tool,&outer);
            HDC dc=GetWindowDC(tool);
            if(dc){
                RECT title=info.rcTitleBar;OffsetRect(&title,-outer.left,-outer.top);
                for(int y=title.top+1;y<title.bottom-1;++y)for(int x=title.left+1;x<title.left+100 && x<title.right;++x){
                    COLORREF c=GetPixel(dc,x,y);
                    if(c!=CLR_INVALID && GetRValue(c)>230 && GetGValue(c)>230 && GetBValue(c)>230){
                        if(y<text_top)text_top=y;
                        if(y>text_bottom)text_bottom=y;
                    }
                }
                ReleaseDC(tool,dc);
            }
            UINT dpi=GetDpiForWindow(tool);if(!dpi)dpi=96;
            int expected=MulDiv(15,(int)dpi,96);
            tool_ok=tool_ok && text_bottom>=text_top && text_bottom-text_top+1<=MulDiv(8,(int)dpi,96) &&
                info.rcTitleBar.bottom-info.rcTitleBar.top==MulDiv(16,(int)dpi,96) &&
                info.rgrect[5].right-info.rgrect[5].left==expected && info.rgrect[5].bottom-info.rgrect[5].top==expected;
            DestroyWindow(tool);
        }
        printf("WIN2K TOOL bits=%u styled=%d glyph_height=%d\n",(unsigned)(sizeof(void*)*8),tool_ok,text_bottom>=text_top?text_bottom-text_top+1:0);
        return tool_ok?0:3;
    }
    if(argc!=1)return 2;
    HWND w=CreateWindowExW(0,wc.lpszClassName,L"Classic frame",WS_OVERLAPPEDWINDOW,
                            80,80,420,260,NULL,NULL,instance,NULL);
    if(!w)return 2;
    BOOL attached=GetPropW(w,L"Win2kStyle.Frame")!=NULL;
    ShowWindow(w,SW_SHOW);UpdateWindow(w);
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    RECT window;GetWindowRect(w,&window);
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    int expected_width=MulDiv(16,(int)dpi,96),expected_height=MulDiv(14,(int)dpi,96);
    BOOL button_size=TRUE,button_hits=TRUE;
    for(unsigned i=2;i<=5;++i){
        if(info.rgstate[i]&STATE_SYSTEM_INVISIBLE)continue;
        RECT r=info.rgrect[i];
        int expected_top=info.rcTitleBar.top+(info.rcTitleBar.bottom-info.rcTitleBar.top-expected_height)/2;
        if(r.right-r.left!=expected_width || r.bottom-r.top!=expected_height || r.top!=expected_top)button_size=FALSE;
        static const LRESULT hits[]={0,0,HTMINBUTTON,HTMAXBUTTON,HTHELP,HTCLOSE};
        LRESULT observed=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2));
        if(observed!=hits[i])button_hits=FALSE;
    }
    if(!(info.rgstate[5]&STATE_SYSTEM_INVISIBLE) && !(info.rgstate[3]&STATE_SYSTEM_INVISIBLE)){
        if(info.rgrect[5].right!=info.rcTitleBar.right-MulDiv(2,(int)dpi,96))button_size=FALSE;
        POINT gap={info.rgrect[5].left-1,(info.rgrect[5].top+info.rgrect[5].bottom)/2};
        if(SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(gap.x,gap.y))!=HTCAPTION)button_hits=FALSE;
    }
    HDC dc=GetWindowDC(w);
    int x=info.rcTitleBar.left-window.left+40;
    int y=info.rcTitleBar.top-window.top+2;
    int end=info.rcTitleBar.right-window.left;
    for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
        RECT r=info.rgrect[i];OffsetRect(&r,-window.left,-window.top);
        if(r.right>r.left && r.left>x && r.left<end && !(info.rgstate[i]&STATE_SYSTEM_INVISIBLE))end=r.left;
    }
    SendMessageW(w,WM_NCACTIVATE,TRUE,0);
    COLORREF pixel=dc?GetPixel(dc,x,y):CLR_INVALID;
    COLORREF pixel2=dc?GetPixel(dc,x+80,y):CLR_INVALID;
    COLORREF endpoint=dc?GetPixel(dc,end-3,y):CLR_INVALID;
    int frame_width=info.rcTitleBar.top-window.top;
    COLORREF frame_face=dc && frame_width>4?GetPixel(dc,frame_width/2,(window.bottom-window.top)/2):CLR_INVALID;
    unsigned classic_button_pixels=0;COLORREF button_top=CLR_INVALID;
    COLORREF close_before[64*64];unsigned close_changed=0;BOOL close_disabled=FALSE;
    if(dc){
        RECT button=info.rgrect[5];OffsetRect(&button,-window.left,-window.top);
        button_top=GetPixel(dc,button.left+3,button.top);
        for(int by=button.top;by<button.bottom;++by)for(int bx=button.left;bx<button.right;++bx)
            if(GetPixel(dc,bx,by)==RGB(212,208,200))++classic_button_pixels;
        int bw=button.right-button.left,bh=button.bottom-button.top;
        HMENU system=GetSystemMenu(w,FALSE);
        if(system && bw>0 && bw<=64 && bh>0 && bh<=64){
            for(int by=0;by<bh;++by)for(int bx=0;bx<bw;++bx)
                close_before[by*64+bx]=GetPixel(dc,button.left+bx,button.top+by);
            EnableMenuItem(system,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
            DrawMenuBar(w);
            TITLEBARINFOEX disabled={.cbSize=sizeof(disabled)};
            SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&disabled);
            close_disabled=(disabled.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)!=0;
            for(int by=0;by<bh;++by)for(int bx=0;bx<bw;++bx)
                if(close_before[by*64+bx]!=GetPixel(dc,button.left+bx,button.top+by))++close_changed;
            EnableMenuItem(system,SC_CLOSE,MF_BYCOMMAND|MF_ENABLED);
            DrawMenuBar(w);
        }
    }
    unsigned icon_pixels=0;
    COLORREF icon_before[16*12];
    if(dc){
        for(int iy=info.rcTitleBar.top-window.top+2;iy<info.rcTitleBar.bottom-window.top-2;++iy)
            for(int ix=info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+4;ix<info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+20;++ix){
                COLORREF c=GetPixel(dc,ix,iy);
                if(c!=CLR_INVALID && GetRValue(c)>=GetBValue(c))++icon_pixels;
            }
    }
    unsigned icon_changed=0;
    if(dc){
        int left=info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+4,top=info.rcTitleBar.top-window.top+4;
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<16;++ix)icon_before[iy*16+ix]=GetPixel(dc,left+ix,top+iy);
        SendMessageW(w,WM_SETICON,ICON_SMALL,(LPARAM)LoadIconW(NULL,MAKEINTRESOURCEW(32513)));
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<16;++ix)
            if(icon_before[iy*16+ix]!=GetPixel(dc,left+ix,top+iy))++icon_changed;
    }
    COLORREF before[100*12];unsigned changed=0;
    if(dc){
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<100;++ix)
            before[iy*100+ix]=GetPixel(dc,x+ix,y+iy+2);
        SetWindowTextW(w,L"WWWWWWWWWW");
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<100;++ix)
            if(before[iy*100+ix]!=GetPixel(dc,x+ix,y+iy+2))++changed;
    }
    SendMessageW(w,WM_NCACTIVATE,FALSE,0);
    COLORREF inactive=dc?GetPixel(dc,x,y):CLR_INVALID;
    COLORREF inactive_end=dc?GetPixel(dc,end-3,y):CLR_INVALID;
    if(dc)ReleaseDC(w,dc);
    LRESULT hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(info.rcTitleBar.left+50,(info.rcTitleBar.top+info.rcTitleBar.bottom)/2));
    SendMessageW(w,WM_NCACTIVATE,TRUE,0);
    BOOL resized=SetWindowPos(w,NULL,0,0,520,320,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE) && blue_caption(w);
    uint64_t normal_button=button_signature(w,3);
    SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);
    uint64_t zoom_button=button_signature(w,3);
    BOOL maximized=IsZoomed(w) && blue_caption(w) && normal_button && zoom_button && normal_button!=zoom_button;
    SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);
    uint64_t restore_button=button_signature(w,3);
    BOOL restored=!IsZoomed(w) && GetPropW(w,L"Win2kStyle.Frame")!=NULL && blue_caption(w) && restore_button==normal_button;
    printf("WIN2K FRAME bits=%u attached=%d title=%ldx%ld button_size=%d button_hits=%d frame_width=%d button_top=%ld button_edge=%06lx pixels=%06lx/%06lx end=%06lx inactive=%06lx/%06lx frame=%06lx button_gray=%u close_disabled=%d close_changed=%u icon_pixels=%u icon_changed=%u text_pixels=%u hit=%lld\n",
        (unsigned)(sizeof(void*)*8),attached,info.rcTitleBar.right-info.rcTitleBar.left,
        info.rcTitleBar.bottom-info.rcTitleBar.top,button_size,button_hits,frame_width,info.rgrect[5].top-window.top,(unsigned long)button_top,(unsigned long)pixel,(unsigned long)pixel2,(unsigned long)endpoint,(unsigned long)inactive,(unsigned long)inactive_end,(unsigned long)frame_face,classic_button_pixels,close_disabled,close_changed,icon_pixels,icon_changed,changed,(long long)hit);
    SendMessageW(w,WM_SYSCOMMAND,SC_CLOSE,0);
    BOOL closed=!IsWindow(w);if(!closed)DestroyWindow(w);
    return attached && button_size && button_hits && pixel!=CLR_INVALID && pixel2!=CLR_INVALID && pixel!=pixel2 && inactive!=CLR_INVALID &&
           GetBValue(pixel)>GetRValue(pixel) && GetRValue(endpoint)>=150 && GetGValue(endpoint)>=180 && GetBValue(endpoint)>=220 && (frame_width<=4 || frame_face==RGB(212,208,200)) && button_top==RGB(255,255,255) && classic_button_pixels>20 && close_disabled && close_changed>0 &&
           GetRValue(inactive)==GetGValue(inactive) && GetGValue(inactive)==GetBValue(inactive) && GetRValue(inactive_end)>=175 && GetRValue(inactive_end)<=181 && icon_pixels>0 && icon_changed>0 && changed>0 &&
           hit==HTCAPTION && resized && maximized && restored && closed?0:3;
}
