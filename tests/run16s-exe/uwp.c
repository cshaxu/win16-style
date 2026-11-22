#include "run16s-exe/uwp.h"
#include "ntw16s-dll/uwp/target.h"
#include "lib-win32-style/application/adapters/uwp/client_frame_layout.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include <stdio.h>
#include <dwmapi.h>
#include <tlhelp32.h>
#include <wchar.h>
static DWORD pid;
static HWND frame,core,surface;
static PCWSTR target_image=L"CalculatorApp.exe";
static classic_theme rendered_theme;
static BOOL invalid_screen_sample,valid_screen_sample;
static const classic_theme stock_win2k={
    .title_buttons=WIN2000_TITLE_BUTTON_GROUP,.caption_metrics={19,16,{16,14},{15,15}}
};
static const classic_button_group stock_win16_buttons={3,0,0,{
    {CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
    {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
    {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}
},0};
static const classic_caption_metrics stock_win16_metrics={.height=18,.button={19,18}};
static const style_frame_geometry stock_win16_frame={STYLE_FRAME_LOGICAL,4,3,6,1,20,1,1};
static BOOL resize_pointer(void)
{
    RECT r;POINT previous;
    if(!surface||!GetWindowRect(surface,&r)||!GetCursorPos(&previous))return FALSE;
    BOOL topmost=!!(GetWindowLongPtrW(frame,GWL_EXSTYLE)&WS_EX_TOPMOST);
    if(!topmost&&!SetWindowPos(frame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE))return FALSE;
    INPUT movement={.type=INPUT_MOUSE};
    movement.mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK;
    movement.mi.dx=MulDiv(r.left+2-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);
    movement.mi.dy=MulDiv((r.top+r.bottom)/2-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);
    BOOL sent=SendInput(1,&movement,sizeof(movement))==1,ok=FALSE;
    DWORD last_error=0,flags=0;HCURSOR last=NULL;
    ULONGLONG deadline=GetTickCount64()+500;
    while(sent&&GetTickCount64()<deadline){
        /* cbSize is an input on every call; do not reuse an output structure. */
        CURSORINFO cursor={.cbSize=sizeof(cursor)};
        if(GetCursorInfo(&cursor)){
            flags=cursor.flags;last=cursor.hCursor;
            POINT p=cursor.ptScreenPos;
            BOOL edge=p.x>=r.left&&p.x<r.left+8&&p.y>r.top+40&&p.y<r.bottom-40;
            if(edge&&WindowFromPoint(p)==surface&&(flags&CURSOR_SHOWING)&&last==LoadCursorW(NULL,IDC_SIZEWE)){ok=TRUE;break;}
        }else last_error=GetLastError();
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,0);
    }
    BOOL restored=SetCursorPos(previous.x,previous.y);
    if(!topmost)restored=SetWindowPos(frame,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE)&&restored;
    printf("UWP_RESIZE_POINTER sent=%d match=%d flags=%lu cursor=%p error=%lu restored=%d\n",sent,ok,flags,(void*)last,last_error,restored);
    return ok&&restored;
}
static BOOL copied_entry(PCWSTR system_image)
{
    WCHAR root[MAX_PATH],directory[MAX_PATH],copy[MAX_PATH];
    if(!GetTempPathW(ARRAYSIZE(root),root)||!GetTempFileNameW(root,L"wca",0,directory))return FALSE;
    DeleteFileW(directory);
    if(!CreateDirectoryW(directory,NULL))return FALSE;
    BOOL ok=swprintf(copy,ARRAYSIZE(copy),L"%ls\\calc.exe",directory)>0&&CopyFileW(system_image,copy,FALSE);
    if(ok)ok=run16s_uwp_calculator_image(copy);
    HANDLE file=CreateFileW(copy,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);DWORD written=0;BYTE byte=0;
    if(file!=INVALID_HANDLE_VALUE){ok=WriteFile(file,&byte,1,&written,NULL)&&written==1&&ok;CloseHandle(file);}
    else ok=FALSE;
    ok=!run16s_uwp_calculator_image(copy)&&ok;
    DeleteFileW(copy);RemoveDirectoryW(directory);return ok;
}
static BOOL CALLBACK child(HWND w,LPARAM unused)
{
    (void)unused;DWORD id=0;WCHAR name[80];GetWindowThreadProcessId(w,&id);GetClassNameW(w,name,80);
    if(id==pid&&!wcscmp(name,L"Windows.UI.Core.CoreWindow")){core=w;frame=GetAncestor(w,GA_ROOT);return FALSE;}
    return TRUE;
}
static BOOL CALLBACK top(HWND w,LPARAM unused)
{child(w,unused);if(!frame)EnumChildWindows(w,child,0);return !frame;}
static BOOL CALLBACK owned(HWND w,LPARAM unused)
{
    (void)unused;
    if(frame&&GetWindow(w,GW_OWNER)==frame&&(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LAYERED))surface=w;
    return TRUE;
}
static DWORD target_pid(void)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return MAXDWORD;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};DWORD found=0;
    for(BOOL next=Process32FirstW(snapshot,&entry);next;next=Process32NextW(snapshot,&entry)){
        if(!_wcsicmp(entry.szExeFile,target_image)){
            if(found){found=MAXDWORD;break;}
            found=entry.th32ProcessID;
        }
    }
    CloseHandle(snapshot);return found;
}
static BOOL button(int hit,BOOL win2k)
{
    RECT outer,client,band;HWND title=NULL;
    for(HWND w=GetWindow(frame,GW_CHILD);w;w=GetWindow(w,GW_HWNDNEXT)){
        WCHAR name[80];GetClassNameW(w,name,80);
        if(IsWindowVisible(w)&&!wcscmp(name,L"ApplicationFrameTitleBarWindow")){title=w;break;}
    }
    if(!title||!IsWindow(surface)||!IsWindowVisible(surface))return FALSE;
    GetWindowRect(surface,&outer);GetWindowRect(title,&band);GetClientRect(frame,&client);
    POINT origin={0,0};ClientToScreen(frame,&origin);OffsetRect(&client,origin.x-outer.left,origin.y-outer.top);client.top=band.bottom-outer.top;
    SIZE size={outer.right-outer.left,outer.bottom-outer.top};
    UINT dpi=GetDpiForWindow(frame);
    DWORD style=(DWORD)GetWindowLongPtrW(frame,GWL_STYLE);
    RECT r;
    if(win2k){
        int border=IsZoomed(frame)?0:4;
        RECT caption={border,border,size.cx-border,border+classic_theme_caption_height(dpi,FALSE,&stock_win2k)},buttons[4];
        BOOL present[]={!!(style&WS_MINIMIZEBOX),!!(style&WS_MAXIMIZEBOX),FALSE,TRUE};
        if(!win2k_caption_buttons_with_group(&caption,dpi,FALSE,present,&stock_win2k.title_buttons,&stock_win2k.caption_metrics,buttons))return FALSE;
        r=buttons[hit==HTMAXBUTTON?1:0];
    }else{
        style_full_client_layout layout;
        if(!style_uwp_client_frame_layout(size,dpi,style,(DWORD)GetWindowLongPtrW(frame,GWL_EXSTYLE),IsZoomed(frame),client,
            &stock_win16_buttons,&stock_win16_metrics,&stock_win16_frame,&layout))return FALSE;
        r=hit==HTMAXBUTTON?layout.buttons[CLASSIC_BUTTON_MAXIMIZE]:layout.buttons[CLASSIC_BUTTON_MINIMIZE];
    }
    LPARAM point=MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2);DWORD_PTR ignored;
    return SendMessageTimeoutW(surface,WM_LBUTTONDOWN,MK_LBUTTON,point,SMTO_ABORTIFHUNG,1000,&ignored)&&
        SendMessageTimeoutW(surface,WM_LBUTTONUP,0,point,SMTO_ABORTIFHUNG,1000,&ignored);
}
static BOOL await_state(BOOL minimized,BOOL maximized)
{
    ULONGLONG deadline=GetTickCount64()+1500;
    do{
        RECT expected,actual;BOOL geometry=FALSE;
        if(GetWindowRect(frame,&expected)&&GetWindowRect(surface,&actual)){
            if(maximized){MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT clipped;
                if(GetMonitorInfoW(MonitorFromWindow(frame,MONITOR_DEFAULTTONEAREST),&monitor)&&
                   IntersectRect(&clipped,&expected,&monitor.rcWork))expected=clipped;
            }
            geometry=EqualRect(&expected,&actual);
        }
        if(IsIconic(frame)==minimized&&IsZoomed(frame)==maximized&&
           (minimized||(geometry&&GetAncestor(core,GA_ROOT)==frame&&IsWindowVisible(surface))))return TRUE;
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }while(GetTickCount64()<deadline);
    return FALSE;
}
static HRESULT read_rendered_theme(PCWSTR launcher,PCWSTR folder)
{
    WCHAR path[32768],*leaf=NULL;
    DWORD length=launcher?GetFullPathNameW(launcher,ARRAYSIZE(path),path,&leaf):
        GetModuleFileNameW(NULL,path,ARRAYSIZE(path));
    if(!length || length>=ARRAYSIZE(path))return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
    if(!leaf){
        WCHAR *slash=wcsrchr(path,L'\\');
        leaf=slash?slash+1:NULL;
    }
    if(!leaf || swprintf_s(leaf,ARRAYSIZE(path)-(size_t)(leaf-path),L"%ls\\%ls.theme",folder,folder)<0)
        return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
    classic_theme_mapping source={0};
    HRESULT loaded=classic_theme_open(path,&source);
    if(SUCCEEDED(loaded))rendered_theme=source.model;
    classic_theme_close(&source);
    return loaded;
}
static BOOL classic_pixels(void)
{
    RECT bounds;
    UINT dpi=GetDpiForWindow(frame);
    if(!dpi || !GetWindowRect(surface,&bounds))return FALSE;
    DWORD style=(DWORD)GetWindowLongPtrW(frame,GWL_STYLE);
    BOOL maximized=IsZoomed(frame);
    enum style_frame_border_kind kind=style&WS_THICKFRAME?STYLE_FRAME_SIZEABLE:STYLE_FRAME_FIXED;
    const style_frame_geometry *geometry=&rendered_theme.border_presentation.geometry;
    int x=maximized?0:style_frame_window_border_width(frame,geometry,kind,TRUE,dpi);
    int y=maximized?0:style_frame_window_border_width(frame,geometry,kind,FALSE,dpi);
    RECT band={x,y,bounds.right-bounds.left-x,y+classic_theme_caption_height(dpi,FALSE,&rendered_theme)},buttons[4];
    BOOL present[]={!!(style&WS_MINIMIZEBOX),!!(style&WS_MAXIMIZEBOX),FALSE,TRUE};
    if(!win2k_caption_buttons_with_group(&band,dpi,FALSE,present,&rendered_theme.title_buttons,&rendered_theme.caption_metrics,buttons))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(buttons);++i)
        if(present[i])band.right=min(band.right,buttons[i].left);
    int width=band.right-band.left;
    if(width<80)return FALSE;
    if(FAILED(DwmFlush()))return FALSE;
    HDC dc=GetDC(NULL);
    if(!dc)return FALSE;
    COLORREF sampled[2];
    const int offsets[]={width*11/20,width*4/5};
    BOOL visible=TRUE;
    for(unsigned i=0;i<ARRAYSIZE(sampled);++i){
        POINT at={bounds.left+band.left+offsets[i],bounds.top+band.top+2};
        visible=visible && WindowFromPoint(at)==surface;
        sampled[i]=GetPixel(dc,at.x,at.y);
    }
    ReleaseDC(NULL,dc);
    if(sampled[0]==CLR_INVALID || sampled[1]==CLR_INVALID){invalid_screen_sample=TRUE;return FALSE;}
    valid_screen_sample=TRUE;
    BOOL matched=FALSE;
    for(unsigned active=0;visible && active<2;++active){
        classic_caption_colors c=active?rendered_theme.active:rendered_theme.inactive;
        if(!rendered_theme.gradient)c.end=c.start;
        BOOL match=TRUE;
        for(unsigned i=0;i<ARRAYSIZE(sampled);++i)
            for(unsigned shift=0;shift<24;shift+=8){
                int left=(c.start>>shift)&255,right=(c.end>>shift)&255;
                int expected=left+MulDiv(right-left,offsets[i],width);
                int actual=(sampled[i]>>shift)&255;
                if(actual<expected-4 || actual>expected+4)match=FALSE;
            }
        matched|=match;
    }
    printf("UWP CLASSIC PIXELS visible=%d matched=%d first=%06lx second=%06lx\n",
        visible,matched,(unsigned long)sampled[0],(unsigned long)sampled[1]);
    return matched;
}
static BOOL settled_classic_pixels(void)
{
    ULONGLONG deadline=GetTickCount64()+1500;
    do{
        if(classic_pixels())return TRUE;
        Sleep(40);
    }while(GetTickCount64()<deadline);
    return FALSE;
}
static BOOL CALLBACK find_system_popup(HWND w,LPARAM context)
{
    WCHAR cls[16];
    DWORD_PTR menu=0;
    if(!IsWindowVisible(w) || !GetClassNameW(w,cls,ARRAYSIZE(cls)) || wcscmp(cls,L"#32768"))return TRUE;
    if(!SendMessageTimeoutW(w,0x01e1,0,0,SMTO_ABORTIFHUNG,100,&menu) ||
       !menu || (HMENU)menu!=GetSystemMenu(frame,FALSE))return TRUE;
    *(HWND *)context=w;
    return FALSE;
}
static BOOL menu_and_resize(void)
{
    if(IsZoomed(frame))ShowWindowAsync(frame,SW_RESTORE);
    RECT old,changed;
    if(!await_state(FALSE,FALSE) || !GetWindowRect(frame,&old) ||
       !SetWindowPos(frame,NULL,0,0,old.right-old.left+32,old.bottom-old.top+24,
                     SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE) ||
       !await_state(FALSE,FALSE) || !GetWindowRect(frame,&changed) ||
       changed.right-old.right!=32 || changed.bottom-old.bottom!=24)return FALSE;
    if(!settled_classic_pixels())return FALSE;
    DWORD thread=GetWindowThreadProcessId(frame,NULL);
    HWND popup=NULL;
    EnumThreadWindows(thread,find_system_popup,(LPARAM)&popup);
    if(popup || !PostMessageW(surface,WM_RBUTTONUP,0,MAKELPARAM(160,12)))return FALSE;
    ULONGLONG deadline=GetTickCount64()+1500;
    do{
        EnumThreadWindows(thread,find_system_popup,(LPARAM)&popup);
        if(popup)break;
        Sleep(20);
    }while(GetTickCount64()<deadline);
    BOOL seen=popup!=NULL;
    PostMessageW(frame,WM_CANCELMODE,0,0);
    deadline=GetTickCount64()+1500;
    do{
        if(!popup || !IsWindowVisible(popup))break;
        Sleep(20);
    }while(GetTickCount64()<deadline);
    BOOL dismissed=!popup || !IsWindowVisible(popup);
    printf("UWP CLASSIC INTERACTION resize=1 popup=%d dismissed=%d\n",seen,dismissed);
    return seen && dismissed && await_state(FALSE,FALSE) && settled_classic_pixels();
}
static void titles(PCSTR phase)
{
    printf("FRAME phase=%s hwnd=%p surface_exists=%d\n",phase,(void*)frame,IsWindow(surface));
    for(HWND w=GetWindow(frame,GW_CHILD);w;w=GetWindow(w,GW_HWNDNEXT)){
        WCHAR name[80];GetClassNameW(w,name,80);
        if(!wcscmp(name,L"ApplicationFrameTitleBarWindow"))printf("TITLE hwnd=%p visible=%d\n",(void*)w,IsWindowVisible(w));
    }
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2&&!wcscmp(argv[1],L"--resolve")){
        WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,ARRAYSIZE(path));
        if(!n||n+10>=ARRAYSIZE(path))return 1;
        wcscpy(path+n,L"\\calc.exe");
        BOOL ok=run16s_uwp_calculator_image(path)&&copied_entry(path);CharUpperBuffW(path,(DWORD)wcslen(path));
        ok=ok&&run16s_uwp_calculator_image(path);
        ok=ok&&!run16s_uwp_calculator_image(NULL)&&!run16s_uwp_calculator_image(L"calc.exe")&&
            !run16s_uwp_calculator_image(L"C:\\other\\calc.exe")&&!run16s_uwp_calculator_image(L"C:\\other\\notepad.exe");
        BOOL handled=TRUE;
        ok=ok&&!run16s_uwp_command(L"C:\\other\\calc.exe",L"",FALSE,
            (classic_theme_selection){0,CLASSIC_THEME_WIN16},&handled)&&!handled;
        printf("UWP_CALCULATOR_RESOLVE_%s\n",ok?"PASS":"FAIL");return ok?0:1;
    }
    BOOL interactions=(argc==3&&!wcscmp(argv[1],L"--interactions-2k")) ||
                      (argc==5&&!wcscmp(argv[1],L"--app-interactions-2k"));
    BOOL command2k=argc==3&&(interactions||!wcscmp(argv[1],L"--command-2k"));
    BOOL command_app=argc==5&&(interactions||!wcscmp(argv[1],L"--app-command-2k"));
    BOOL activate2k=argc==4&&!wcscmp(argv[1],L"--activate-2k");
    BOOL win2k=command2k || command_app || activate2k;
    BOOL command_mode=command2k || (argc==3&&!wcscmp(argv[1],L"--command-api"));
    BOOL pointer_mode=argc==4&&!wcscmp(argv[1],L"--activate-pointer");
    BOOL repeat_mode=argc==4&&!wcscmp(argv[1],L"--repeat-api");
    BOOL api_only=command_mode||activate2k||repeat_mode||(argc>=2&&!wcscmp(argv[1],L"--activate-api"));
    PCWSTR app_id=L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App";
    if(command_app){
        app_id=argv[3];target_image=argv[4];
        if(!wcschr(app_id,L'!')||wcschr(app_id,L'"')||!*target_image||wcschr(target_image,L'\\')||wcschr(target_image,L'/'))return 2;
    }else if(argc==4&&(!wcscmp(argv[1],L"--activate-api")||activate2k||pointer_mode||repeat_mode)){
        app_id=argv[2];target_image=argv[3];
        if(!wcschr(app_id,L'!')||!*target_image||wcschr(target_image,L'\\')||wcschr(target_image,L'/'))return 2;
    }else if((argc!=2&&!command_mode)||(!api_only&&wcscmp(argv[1],L"--activate"))){puts("SKIP use --activate, --activate-api [AUMID image.exe], or --command-api launcher");return 77;}
    HDESK desktop=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);WCHAR name[80]={0};DWORD needed;
    if(desktop){GetUserObjectInformationW(desktop,UOI_NAME,name,sizeof(name),&needed);CloseDesktop(desktop);}
    if((!api_only&&wcscmp(name,L"Default"))||target_pid()){puts("SKIP desktop or existing target");return 77;}
    if(api_only)puts("DISPLAY_UNVERIFIED activation starts without physical input");
    if(win2k){
        HRESULT loaded=read_rendered_theme(command_mode?argv[2]:NULL,L"Win2K");
        if(FAILED(loaded)){printf("THEME_READ error=%08lx\n",(unsigned long)loaded);return 1;}
    }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    DWORD error=0;
    PCWSTR theme_folder=win2k?L"Win2K":L"Win16";
    const classic_theme_selection selection={classic_theme_folder_id(theme_folder),
        win2k?CLASSIC_THEME_WIN2K:CLASSIC_THEME_WIN16};
    FILETIME started;GetSystemTimeAsFileTime(&started);
    if(command_mode){
        WCHAR command_line[2048];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child_process={0};
        int n=swprintf(command_line,ARRAYSIZE(command_line),L"\"%ls\" %ls calc.exe",argv[2],theme_folder);
        if(n<=0||(size_t)n>=ARRAYSIZE(command_line)||!CreateProcessW(argv[2],command_line,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child_process))return 1;
        CloseHandle(child_process.hThread);DWORD code=~0u;
        if(WaitForSingleObject(child_process.hProcess,15000)==WAIT_OBJECT_0)GetExitCodeProcess(child_process.hProcess,&code);
        else {TerminateProcess(child_process.hProcess,1);WaitForSingleObject(child_process.hProcess,1000);}
        CloseHandle(child_process.hProcess);printf("CALC_COMMAND exit=%lu\n",code);
        pid=target_pid();
        error=code;
    }else{
        error=run16s_uwp_activate(app_id,selection,&pid);
    }
    printf("ACTIVATE error=%lu own_pid=%lu\n",error,pid);fflush(stdout);
    HANDLE process=pid && pid!=MAXDWORD && target_pid()==pid?
        OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid):NULL;
    FILETIME created,exited,kernel,user;
    if(!process || !GetProcessTimes(process,&created,&exited,&kernel,&user) || CompareFileTime(&created,&started)<0){
        if(process)CloseHandle(process);
        puts("FAIL no uniquely identified new test process");return 1;
    }
    EnumWindows(top,0);EnumWindows(owned,0);BOOL ok=!error&&core&&frame&&surface;
    printf("TARGET core=%p frame=%p surface=%p dpi=%u\n",(void*)core,(void*)frame,(void*)surface,frame?GetDpiForWindow(frame):0);
    BOOL visual_missing=FALSE;
    if(ok){
        BOOL pointer_ok=!pointer_mode||resize_pointer();
        titles("before");
        BOOL zoomed=IsZoomed(frame);
        BOOL motion=await_state(FALSE,zoomed)&&button(HTMAXBUTTON,win2k)&&await_state(FALSE,!zoomed)&&button(HTMAXBUTTON,win2k)&&await_state(FALSE,zoomed);
        BOOL minimize=motion&&button(HTMINBUTTON,win2k);
        if(minimize){ULONGLONG deadline=GetTickCount64()+1500;while(!IsIconic(frame)&&GetTickCount64()<deadline)MsgWaitForMultipleObjects(0,NULL,FALSE,20,0);minimize=IsIconic(frame);}
        ShowWindowAsync(frame,SW_RESTORE);BOOL restored=await_state(FALSE,zoomed);
        printf("BUTTONS maximize_restore=%d minimize=%d restored=%d surface_visible=%d\n",motion,minimize,restored,IsWindowVisible(surface));
        titles("restored");
        BOOL actions=motion&&minimize&&restored&&IsWindowVisible(surface);
        BOOL appearance=!win2k || (actions && settled_classic_pixels());
        if(appearance && interactions)appearance=menu_and_resize();
        WCHAR renderer[32768];WCHAR *filename=NULL;
        DWORD length=command_mode?GetFullPathNameW(argv[2],ARRAYSIZE(renderer),renderer,&filename):
            GetModuleFileNameW(NULL,renderer,ARRAYSIZE(renderer));
        if(!command_mode && length && length<ARRAYSIZE(renderer)){
            PWSTR slash=wcsrchr(renderer,L'\\');
            filename=slash?slash+1:NULL;
        }
        const style_uwp_target target={pid,core,frame};
        RECT before,after;
        DWORD removed=ERROR_INVALID_NAME;
        if(length && length<ARRAYSIZE(renderer) && filename &&
           !wcscpy_s(filename,ARRAYSIZE(renderer)-(size_t)(filename-renderer),L"ntwshk64.dll")){
            removed=GetWindowRect(frame,&before)?
                ntw16s_uwp_target_change(&target,renderer,FALSE,selection):GetLastError();
        }
        printf("REMOVE error=%lu\n",removed);
        ok=!removed && GetWindowRect(frame,&after) && !IsWindow(surface) && EqualRect(&before,&after);
        printf("REMOVE_AND_GEOMETRY=%d\n",ok);
        visual_missing=ok && actions && pointer_ok && !appearance && invalid_screen_sample && !valid_screen_sample;
        ok=ok&&actions&&pointer_ok&&appearance;
    }
    if(ok&&repeat_mode){
        DWORD first_pid=pid,fresh=0;HWND first_frame=frame,first_core=core;
        error=run16s_uwp_activate(app_id,selection,&fresh);
        if(fresh&&fresh!=pid){
            PostMessageW(frame,WM_CLOSE,0,0);
            if(process){if(WaitForSingleObject(process,3000)!=WAIT_OBJECT_0){TerminateProcess(process,0);WaitForSingleObject(process,1000);}CloseHandle(process);}
            pid=fresh;process=OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
        }
        frame=NULL;core=NULL;surface=NULL;EnumWindows(top,0);EnumWindows(owned,0);
        BOOL reused=!fresh&&pid==first_pid&&frame==first_frame&&core==first_core;
        ok=!error&&process&&surface&&IsWindowVisible(surface)&&(fresh||reused);
        printf("REACTIVATE error=%lu first_pid=%lu created_pid=%lu target_pid=%lu reused=%d styled=%d\n",error,first_pid,fresh,pid,reused,ok);
    }
    HWND closing_surface=surface;
    if(frame)PostMessageW(frame,WM_CLOSE,0,0);
    if(process){if(WaitForSingleObject(process,3000)!=WAIT_OBJECT_0){TerminateProcess(process,0);WaitForSingleObject(process,1000);}CloseHandle(process);}
    if(repeat_mode){
        ULONGLONG deadline=GetTickCount64()+1500;
        while(IsWindow(closing_surface)&&GetTickCount64()<deadline)MsgWaitForMultipleObjects(0,NULL,FALSE,20,0);
        BOOL destroyed=!IsWindow(closing_surface);
        printf("REACTIVATE_CLEANUP surface_destroyed=%d\n",destroyed);
        ok=ok&&destroyed;
    }
    if(visual_missing){puts("UWP_LAUNCH_REAL_SKIP_SCREEN activation=1 actions=1 removal=1");return 77;}
    printf("UWP_LAUNCH_REAL_%s\n",ok?"PASS":"FAIL");return ok?0:1;
}
