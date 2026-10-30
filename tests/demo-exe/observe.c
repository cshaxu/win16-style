#include <windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h>
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DEMO OBSERVER FAIL %d error=%lu: %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static HANDLE tree,ready;
static HWND observed;
static HANDLE focus_ready,property_closed;
static HWND property_window;
static HWND focus_expected,dialog_window;
static DWORD WINAPI about_call(void *argument)
{
    DWORD_PTR result=0;
    return SendMessageTimeoutW((HWND)argument,WM_COMMAND,1003,0,SMTO_ABORTIFHUNG,20000,&result)?0:1;
}
static void CALLBACK interaction_event(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    (void)hook;(void)thread;(void)time;
    if(event==EVENT_OBJECT_DESTROY && object==OBJID_WINDOW && !child && window==property_window)SetEvent(property_closed);
    if(event==EVENT_OBJECT_FOCUS && object==OBJID_CLIENT && !child && window==focus_expected)SetEvent(focus_ready);
    if(event==EVENT_OBJECT_SHOW && object==OBJID_WINDOW && !child && observed && GetWindow(window,GW_OWNER)==observed){
        WCHAR title[80],cls[40];GetWindowTextW(window,title,ARRAYSIZE(title));GetClassNameW(window,cls,ARRAYSIZE(cls));
        printf("DEMO DIALOG EVENT class=%ls title=%ls\n",cls,title);fflush(stdout);
        if(!wcscmp(cls,L"#32770") && !wcscmp(title,L"About Win16 Style Demo")){
            dialog_window=window;CHECK(PostMessageW(window,WM_CLOSE,0,0));
        }
    }
}
static void CALLBACK shown(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    WCHAR name[64];DWORD pid;BOOL owned=FALSE;(void)hook;(void)event;(void)thread;(void)time;
    if(object!=OBJID_WINDOW || child || !GetClassNameW(window,name,ARRAYSIZE(name)) || wcscmp(name,L"Win16StyleDemo"))return;
    GetWindowThreadProcessId(window,&pid);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(process){IsProcessInJob(process,tree,&owned);CloseHandle(process);}
    if(owned){observed=window;SetEvent(ready);}
}
static DWORD pump_wait(HANDLE object,DWORD timeout)
{
    ULONGLONG deadline=GetTickCount64()+timeout;
    for(;;){MSG message;ULONGLONG now=GetTickCount64();if(now>=deadline)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&object,(DWORD)(deadline-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
}
static void text(HWND window,int id,PCWSTR expected)
{WCHAR actual[300]={0};DWORD_PTR result=0;
 BOOL sent=(BOOL)SendMessageTimeoutW(id?GetDlgItem(window,id):window,WM_GETTEXT,ARRAYSIZE(actual),(LPARAM)actual,SMTO_ABORTIFHUNG,5000,&result);
 if(!sent || !result || wcscmp(actual,expected))fwprintf(stderr,L"TEXT id=%d actual=[%ls] expected=[%ls] sent=%d result=%llu\n",id,actual,expected,sent,(unsigned long long)result);
 CHECK(sent && result && !wcscmp(actual,expected));}
static void modules(DWORD pid,BOOL styled,PCWSTR package,PCWSTR image)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);CHECK(snapshot!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE,main=FALSE;WCHAR wanted[MAX_PATH];
    CHECK(swprintf_s(wanted,MAX_PATH,L"%ls\\ntw16s64.dll",package)>0 && Module32FirstW(snapshot,&entry));
    do{
        if(!_wcsicmp(entry.szModule,L"ntw16s32.dll"))CHECK(FALSE);
        if(!_wcsicmp(entry.szModule,L"ntw16s64.dll")){CHECK(styled && !_wcsicmp(entry.szExePath,wanted));found=TRUE;}
        if(!_wcsicmp(entry.szExePath,image))main=TRUE;
    }while(Module32NextW(snapshot,&entry));
    CHECK(found==styled && main);CloseHandle(snapshot);
}
typedef struct pixels {uint64_t hash;unsigned caption;} pixels;
static pixels capture(HWND window,PCWSTR file)
{
    RECT r;CHECK(GetWindowRect(window,&r));int width=r.right-r.left,height=r.bottom-r.top;CHECK(width>0 && height>0);
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    HDC screen=GetDC(window),dc=CreateCompatibleDC(screen);void *data=NULL;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&data,NULL,0);CHECK(screen && dc && bitmap && data);
    HGDIOBJ previous=SelectObject(dc,bitmap);memset(data,0xff,(size_t)width*height*4);
    CHECK(PrintWindow(window,dc,0));
    pixels value={14695981039346656037ull,0};BYTE *bytes=data;
    for(size_t i=0;i<(size_t)width*height*4;++i)value.hash=(value.hash^bytes[i])*1099511628211ull;
    for(size_t i=0;i<(size_t)width*height;++i)if(bytes[i*4]==240 && bytes[i*4+1]==200 && bytes[i*4+2]==164)++value.caption;
    BITMAPFILEHEADER header={0};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);
    DWORD size=(DWORD)((size_t)width*height*4),written;header.bfSize=header.bfOffBits+size;
    HANDLE output=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(output!=INVALID_HANDLE_VALUE);
    CHECK(WriteFile(output,&header,sizeof(header),&written,NULL) && written==sizeof(header));
    CHECK(WriteFile(output,&info.bmiHeader,sizeof(info.bmiHeader),&written,NULL) && written==sizeof(info.bmiHeader));
    CHECK(WriteFile(output,data,size,&written,NULL) && written==size && CloseHandle(output));
    SelectObject(dc,previous);CHECK(DeleteObject(bitmap) && DeleteDC(dc));ReleaseDC(window,screen);return value;
}
static HWND catalog_window;
static BOOL CALLBACK find_property(HWND w,LPARAM owner)
{WCHAR name[80];if(GetWindow(w,GW_OWNER)==(HWND)owner && GetClassNameW(w,name,80) && !wcscmp(name,L"#32770"))property_window=w;return TRUE;}
static BOOL CALLBACK find_catalog(HWND w,LPARAM owner)
{
    WCHAR name[80];if(GetWindow(w,GW_OWNER)==(HWND)owner && GetClassNameW(w,name,80) && !wcscmp(name,L"Win16StyleDemoCatalog"))catalog_window=w;
    return TRUE;
}
static HWND demo_test_dialog;
static BOOL demo_modern,demo_tree;
static BOOL CALLBACK demo_child_scan(HWND w,LPARAM unused)
{
    WCHAR name[80]=L"";(void)unused;
    if(GetClassNameW(w,name,ARRAYSIZE(name)) &&
        (!lstrcmpiW(name,L"DirectUIHWND") || !lstrcmpiW(name,L"DUIViewWndClassName")))demo_modern=TRUE;
    if(!lstrcmpiW(name,L"SysTreeView32"))demo_tree=TRUE;
    return TRUE;
}
static BOOL CALLBACK demo_dialog_scan(HWND w,LPARAM owner)
{
    WCHAR cls[40];
    if(IsWindowVisible(w) && GetWindow(w,GW_OWNER)==(HWND)owner &&
        GetClassNameW(w,cls,ARRAYSIZE(cls)) && !lstrcmpW(cls,L"#32770")){
        demo_modern=demo_tree=FALSE;EnumChildWindows(w,demo_child_scan,0);demo_test_dialog=w;return FALSE;
    }
    return TRUE;
}
static void demo_dialog_paths(HWND page,DWORD thread,const WCHAR *scratch)
{
    const int extra[]={2920,2921,2922,2924,2925,2926,2927};
    for(unsigned j=0;j<ARRAYSIZE(extra);++j){
        demo_test_dialog=NULL;CHECK(PostMessageW(page,WM_COMMAND,(WPARAM)extra[j],0));
        for(unsigned i=0;i<120 && !demo_test_dialog;++i){EnumThreadWindows(thread,demo_dialog_scan,(LPARAM)page);if(!demo_test_dialog)Sleep(50);}
        CHECK(demo_test_dialog);if(extra[j]==2920)CHECK(demo_tree && !demo_modern);
        printf("DEMO EXTRA DIALOG id=%d modern=%d tree=%d\n",extra[j],demo_modern,demo_tree);fflush(stdout);
        CHECK(PostMessageW(demo_test_dialog,WM_CLOSE,0,0));
        for(unsigned i=0;i<100 && IsWindow(demo_test_dialog);++i)Sleep(50);
        CHECK(!IsWindow(demo_test_dialog));
    }
    puts("DEMO EXTRA DIALOG PASS opened=7 cancelled=7 shell_tree=1");
    for(int id=2901;id<=2909;++id){
        demo_test_dialog=NULL;CHECK(SetDlgItemTextW(page,2099,L""));CHECK(PostMessageW(page,WM_COMMAND,(WPARAM)id,0));
        for(unsigned i=0;i<100 && !demo_test_dialog;++i){
            EnumThreadWindows(thread,demo_dialog_scan,(LPARAM)page);if(!demo_test_dialog)Sleep(50);
        }
        CHECK(demo_test_dialog);
        BOOL legacy=GetDlgItem(demo_test_dialog,1120)!=NULL || GetDlgItem(demo_test_dialog,1121)!=NULL;
        printf("DEMO DIALOG PATH id=%d modern=%d legacy_list=%d\n",id,demo_modern,legacy);fflush(stdout);
        /* The legacy Shell browser is a separate layout, without the old
         * file template's lists. Its window and cancellation are checked. */
        CHECK(((id-2901)%3)==2?demo_modern:((id==2907 || id==2908)?(demo_tree && !demo_modern):(legacy && !demo_modern)));
        CHECK(PostMessageW(demo_test_dialog,WM_CLOSE,0,0));
        for(unsigned i=0;i<100 && IsWindow(demo_test_dialog);++i)Sleep(50);
        CHECK(!IsWindow(demo_test_dialog));
        WCHAR result[160]=L"";
        for(unsigned i=0;i<100;++i){
            GetDlgItemTextW(page,2099,result,ARRAYSIZE(result));
            if(!wcsncmp(result,L"Cancelled",9))break;
            Sleep(50);
        }
        CHECK(!wcsncmp(result,L"Cancelled",9));
    }
    puts("DEMO DIALOG PATHS PASS opened=9 cancelled=9");
    WCHAR selected[MAX_PATH];
    CHECK(swprintf_s(selected,ARRAYSIZE(selected),L"%ls\\dialog-selection.txt",scratch)>0);
    HANDLE fixture=CreateFileW(selected,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    CHECK(fixture!=INVALID_HANDLE_VALUE && CloseHandle(fixture));
    WCHAR save_path[MAX_PATH];
    CHECK(swprintf_s(save_path,ARRAYSIZE(save_path),L"%ls\\dialog-save-selection.txt",scratch)>0);
    CHECK(GetFileAttributesW(save_path)==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND);
    const int ids[]={2901,2902,2904,2905};
    for(unsigned index=0;index<ARRAYSIZE(ids);++index){
        int id=ids[index];const WCHAR *expected=index<2?selected:save_path;
        demo_test_dialog=NULL;CHECK(PostMessageW(page,WM_COMMAND,(WPARAM)id,0));
        for(unsigned i=0;i<100 && !demo_test_dialog;++i){
            EnumThreadWindows(thread,demo_dialog_scan,(LPARAM)page);if(!demo_test_dialog)Sleep(50);
        }
        CHECK(demo_test_dialog && !demo_modern);
        HWND edit=GetDlgItem(demo_test_dialog,1152),ok=GetDlgItem(demo_test_dialog,IDOK);
        DWORD_PTR result=0;WCHAR value[MAX_PATH]=L"";
        CHECK(edit && ok && SendMessageTimeoutW(edit,WM_SETTEXT,0,(LPARAM)expected,SMTO_ABORTIFHUNG,2000,&result) && result);
        CHECK(SendMessageTimeoutW(edit,WM_GETTEXT,ARRAYSIZE(value),(LPARAM)value,SMTO_ABORTIFHUNG,2000,&result) && !_wcsicmp(value,expected));
        CHECK(SendMessageTimeoutW(ok,BM_CLICK,0,0,SMTO_ABORTIFHUNG,2000,&result));
        for(unsigned i=0;i<100 && IsWindow(demo_test_dialog);++i)Sleep(50);
        CHECK(!IsWindow(demo_test_dialog));
        for(unsigned i=0;i<100;++i){
            value[0]=0;CHECK(SendMessageTimeoutW(GetDlgItem(page,2099),WM_GETTEXT,ARRAYSIZE(value),(LPARAM)value,SMTO_ABORTIFHUNG,2000,&result));
            if(!_wcsicmp(value,expected))break;
            Sleep(50);
        }
        CHECK(!_wcsicmp(value,expected));
    }
    CHECK(DeleteFileW(selected));
    CHECK(GetFileAttributesW(save_path)==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND);
    puts("DEMO DIALOG SELECT PASS legacy=1 bridge=1 save=1 cleanup=1");
    /* BFFM_SETSELECTIONW is a private message: allocate its string in the
     * owned demo process rather than passing a pointer from the observer. */
    demo_test_dialog=NULL;CHECK(PostMessageW(page,WM_COMMAND,2908,0));
    for(unsigned i=0;i<100 && !demo_test_dialog;++i){EnumThreadWindows(thread,demo_dialog_scan,(LPARAM)page);if(!demo_test_dialog)Sleep(50);}
    CHECK(demo_test_dialog && demo_tree && !demo_modern);
    WCHAR expected[MAX_PATH],actual[MAX_PATH]=L"";UINT n=GetWindowsDirectoryW(expected,ARRAYSIZE(expected));CHECK(n && n<ARRAYSIZE(expected));
    DWORD pid=0;GetWindowThreadProcessId(demo_test_dialog,&pid);
    HANDLE process=OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);CHECK(process);
    SIZE_T bytes=((SIZE_T)n+1)*sizeof(WCHAR),written=0;void *remote=VirtualAllocEx(process,NULL,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);CHECK(remote);
    CHECK(WriteProcessMemory(process,remote,expected,bytes,&written) && written==bytes);
    DWORD_PTR response=0;CHECK(SendMessageTimeoutW(demo_test_dialog,BFFM_SETSELECTIONW,TRUE,(LPARAM)remote,SMTO_ABORTIFHUNG,5000,&response));
    CHECK(VirtualFreeEx(process,remote,0,MEM_RELEASE));CloseHandle(process);
    HWND ok=GetDlgItem(demo_test_dialog,IDOK);CHECK(ok);
    for(unsigned i=0;i<100 && !IsWindowEnabled(ok);++i)Sleep(50);
    CHECK(IsWindowEnabled(ok));CHECK(PostMessageW(demo_test_dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)ok));
    for(unsigned i=0;i<100 && IsWindow(demo_test_dialog);++i)Sleep(50);
    CHECK(!IsWindow(demo_test_dialog));
    for(unsigned i=0;i<100;++i){actual[0]=0;CHECK(SendMessageTimeoutW(GetDlgItem(page,2099),WM_GETTEXT,ARRAYSIZE(actual),(LPARAM)actual,SMTO_ABORTIFHUNG,2000,&response));if(!_wcsicmp(actual,expected))break;Sleep(50);}
    CHECK(!_wcsicmp(actual,expected));
    puts("DEMO FOLDER TREE SELECT PASS filesystem=1");
}
static void catalog_pages(HWND main,DWORD thread,HANDLE target,BOOL styled,const WCHAR *scratch)
{
    static const WCHAR *titles[]={L"Buttons and states",L"Editing and lists",L"Files and scrollbars",L"Containers and drawing",L"Lists and toolbars",L"Ranges and dates",L"Text and special objects",L"MDI windows",L"Common dialogs"};
    DWORD initial=GetGuiResources(target,GR_GDIOBJECTS);WCHAR file[MAX_PATH];
    CHECK(GetMenuItemCount(GetSubMenu(GetMenu(main),1))==9);
    for(unsigned i=0;i<9;++i){
        catalog_window=NULL;SendMessageW(main,WM_COMMAND,2000+i,0);CHECK(EnumThreadWindows(thread,find_catalog,(LPARAM)main));HWND w=catalog_window;
        CHECK(w && IsWindowVisible(w));text(w,0,titles[i]);CHECK(WaitForInputIdle(target,3000)==0);
        if(i==0){CHECK(IsDlgButtonChecked(w,2112)==BST_INDETERMINATE);SendDlgItemMessageW(w,2110,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,2110)==BST_CHECKED);SendDlgItemMessageW(w,2122,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,2122)==BST_CHECKED && !IsDlgButtonChecked(w,2121) && IsDlgButtonChecked(w,2123)==BST_CHECKED);CHECK(!IsWindowEnabled(GetDlgItem(w,2113)));}
        if(i==1){for(int id=2211;id<=2213;++id)CHECK(SendDlgItemMessageW(w,id,CB_GETCOUNT,0,0)==8);CHECK(SendDlgItemMessageW(w,2222,LB_SETSEL,TRUE,0)!=LB_ERR && SendDlgItemMessageW(w,2222,LB_SETSEL,TRUE,2)!=LB_ERR && SendDlgItemMessageW(w,2222,LB_GETSELCOUNT,0,0)==2);CHECK(GetWindowLongPtrW(GetDlgItem(w,2202),GWL_STYLE)&ES_READONLY);}
        if(i==2){CHECK(SendDlgItemMessageW(w,2302,LB_GETCOUNT,0,0)>=0 && SendDlgItemMessageW(w,2303,LB_GETCOUNT,0,0)>=0);CHECK(GetScrollPos(GetDlgItem(w,2311),SB_CTL)==30);SendMessageW(w,WM_HSCROLL,SB_LINERIGHT,(LPARAM)GetDlgItem(w,2311));CHECK(GetScrollPos(GetDlgItem(w,2311),SB_CTL)==31);}
        if(i==3){CHECK(GetDlgItem(w,2400) && GetDlgItem(w,2405));}
        if(i==4){CHECK(SendDlgItemMessageW(w,2502,TVM_GETCOUNT,0,0)==2 && SendDlgItemMessageW(w,2503,LVM_GETITEMCOUNT,0,0)==1 && SendDlgItemMessageW(w,2504,TCM_GETITEMCOUNT,0,0)==2);}
        if(i==5){CHECK(SendDlgItemMessageW(w,2601,PBM_GETPOS,0,0)==45);SendMessageW(w,WM_COMMAND,2602,0);CHECK(SendDlgItemMessageW(w,2601,PBM_GETPOS,0,0)==50 && SendDlgItemMessageW(w,2603,TBM_GETPOS,0,0)==40 && SendDlgItemMessageW(w,2605,UDM_GETPOS,0,0)==25);}
        if(i==6){WCHAR cls[80];CHECK(GetClassNameW(GetDlgItem(w,2701),cls,80) && !_wcsicmp(cls,L"RICHEDIT50W"));SendMessageW(w,WM_COMMAND,2721,0);text(w,2722,L"Record 2: Beta");SendMessageW(w,WM_COMMAND,2712,0);text(w,2712,L"&Stop timer");SendMessageW(w,WM_COMMAND,2712,0);text(w,2712,L"&Start timer");property_window=NULL;SendMessageW(w,WM_COMMAND,2731,0);EnumThreadWindows(thread,find_property,(LPARAM)w);CHECK(property_window);HWND tab=(HWND)SendMessageW(property_window,PSM_GETTABCONTROL,0,0);CHECK(tab && SendMessageW(tab,TCM_GETITEMCOUNT,0,0)==2);CHECK(SendMessageW(property_window,PSM_SETCURSEL,1,0));CHECK(ResetEvent(property_closed));CHECK(PostMessageW(property_window,WM_CLOSE,0,0) && PostMessageW(w,WM_NULL,0,0));DWORD closed=pump_wait(property_closed,3000);printf("DEMO PROPERTY CLOSE styled=%u wait=%lu exists=%d current=%p visible=%d\n",(unsigned)styled,closed,IsWindow(property_window),(void *)SendMessageW(property_window,PSM_GETCURRENTPAGEHWND,0,0),IsWindowVisible(property_window));fflush(stdout);CHECK(closed==WAIT_OBJECT_0 && !IsWindow(property_window));}
        if(i==7){HWND client=GetDlgItem(w,2800);CHECK(client && GetWindow(client,GW_CHILD) && GetWindow(GetWindow(client,GW_CHILD),GW_HWNDNEXT));}
        if(i==8){
            static const WCHAR *dialog_buttons[]={L"Open: legacy",L"Open: modern to classic",L"Open: modern retained",
                L"Save: legacy",L"Save: modern to classic",L"Save: modern retained",
                L"Folder: Shell (old request)",L"Folder: COM to Shell tree",L"Folder: modern retained"};
            for(unsigned j=0;j<ARRAYSIZE(dialog_buttons);++j){
                HWND button=GetDlgItem(w,2901+(int)j);CHECK(button && IsWindowVisible(button));
                text(w,2901+(int)j,dialog_buttons[j]);
            }
            text(w,2920,L"Folder: Shell (new request)");
            puts("DEMO DIALOG ENTRIES PASS open=3 save=3 folder=4");
            for(int id=2921;id<=2927;++id)CHECK(GetDlgItem(w,id) && IsWindowVisible(GetDlgItem(w,id)));
            puts("DEMO EXTRA ENTRIES PASS 7");
            {const int ids[]={2901,2924,2902,2903,2921,2922,2926,2904,2925,2905,2906,2907,2920,2908,2909,2910,2911,2912,2913,2914,2923,2915,2916,2917,2918,2919,2927};
             RECT client;CHECK(GetClientRect(w,&client));
             for(unsigned a=0;a<ARRAYSIZE(ids);++a){RECT ra;HWND button=GetDlgItem(w,ids[a]);CHECK(button && GetWindowRect(button,&ra));MapWindowPoints(NULL,w,(POINT *)&ra,2);
                CHECK(ra.left>=0 && ra.top>=0 && ra.right<=client.right && ra.bottom<=client.bottom);
                for(unsigned b=a+1;b<ARRAYSIZE(ids);++b){RECT rb,intersection;CHECK(GetWindowRect(GetDlgItem(w,ids[b]),&rb));MapWindowPoints(NULL,w,(POINT *)&rb,2);CHECK(!IntersectRect(&intersection,&ra,&rb));}}
             puts("DEMO DIALOG LAYOUT PASS 27 visible nonoverlapping buttons");}
            if(styled)demo_dialog_paths(w,thread,scratch);
        }
        CHECK(swprintf_s(file,MAX_PATH,L"%ls\\catalog-%u-%u.bmp",scratch,(unsigned)styled,i)>0);SendMessageW(w,WM_NCACTIVATE,TRUE,0);UpdateWindow(w);pixels screenshot=capture(w,file);CHECK(screenshot.hash);
        SendMessageW(w,WM_CLOSE,0,0);CHECK(!IsWindow(w));printf("DEMO CATALOG PAGE PASS styled=%u page=%u gdi=%lu\n",(unsigned)styled,i,GetGuiResources(target,GR_GDIOBJECTS));
    }
    /* One representative reopened page checks application-owned font retirement
     * after native libraries have warmed, without replaying a full UI matrix. */
    DWORD warm=GetGuiResources(target,GR_GDIOBJECTS);catalog_window=NULL;SendMessageW(main,WM_COMMAND,2000,0);EnumThreadWindows(thread,find_catalog,(LPARAM)main);CHECK(catalog_window);SendMessageW(catalog_window,WM_CLOSE,0,0);
    /* The first re-open can lazily allocate a native common-control cache.
     * Establish that post-warm baseline, then require the next identical
     * lifecycle not to exceed it; native caches may also release objects
     * asynchronously, so a decrease is not evidence of a leak. */
    DWORD settled=GetGuiResources(target,GR_GDIOBJECTS);catalog_window=NULL;SendMessageW(main,WM_COMMAND,2000,0);EnumThreadWindows(thread,find_catalog,(LPARAM)main);CHECK(catalog_window);SendMessageW(catalog_window,WM_CLOSE,0,0);
    DWORD final=GetGuiResources(target,GR_GDIOBJECTS);printf("DEMO CATALOG RESOURCE SAMPLE styled=%u warm=%lu settled=%lu final=%lu\n",(unsigned)styled,warm,settled,final);
    CHECK(final<=settled);printf("DEMO CATALOG PASS styled=%u pages=9 reopen=2 resources_stable=1 initial=%lu final=%lu\n",(unsigned)styled,initial,final);
}
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],package[MAX_PATH],launcher[MAX_PATH],image[MAX_PATH],path[MAX_PATH],command[32768];
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    CHECK(argc==5 && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    CHECK(swprintf_s(package,MAX_PATH,L"%ls\\demo package",scratch)>0 && CreateDirectoryW(package,NULL));
    CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",package)>0 && CopyFileW(argv[1],launcher,FALSE));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls\\ntw16s64.dll",package)>0 && CopyFileW(argv[2],path,FALSE));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls\\ntw16s32.dll",package)>0 && CopyFileW(argv[3],path,FALSE));
    CHECK(swprintf_s(image,MAX_PATH,L"%ls\\demo.exe",package)>0 && CopyFileW(argv[4],image,FALSE));
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(ready);
    focus_ready=CreateEventW(NULL,TRUE,FALSE,NULL);property_closed=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(focus_ready && property_closed);
    HWINEVENTHOOK events=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,shown,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);CHECK(events);
    HWINEVENTHOOK interaction=SetWinEventHook(EVENT_OBJECT_CREATE,EVENT_OBJECT_VALUECHANGE,NULL,interaction_event,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);CHECK(interaction);
    pixels screenshots[2];int advances[2];
    for(unsigned styled=0;styled<2;++styled){
        tree=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(tree && SetInformationJobObject(tree,JobObjectExtendedLimitInformation,&limits,sizeof(limits)) && ResetEvent(ready));observed=NULL;
        if(styled)CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\"",launcher,image)>0);
        else CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",image)>0);
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
        startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_SHOWNORMAL;
        CHECK(CreateProcessW(styled?launcher:image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&process));
        if(!AssignProcessToJobObject(tree,process.hProcess)){TerminateProcess(process.hProcess,1);CHECK(FALSE);}
        CHECK(ResumeThread(process.hThread)==1 && pump_wait(ready,30000)==WAIT_OBJECT_0 && observed);
        HWND w=observed;DWORD pid;GetWindowThreadProcessId(w,&pid);modules(pid,styled,package,image);text(w,0,L"Win16 Style Demo");
        HANDLE target=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_INFORMATION,FALSE,pid);CHECK(target && WaitForInputIdle(target,10000)==0);
        DWORD gui_thread=GetWindowThreadProcessId(w,NULL);GUITHREADINFO gui={.cbSize=sizeof(gui)};
        CHECK(GetGUIThreadInfo(gui_thread,&gui) && gui.hwndFocus==GetDlgItem(w,101));
        HWND previous=gui.hwndFocus;
        for(int tab=0;tab<7;++tab){
            focus_expected=GetNextDlgTabItem(w,previous,FALSE);CHECK(focus_expected && focus_expected!=previous && ResetEvent(focus_ready));
            CHECK(PostMessageW(previous,WM_KEYDOWN,VK_TAB,1));CHECK(pump_wait(focus_ready,10000)==WAIT_OBJECT_0);
            CHECK(GetGUIThreadInfo(gui_thread,&gui) && gui.hwndFocus==focus_expected);previous=focus_expected;
        }
        UINT dpi=GetDpiForWindow(w);RECT scrollbar;CHECK(dpi && GetClientRect(GetDlgItem(w,107),&scrollbar));
        CHECK(scrollbar.bottom-scrollbar.top==MulDiv(17,(int)dpi,96));
        printf("DEMO INTERACTION PASS styled=%u posted Tab=7 focus verified via GUI thread; dpi=%u scrollbar thickness=%ld\n",styled,dpi,scrollbar.bottom-scrollbar.top);
        static const WCHAR *classes[]={L"BUTTON",L"EDIT",L"BUTTON",L"BUTTON",L"BUTTON",L"LISTBOX",L"COMBOBOX",L"SCROLLBAR",L"STATIC",L"STATIC",L"BUTTON"};
        for(int i=0;i<11;++i){WCHAR actual[40];HWND control=GetDlgItem(w,100+i);CHECK(control && GetClassNameW(control,actual,ARRAYSIZE(actual)) && !_wcsicmp(actual,classes[i]));}
        text(w,101,L"Native text");text(w,108,L"Ready");CHECK(IsDlgButtonChecked(w,102)==BST_CHECKED && IsDlgButtonChecked(w,103)==BST_CHECKED);
        CHECK(SendDlgItemMessageW(w,105,LB_GETCOUNT,0,0)==6 && SendDlgItemMessageW(w,106,CB_GETCOUNT,0,0)==6);
        CHECK(SendDlgItemMessageW(w,105,LB_GETCURSEL,0,0)==1 && SendDlgItemMessageW(w,106,CB_GETCURSEL,0,0)==1);
        CHECK(GetMenuItemCount(GetMenu(w))==3 && GetMenuItemCount(GetSubMenu(GetMenu(w),0))==3);
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);UpdateWindow(w);
        CHECK(swprintf_s(path,MAX_PATH,L"%ls\\demo-%ls.bmp",scratch,styled?L"injected":L"native")>0);screenshots[styled]=capture(w,path);
        LRESULT first=SendDlgItemMessageW(w,101,EM_POSFROMCHAR,0,0),last=SendDlgItemMessageW(w,101,EM_POSFROMCHAR,5,0);
        CHECK(first!=-1 && last!=-1);advances[styled]=(short)LOWORD(last)-(short)LOWORD(first);
        CHECK(SendMessageW(GetDlgItem(w,101),WM_SETTEXT,0,(LPARAM)L"User text"));text(w,101,L"User text");SendDlgItemMessageW(w,100,BM_CLICK,0,0);text(w,108,L"Applied: User text");
        SendDlgItemMessageW(w,102,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,102)==BST_UNCHECKED);text(w,108,L"Disabled");
        SendDlgItemMessageW(w,104,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,104)==BST_CHECKED && IsDlgButtonChecked(w,103)==BST_UNCHECKED);text(w,108,L"Option B");
        SendDlgItemMessageW(w,105,WM_KEYDOWN,VK_DOWN,0);CHECK(SendDlgItemMessageW(w,105,LB_GETCURSEL,0,0)==2);text(w,108,L"List selection: 3");
        SendDlgItemMessageW(w,106,WM_KEYDOWN,VK_DOWN,0);CHECK(SendDlgItemMessageW(w,106,CB_GETCURSEL,0,0)==2);text(w,108,L"Combo selection: 3");
        SendDlgItemMessageW(w,106,CB_SHOWDROPDOWN,TRUE,0);CHECK(SendDlgItemMessageW(w,106,CB_GETDROPPEDSTATE,0,0));
        SendDlgItemMessageW(w,106,CB_SHOWDROPDOWN,FALSE,0);CHECK(!SendDlgItemMessageW(w,106,CB_GETDROPPEDSTATE,0,0));
        if(styled){
            HWND scroll=GetDlgItem(w,107);int cross=scrollbar.bottom-scrollbar.top;
            SendMessageW(scroll,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(cross/2,cross/2));
            CHECK(GetGUIThreadInfo(gui_thread,&gui) && gui.hwndCapture==scroll);
            SendMessageW(scroll,WM_LBUTTONUP,0,MAKELPARAM(cross/2,cross/2));
            CHECK(GetGUIThreadInfo(gui_thread,&gui) && gui.hwndCapture!=scroll && GetScrollPos(scroll,SB_CTL)==29);
            printf("DEMO CAPTURE PASS styled=1 scrollbar arrow press/release position=29\n");
            SendMessageW(w,WM_HSCROLL,SB_LINERIGHT,(LPARAM)scroll);
        }
        SendMessageW(w,WM_HSCROLL,SB_LINERIGHT,(LPARAM)GetDlgItem(w,107));CHECK(GetScrollPos(GetDlgItem(w,107),SB_CTL)==31);text(w,108,L"Scroll position: 31");
        SendMessageW(w,WM_COMMAND,1001,0);text(w,108,L"Ready");text(w,101,L"Native text");CHECK(IsDlgButtonChecked(w,102)==BST_CHECKED && GetScrollPos(GetDlgItem(w,107),SB_CTL)==30);
        SendMessageW(w,WM_SYSCOMMAND,SC_MINIMIZE,0);CHECK(IsIconic(w));SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);CHECK(!IsIconic(w));
        SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);CHECK(IsZoomed(w));SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);CHECK(!IsZoomed(w));
        RECT before,after;CHECK(GetWindowRect(w,&before));CHECK(SetWindowPos(w,NULL,before.left+13,before.top+19,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));
        CHECK(GetWindowRect(w,&after) && after.left==before.left+13 && after.top==before.top+19);
        dialog_window=NULL;HANDLE about=CreateThread(NULL,0,about_call,w,0,NULL);CHECK(about);
        CHECK(pump_wait(about,25000)==WAIT_OBJECT_0 && dialog_window);
        DWORD about_code;CHECK(GetExitCodeThread(about,&about_code) && !about_code && !IsWindow(dialog_window) && IsWindowEnabled(w) && CloseHandle(about));
        printf("DEMO ABOUT/MOVE PASS styled=%u modal dialog opened/closed and exact window displacement\n",styled);
        catalog_pages(w,gui_thread,target,styled,scratch);
        SendMessageW(w,WM_COMMAND,1002,0);CHECK(pump_wait(process.hProcess,10000)==WAIT_OBJECT_0);DWORD code;CHECK(GetExitCodeProcess(process.hProcess,&code) && !code);
        CHECK(WaitForSingleObject(target,10000)==WAIT_OBJECT_0 && CloseHandle(target));
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(tree);tree=NULL;
        printf("DEMO OBSERVE PASS styled=%u same image, six classes, data/notifications/menu/window commands, text advance=%d caption pixels=%u\n",styled,advances[styled],screenshots[styled].caption);
    }
    CHECK(screenshots[0].hash!=screenshots[1].hash && screenshots[1].caption>screenshots[0].caption+500 && advances[1]==advances[0]);
    CHECK(UnhookWinEvent(interaction) && UnhookWinEvent(events) && CloseHandle(focus_ready) && CloseHandle(property_closed) && CloseHandle(ready));puts("DEMO PAIR PASS external module/appearance/business observations on private desktop; physical input not claimed");return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
