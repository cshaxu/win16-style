/* Project-authored optional observer for original VB5/VB6 intrinsic samples.
 * Binaries/runtime paths are explicit; no copied external implementation. */
/* Optional real VB6 application observer; paths are explicit arguments.
 * Own private desktop/Job, native API observations, no existing-process access. */
#include <windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include "../ntw16s-dll/isolated_process.h"
#include "../../src/ntw16s-dll/control_glyphs.h"
#include "../../src/ntw16s-dll/combo_glyph.h"
#include "../../src/ntw16s-dll/scroll_glyphs.h"
#include "vb_queue.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_SAMPLE_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static void file_path(const WCHAR *path){
    HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    CHECK(f!=INVALID_HANDLE_VALUE);
    WCHAR actual[32768];
    CHECK(GetFinalPathNameByHandleW(f,actual,32768,FILE_NAME_NORMALIZED));
    printf("VB_SAMPLE_MODULE_FILE actual=%ls\n",actual);
    CloseHandle(f);
}
static HANDLE job,ready,port;
static HWND form;
static BOOL graphical_pixel_check;
static DWORD pid;
static HWND checks[80];
static unsigned count;
static unsigned about_closed;
static WCHAR about_capture_dir[MAX_PATH];
static uint64_t capture(HWND,const WCHAR *,BOOL);
static BOOL CALLBACK capture_print(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[80];GetClassNameW(w,cls,80);
    if(!wcscmp(cls,L"ThunderRT5PictureBox") && !(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)){
        WCHAR path[MAX_PATH];CHECK(swprintf_s(path,MAX_PATH,L"%ls/about-print.bmp",about_capture_dir)>0);
        printf("VB_REAL_PRINT_IMAGE hash=%llu\n",(unsigned long long)capture(w,path,FALSE));
    }
    return TRUE;
}
static BOOL CALLBACK close_about(HWND w,LPARAM main)
{
    WCHAR cls[80],title[80];GetClassNameW(w,cls,80);GetWindowTextW(w,title,80);
    if(w!=(HWND)main && !wcscmp(cls,L"ThunderRT5Form") && !wcscmp(title,L"About...")){
        CHECK(EnumChildWindows(w,capture_print,0));
        SendMessageW(w,WM_CLOSE,0,0);if(!IsWindow(w))++about_closed;
    }
    return TRUE;
}
static HANDLE queue_mapping,queue_ready;
static vb_queue_state *queue_state;
static HMODULE queue_module;
static HHOOK queue_get,queue_sent;
static DWORD pump(HANDLE h,DWORD timeout){
    ULONGLONG end=GetTickCount64()+timeout;
    for(;;){
        ULONGLONG now=GetTickCount64();
        if(now>=end)return WAIT_TIMEOUT;
        DWORD r=MsgWaitForMultipleObjectsEx(1,&h,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(r!=WAIT_OBJECT_0+1)return r;
        MSG m;
        while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
    }
}
static void queue_prepare(void)
{
    WCHAR name[128],event_name[128];
    CHECK(swprintf_s(name,128,L"Local\\Win16VBQueue-%lu-%llu",GetCurrentProcessId(),(unsigned long long)GetTickCount64())>0);
    CHECK(swprintf_s(event_name,128,L"%ls-ready",name)>0);
    queue_mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*queue_state),name);
    CHECK(queue_mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    queue_state=MapViewOfFile(queue_mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*queue_state));
    CHECK(queue_state);ZeroMemory(queue_state,sizeof(*queue_state));queue_state->size=sizeof(*queue_state);
    CHECK(!wcscpy_s(queue_state->event_name,128,event_name));
    queue_ready=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(queue_ready && GetLastError()!=ERROR_ALREADY_EXISTS);
    CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,name));
}
static void queue_install(const WCHAR *path)
{
    queue_state->pid=pid;queue_state->tid=GetWindowThreadProcessId(form,NULL);
    queue_module=LoadLibraryW(path);CHECK(queue_module);
    HOOKPROC get=(HOOKPROC)(UINT_PTR)GetProcAddress(queue_module,"VbQueueGet");
    HOOKPROC sent=(HOOKPROC)(UINT_PTR)GetProcAddress(queue_module,"VbQueueSent");
    CHECK(get && sent);
    queue_get=SetWindowsHookExW(WH_GETMESSAGE,get,queue_module,queue_state->tid);
    queue_sent=SetWindowsHookExW(WH_CALLWNDPROC,sent,queue_module,queue_state->tid);
    CHECK(queue_get && queue_sent);
}
static void queue_key(HWND window,WPARAM key)
{
    DWORD target_pid=0;CHECK(GetWindowThreadProcessId(window,&target_pid)==queue_state->tid && target_pid==pid);
    CHECK(IsChild(form,window) && AttachThreadInput(GetCurrentThreadId(),queue_state->tid,TRUE));
    SetActiveWindow(form);SetFocus(window);GUITHREADINFO gui={.cbSize=sizeof(gui)};
    CHECK(GetGUIThreadInfo(queue_state->tid,&gui) && (gui.hwndFocus==window || IsChild(window,gui.hwndFocus)));
    HWND destination=gui.hwndFocus;
    CHECK(ResetEvent(queue_ready));queue_state->root=GetParent(window);queue_state->window=window;
    queue_state->commands=queue_state->hscroll=queue_state->vscroll=0;
    queue_state->action=0;
    queue_state->acknowledgements=queue_state->rejected=0;
    queue_state->command_code=queue_state->scroll_code=0;
    ++queue_state->generation;MemoryBarrier();
    UINT barrier=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(barrier);
    LPARAM key_data=1|((LPARAM)MapVirtualKeyW((UINT)key,MAPVK_VK_TO_VSC)<<16)|0x01000000;
    CHECK(PostMessageW(window,barrier,queue_state->generation+1,0));
    CHECK(PostMessageW(destination,WM_KEYDOWN,key,key_data) && PostMessageW(destination,WM_KEYUP,key,key_data|(LPARAM)0xc0000000u));
    CHECK(PostMessageW(window,barrier,queue_state->generation,0));
    CHECK(pump(queue_ready,3000)==WAIT_OBJECT_0);MemoryBarrier();
    CHECK(queue_state->completed==(LONG)queue_state->generation);
    CHECK(queue_state->acknowledgements==1 && queue_state->rejected==1);
    CHECK(AttachThreadInput(GetCurrentThreadId(),queue_state->tid,FALSE));
    printf("VB_SAMPLE_QUEUE_OK key=%u generation=%lu commands=%ld hscroll=%ld vscroll=%ld command_code=%lu scroll_code=%lu\n",
        (unsigned)key,queue_state->generation,queue_state->commands,queue_state->hscroll,queue_state->vscroll,queue_state->command_code,queue_state->scroll_code);
}
static void queue_survey(HWND window,LONG action)
{
    DWORD owner=0;CHECK(GetWindowThreadProcessId(window,&owner)==queue_state->tid && owner==pid);
    queue_state->window=window;queue_state->root=GetParent(window);
    queue_state->action=action?action:VB_QUEUE_SURVEY;
    ++queue_state->generation;CHECK(ResetEvent(queue_ready));MemoryBarrier();
    UINT marker=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(marker);
    CHECK(PostMessageW(window,marker,queue_state->generation,0) && pump(queue_ready,3000)==WAIT_OBJECT_0);
    CHECK(queue_state->completed==(LONG)queue_state->generation);MemoryBarrier();
    WCHAR cls[100];GetClassNameW(window,cls,100);
    printf("VB_FILE_SURVEY class=%ls attached=%ld font_records=%ld system_fonts=%ld draw_items=%ld draw_type=%lu draw_dc_window=%p popup_dc=%ld system_text=%ld native_text=%ld\n",
        cls,queue_state->attached,queue_state->font_records,queue_state->system_fonts,queue_state->draw_items,
        queue_state->draw_type,(void *)queue_state->draw_dc_window,queue_state->popup_dc,queue_state->system_text,queue_state->native_text);
    if(action==VB_QUEUE_PAINT_SURVEY)printf("VB_NORMAL_PAINT class=%ls messages=%ld system_selected=%ld native_selected=%ld restored=%ld probe_error=%ld\n",cls,queue_state->paint_messages,queue_state->paint_system,queue_state->paint_native,queue_state->paint_restored,queue_state->paint_probe_error);
    if(action==VB_QUEUE_PAINT_SURVEY)CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);
    if(action==VB_QUEUE_PAINT_SURVEY)printf("VB_NORMAL_BUFFER class=%ls system_selected=%ld native_selected=%ld\n",cls,queue_state->paint_system_memory,queue_state->paint_native_memory);
    if(action==VB_QUEUE_PAINT_SURVEY && queue_state->paint_system_callsite)printf("VB_PAINT_SYSTEM_CALLSITE address=%p\n",(void *)queue_state->paint_system_callsite);
    if(action==VB_QUEUE_PRINT_SAMPLE){printf("VB_REAL_PRINT_API calls=%ld api=%lu alignment=%lu restored=%ld error=%ld\n",queue_state->print_calls,queue_state->print_api,queue_state->print_alignment,queue_state->paint_restored,queue_state->paint_probe_error);CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);}
    queue_state->action=0;
}
static void CALLBACK shown(HWINEVENTHOOK h,DWORD e,HWND w,LONG object,LONG child,DWORD thread,DWORD time){
    (void)h;
    (void)e;
    (void)thread;
    (void)time;
    WCHAR cls[80];
    if(object!=OBJID_WINDOW || child || !GetClassNameW(w,cls,80))return;
    DWORD candidate;
    GetWindowThreadProcessId(w,&candidate);
    HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,candidate);
    BOOL own=FALSE;
    if(p){
        IsProcessInJob(p,job,&own);
        CloseHandle(p);
    }
    if(own){
        WCHAR title[160];GetWindowTextW(w,title,160);printf("SAMPLE_SHOW class=%ls title=%ls hwnd=%p\n",cls,title,(void *)w);fflush(stdout);
        if(!wcsstr(cls,L"Form") || GetParent(w))return;
        form=w;
        pid=candidate;
        SetEvent(ready);
    }
}
static BOOL CALLBACK collect(HWND w,LPARAM ignored)
{
    (void)ignored;WCHAR cls[100],text[120];GetClassNameW(w,cls,100);GetWindowTextW(w,text,120);
    HWND parent=GetParent(w);WCHAR parent_class[100];GetClassNameW(parent,parent_class,100);
    WCHAR real_class[100];CHECK(RealGetWindowClassW(w,real_class,100));
    printf("SAMPLE_REAL_CLASS hwnd=%p registered=%ls real=%ls\n",(void *)w,cls,real_class);
    printf("SAMPLE_WINDOW hwnd=%p parent=%p class=%ls parent_class=%ls owner=%p style=%08lx ex=%08lx text=%ls font=%p\n",(void *)w,(void *)parent,cls,parent_class,(void *)GetClassLongPtrW(w,GCLP_HMODULE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),text,(void *)SendMessageW(w,WM_GETFONT,0,0));
    if(wcsstr(cls,L"OptionButton") || wcsstr(cls,L"CheckBox"))printf("SAMPLE_BUTTON hwnd=%p dlg=%llx check=%lld state=%lld\n",(void *)w,(unsigned long long)SendMessageW(w,WM_GETDLGCODE,0,0),(long long)SendMessageW(w,BM_GETCHECK,0,0),(long long)SendMessageW(w,BM_GETSTATE,0,0));
    if(wcsstr(cls,L"ComboBox")) {COMBOBOXINFO i={.cbSize=sizeof(i)};BOOL ok=GetComboBoxInfo(w,&i);printf("SAMPLE_COMBO hwnd=%p count=%lld selected=%lld info=%d edit=%p list=%p error=%lu\n",(void *)w,(long long)SendMessageW(w,CB_GETCOUNT,0,0),(long long)SendMessageW(w,CB_GETCURSEL,0,0),ok,(void *)i.hwndItem,(void *)i.hwndList,GetLastError());}
    if(wcsstr(cls,L"ListBox"))printf("SAMPLE_LIST hwnd=%p count=%lld selected=%lld\n",(void *)w,(long long)SendMessageW(w,LB_GETCOUNT,0,0),(long long)SendMessageW(w,LB_GETCURSEL,0,0));
    if(wcsstr(cls,L"DriveListBox")){
        COMBOBOXINFO combo={.cbSize=sizeof(combo)};BOOL valid=GetComboBoxInfo(w,&combo);
        printf("VB_FILE_DRIVE_PROTOCOL info=%d count=%lld selected=%lld item=%p list=%p owner_draw=%u has_strings=%u\n",
            valid,(long long)SendMessageW(w,CB_GETCOUNT,0,0),(long long)SendMessageW(w,CB_GETCURSEL,0,0),
            (void *)combo.hwndItem,(void *)combo.hwndList,((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE))!=0,((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&CBS_HASSTRINGS)!=0);
    }
    if(count<80)checks[count++]=w;
    return TRUE;
}
static uint64_t capture(HWND w,const WCHAR *file,BOOL verify){
    RECT r;
    CHECK(GetWindowRect(w,&r));
    int cx=r.right-r.left,cy=r.bottom-r.top;
    CHECK(cx>0 && cy>0);
    BITMAPINFO info= {0};
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=cx;
    info.bmiHeader.biHeight=-cy;
    info.bmiHeader.biPlanes=1;
    info.bmiHeader.biBitCount=32;
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);
    void *data;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&data,NULL,0);
    CHECK(screen && dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    memset(data,255,(size_t)cx*cy*4);
    CHECK(PrintWindow(w,dc,0));
    uint64_t hash=14695981039346656037ull;
    BYTE *b=data;
    for(size_t i=0; i<(size_t)cx*cy*4;
    ++i)hash=(hash^b[i])*1099511628211ull;
    WCHAR role[100];GetClassNameW(w,role,100);
    if(graphical_pixel_check && wcsstr(role,L"CommandButton") && (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW){
        int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));DWORD *pixels=data;
        CHECK(cx>u && cy>u && (pixels[0]&0xffffff)==0xffffff && (pixels[u]&0xffffff)==0);
        printf("VB_GRAPHICAL_FRAME_PIXELS_OK scale=%d unobserved_print=1\n",u);
    }
    if(verify && wcsstr(role,L"ComboBox")){
        COMBOBOXINFO combo={.cbSize=sizeof(combo)};CHECK(GetComboBoxInfo(w,&combo));
        POINT origin={0,0};CHECK(ClientToScreen(w,&origin));int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
        RECT arrow=combo.rcButton;InflateRect(&arrow,-u,-u);
        int left=(arrow.left+arrow.right-7*u)/2+origin.x-r.left,top=(arrow.top+arrow.bottom-9*u)/2+origin.y-r.top;
        CHECK(left>=0 && top>=0 && left+7*u<=cx && top+9*u<=cy);
        unsigned checked=0;DWORD *pixels=data;
        for(int y=0;y<9;++y)for(int x=0;x<7;++x)if(combo_arrow[y][x]=='0'){
            DWORD expected=IsWindowEnabled(w)?0:0x808080;
            CHECK((pixels[(top+y*u)*cx+left+x*u]&0xffffff)==expected);++checked;
        }
        printf("VB_SAMPLE_COMBO_GLYPH_OK black_source_pixels=%u scale=%d\n",checked,u);
    }else if(verify && wcsstr(role,L"ScrollBar")){
        BOOL vertical=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0;int cross=vertical?cx:cy;
        SCROLLBARINFO bar={.cbSize=sizeof(bar)};CHECK(GetScrollBarInfo(w,OBJID_CLIENT,&bar));
        const char (*glyph)[18]=(bar.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)?(vertical?scroll_uparrowi:scroll_lfarrowi):(vertical?scroll_uparrow:scroll_lfarrow);
        const DWORD colors[4]={0,0xffffff,0x808080,0xc0c0c0};DWORD *pixels=data;unsigned checked=0;
        for(int y=1;y<16;++y)for(int x=1;x<16;++x){int px=MulDiv(x,cross,17),py=MulDiv(y,cross,17);
            if(px==MulDiv(x+1,cross,17) || py==MulDiv(y+1,cross,17))continue;
            CHECK(px<cx && py<cy && (pixels[py*cx+px]&0xffffff)==colors[glyph[y][x]-'0']);++checked;
        }
        CHECK(checked);printf("VB_SAMPLE_SCROLL_GLYPH_OK vertical=%d pixels=%u square=%d\n",vertical,checked,cross);
    }else if(verify){RECT client;CHECK(GetClientRect(w,&client));int dpi=(int)GetDpiForWindow(w);int top=(client.bottom-MulDiv(13,dpi,96))/2;CHECK(top>=0 && cx>=MulDiv(14,dpi,96) && cy>=top+MulDiv(13,dpi,96));
        LRESULT check=SendMessageW(w,BM_GETCHECK,0,0);CHECK(check==0 || check==1);DWORD *pixels=data;WCHAR cls[100];GetClassNameW(w,cls,100);unsigned row=wcsstr(cls,L"OptionButton")?13:0;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){DWORD expected=control_checks[row+y][x+(check?14:0)]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,dpi,96))*cx+MulDiv(x,dpi,96)]&0xffffff)==expected);}
        printf("VB_SAMPLE_GLYPH_OK source_pixels=182 check=%lld\n",(long long)check);
    }
    BITMAPFILEHEADER head= {0};
    head.bfType=0x4d42;
    head.bfOffBits=sizeof(head)+sizeof(info.bmiHeader);
    head.bfSize=head.bfOffBits+(DWORD)((size_t)cx*cy*4);
    DWORD n;
    HANDLE out=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
    CHECK(out!=INVALID_HANDLE_VALUE && WriteFile(out,&head,sizeof(head),&n,NULL) && n==sizeof(head) && WriteFile(out,&info.bmiHeader,sizeof(info.bmiHeader),&n,NULL) && n==sizeof(info.bmiHeader) && WriteFile(out,data,head.bfSize-head.bfOffBits,&n,NULL) && n==head.bfSize-head.bfOffBits && CloseHandle(out));
    SelectObject(dc,old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(w,screen);
    return hash;
}
static int worker(int argc,WCHAR **argv){
    CHECK(argc==4 || argc==5);
    if(argc==5)queue_prepare();
    BOOL styled=wcscmp(argv[1],L"bare")!=0;
    WCHAR command[2048],path[MAX_PATH],scratch[MAX_PATH];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit= {0};
    limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    job=CreateJobObjectW(NULL,NULL);
    CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
    port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT association= {.CompletionKey=job,.CompletionPort=port};
    CHECK(port && SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    HWINEVENTHOOK hook=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,shown,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    CHECK(ready && hook);
    STARTUPINFOW si= {.cb=sizeof(si)};
    PROCESS_INFORMATION p= {0};
    if(styled)CHECK(swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\"",argv[1],argv[2])>0);
    else CHECK(swprintf_s(command,2048,L"\"%ls\"",argv[2])>0);
    CHECK(CreateProcessW(styled?argv[1]:argv[2],command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p));
    CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1 && pump(ready,12000)==WAIT_OBJECT_0);
    HANDLE target=OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE,FALSE,pid);
    CHECK(target && WaitForInputIdle(target,5000)==0 && IsWindowVisible(form) && !IsIconic(form));
    if(queue_state)queue_install(argv[4]);
    DWORD size=MAX_PATH;
    CHECK(QueryFullProcessImageNameW(target,0,path,&size));
    BOOL wow=FALSE;CHECK(IsWow64Process(target,&wow) && wow);
    HANDLE process_snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);CHECK(process_snapshot!=INVALID_HANDLE_VALUE);
    PROCESSENTRY32W entry= {.dwSize=sizeof(entry)};DWORD parent=0;CHECK(Process32FirstW(process_snapshot,&entry));
    do{if(entry.th32ProcessID==pid){parent=entry.th32ParentProcessID;break;}}while(Process32NextW(process_snapshot,&entry));CloseHandle(process_snapshot);
    CHECK(parent==(styled?p.dwProcessId:GetCurrentProcessId()));
    printf("VB_SAMPLE_TARGET pid=%lu creator=%lu initial_pid=%lu bits=32 path=%ls dpi=%u\n",pid,parent,p.dwProcessId,path,GetDpiForWindow(form));
    HANDLE snap=INVALID_HANDLE_VALUE;unsigned retries=0;
    do{snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snap!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }while(++retries<8);
    printf("VB_SAMPLE_MODULE_SNAPSHOT bad_length_retries=%u error=%lu\n",retries,snap==INVALID_HANDLE_VALUE?GetLastError():0);
    CHECK(snap!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W mod= {.dwSize=sizeof(mod)};
    HMODULE runtime=NULL;
    BOOL dll=FALSE;
    CHECK(Module32FirstW(snap,&mod));
    do{
        if(!_wcsicmp(mod.szModule,L"CompressZItLib6.dll") || !_wcsicmp(mod.szModule,L"vbalHook6.dll") || !_wcsicmp(mod.szModule,L"vbalHook.dll") ||
                !_wcsicmp(mod.szModule,L"msscript.ocx") || !_wcsicmp(mod.szModule,L"vbscript.dll")){
            printf("VB_SAMPLE_PRIVATE_COM base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);file_path(mod.szExePath);
        }
        if((!_wcsicmp(mod.szModule,L"MSVBVM60.DLL") || !_wcsicmp(mod.szModule,L"MSVBVM50.DLL"))){
            runtime=mod.hModule;
            printf("VB_SAMPLE_RUNTIME base=%p path=%ls\n",(void *)runtime,mod.szExePath);
            file_path(mod.szExePath);
        }
        if(!_wcsicmp(mod.szModule,L"ntw16s32.dll")){
            dll=TRUE;
            printf("VB_SAMPLE_STYLE_MODULE base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);
            file_path(mod.szExePath);
        }
    }
    while(Module32NextW(snap,&mod));
    CloseHandle(snap);
    printf("VB_GRAPHICAL_OBSERVER_DIAGNOSTICS available=1\n");
    BOOL about_print=!wcscmp(argv[3],L"about-print");
    BOOL container_verify=!wcscmp(argv[3],L"containers-verify");
    BOOL containers=container_verify || !wcscmp(argv[3],L"containers");
    BOOL filedir_verify=!wcscmp(argv[3],L"filedir-verify");
    BOOL file_verify=filedir_verify || !wcscmp(argv[3],L"files-verify");
    BOOL graphical_verify=!wcscmp(argv[3],L"graphical-verify");
    BOOL graphical_pictures=!wcscmp(argv[3],L"graphical-pictures");
    BOOL graphical=graphical_verify || graphical_pictures || !wcscmp(argv[3],L"graphical-observe");
    BOOL inventory=graphical || about_print || containers || file_verify || !wcscmp(argv[3],L"inventory");
    CHECK(runtime && dll==styled && EnumChildWindows(form,collect,0) && (inventory?count>0:count>10));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls/actor-%ls.bmp",scratch,styled?L"styled":L"bare")>0);
    printf("VB_SAMPLE_IMAGE hash=%llu\n",(unsigned long long)capture(form,path,FALSE));
    printf("SAMPLE_ROOT class_owner=%p windows=%u\n",(void *)GetClassLongPtrW(form,GCLP_HMODULE),count);
    if(!inventory){
    unsigned options=0,checkboxes=0,combos=0,lists=0,edits=0,bars=0;
    for(unsigned i=0;i<count;++i){HWND w=checks[i];WCHAR cls[100];GetClassNameW(w,cls,100);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);HFONT original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT original_rect,after_rect;CHECK(GetWindowRect(w,&original_rect));
        if(owner!=runtime)continue;
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/control-%u.bmp",scratch,i)>0);uint64_t hash=capture(w,path,!wcscmp(argv[3],L"verify") && (wcsstr(cls,L"OptionButton") || wcsstr(cls,L"CheckBox") || wcsstr(cls,L"ComboBox") || wcsstr(cls,L"ScrollBar")));printf("SAMPLE_PIXELS index=%u class=%ls hash=%llu\n",i,cls,(unsigned long long)hash);
        if(owner!=runtime)continue;
        if(wcsstr(cls,L"OptionButton")){++options;if(!IsWindowEnabled(w))continue;SendMessageW(w,BM_CLICK,0,0);CHECK(SendMessageW(w,BM_GETCHECK,0,0)==BST_CHECKED);for(unsigned j=0;j<count;++j){WCHAR other[100];GetClassNameW(checks[j],other,100);if(checks[j]!=w && wcsstr(other,L"OptionButton") && GetParent(checks[j])==GetParent(w))CHECK(SendMessageW(checks[j],BM_GETCHECK,0,0)==BST_UNCHECKED);}CHECK(swprintf_s(path,MAX_PATH,L"%ls/selected-%u.bmp",scratch,i)>0);(void)capture(w,path,!wcscmp(argv[3],L"verify"));printf("SAMPLE_OPTION_GROUP_OK index=%u checked=1 siblings_cleared=1\n",i);}
        else if(wcsstr(cls,L"CheckBox")){++checkboxes;if(!IsWindowEnabled(w))continue;LRESULT original=SendMessageW(w,BM_GETCHECK,0,0);SendMessageW(w,BM_CLICK,0,0);CHECK(swprintf_s(path,MAX_PATH,L"%ls/selected-%u.bmp",scratch,i)>0);(void)capture(w,path,!wcscmp(argv[3],L"verify"));SendMessageW(w,BM_CLICK,0,0);CHECK(SendMessageW(w,BM_GETCHECK,0,0)==original);printf("SAMPLE_CHECK_CYCLE_OK index=%u round2=initial\n",i);}
        else if(wcsstr(cls,L"ComboBox")){++combos;LRESULT n=SendMessageW(w,CB_GETCOUNT,0,0);CHECK(n>1);SendMessageW(w,CB_SETCURSEL,1,0);CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==1);SendMessageW(w,CB_SHOWDROPDOWN,TRUE,0);CHECK(SendMessageW(w,CB_GETDROPPEDSTATE,0,0));SendMessageW(w,CB_SHOWDROPDOWN,FALSE,0);CHECK(!SendMessageW(w,CB_GETDROPPEDSTATE,0,0));printf("SAMPLE_COMBO_SELECT_OK index=%u count=%lld select1=1 drop_cancel=1\n",i,(long long)n);}
        else if(wcsstr(cls,L"ListBox")){++lists;LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);CHECK(n>1 && SendMessageW(w,LB_SETCURSEL,1,0)!=LB_ERR && SendMessageW(w,LB_GETCURSEL,0,0)==1);printf("SAMPLE_LIST_SELECT_OK index=%u count=%lld select1=1\n",i,(long long)n);}
        else if(wcsstr(cls,L"TextBox")){++edits;WCHAR original[512],actual[512];CHECK(SendMessageW(w,WM_GETTEXT,512,(LPARAM)original));CHECK(SendMessageW(w,WM_SETTEXT,0,(LPARAM)L"Native sample probe"));SendMessageW(w,WM_GETTEXT,512,(LPARAM)actual);CHECK(!wcscmp(actual,L"Native sample probe"));CHECK(SendMessageW(w,WM_SETTEXT,0,(LPARAM)original));SendMessageW(w,WM_GETTEXT,512,(LPARAM)actual);CHECK(!wcscmp(original,actual));printf("SAMPLE_EDIT_OK index=%u text_restore=1 multiline=%u cross_process_replace_undo=not_covered\n",i,(style&ES_MULTILINE)!=0);}
        else if(wcsstr(cls,L"ScrollBar")){++bars;SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};CHECK(GetScrollInfo(w,SB_CTL,&info));printf("SAMPLE_SCROLL_OK index=%u min=%d max=%d pos=%d page=%u\n",i,info.nMin,info.nMax,info.nPos,info.nPage);}
        if(queue_state && wcsstr(cls,L"ComboBox")){
            CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==1);queue_key(w,VK_DOWN);
            CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==2);
            CHECK(queue_state->commands==2 && queue_state->command_code==CBN_SELCHANGE);
            printf("VB_SAMPLE_COMBO_INPUT_OK index=%u selected=2 commands=%ld\n",i,queue_state->commands);
        }else if(queue_state && wcsstr(cls,L"ListBox")){
            LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);queue_key(w,VK_END);
            CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==n-1);
            CHECK(queue_state->commands==1 && queue_state->command_code==LBN_SELCHANGE);
            printf("VB_SAMPLE_LIST_INPUT_OK index=%u selected=%lld commands=%ld\n",i,(long long)n-1,queue_state->commands);
        }else if(queue_state && wcsstr(cls,L"ScrollBar")){
            SCROLLINFO state={.cbSize=sizeof(state),.fMask=SIF_ALL};CHECK(GetScrollInfo(w,SB_CTL,&state));
            int original=state.nPos;queue_key(w,VK_END);CHECK(GetScrollInfo(w,SB_CTL,&state));
            CHECK(state.nPos>original && state.nPos<=state.nMax);int end=state.nPos;
            queue_key(w,VK_HOME);CHECK(GetScrollInfo(w,SB_CTL,&state) && state.nPos==state.nMin);
            CHECK(queue_state->scroll_code==SB_ENDSCROLL &&
                ((style&SBS_VERT)?queue_state->vscroll==2 && !queue_state->hscroll:queue_state->hscroll==2 && !queue_state->vscroll));
            printf("VB_SAMPLE_SCROLL_INPUT_OK index=%u end=%d home=%d hscroll=%ld vscroll=%ld\n",i,end,state.nPos,queue_state->hscroll,queue_state->vscroll);
        }
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==original_font && GetWindowRect(w,&after_rect) && EqualRect(&original_rect,&after_rect));
    }
    CHECK(options==4 && checkboxes==4 && combos==2 && lists==1 && edits==2 && bars==2);
    printf("SAMPLE_PROTOCOL_PASS option=%u checkbox=%u combo=%u list=%u edit=%u scrollbar=%u\n",options,checkboxes,combos,lists,edits,bars);
    }else {
        if(graphical){
            CHECK(queue_state);unsigned found=0;
            for(unsigned i=0;i<count;++i){HWND w=checks[i];WCHAR cls[100];GetClassNameW(w,cls,100);
                if((wcsstr(cls,L"CommandButton") || wcsstr(cls,L"CheckBox") || wcsstr(cls,L"OptionButton")) &&
                        (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW){
                    HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT before,after,client_before,client_after;
                    LRESULT button_state=SendMessageW(w,BM_GETSTATE,0,0);
                    CHECK(GetWindowRect(w,&before) && GetClientRect(w,&client_before));
                    queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);++found;
                    CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);
                    if(graphical_verify)CHECK(styled?(queue_state->attached && queue_state->system_text>0):!queue_state->system_text);
                    /* Optional source-reviewed 3Ddefline: ten image-only
                     * commands. Never use this mode to click business code. */
                    if(graphical_pictures)CHECK(!wcscmp(cls,L"ThunderRT6CommandButton") && !GetWindowTextLengthW(w) &&
                        queue_state->graphical_frames==1 && queue_state->graphical_type==DFC_BUTTON &&
                        queue_state->graphical_state==DFCS_BUTTONPUSH && (styled?queue_state->attached:queue_state->attached==-1));
                    printf("VB_GRAPHICAL_API index=%u class=%ls attached=%ld frames=%ld type=%lu state=%lu rect=%ld,%ld,%ld,%ld dc_window=%p blits=%ld text=%ld restored=%ld\n",i,cls,queue_state->attached,queue_state->graphical_frames,queue_state->graphical_type,queue_state->graphical_state,queue_state->graphical_rect.left,queue_state->graphical_rect.top,queue_state->graphical_rect.right,queue_state->graphical_rect.bottom,(void *)queue_state->graphical_dc_window,queue_state->graphical_blits,queue_state->graphical_text,queue_state->paint_restored);
                    /* IAT wrappers change the direct caller; inspect actual
                     * style pixels after restoring imports, without wrappers. */
                    graphical_pixel_check=(graphical_verify || graphical_pictures) && styled;
                    CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u.bmp",scratch,i)>0);uint64_t original_image=capture(w,path,FALSE);graphical_pixel_check=FALSE;
                    if(graphical_pictures){
                        LONG commands=queue_state->commands;CHECK(IsWindowEnabled(w) && !(button_state&BST_PUSHED));
                        EnableWindow(w,FALSE);CHECK(!IsWindowEnabled(w));queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);
                        CHECK(queue_state->paint_restored && !queue_state->paint_probe_error && (queue_state->graphical_state&DFCS_INACTIVE));
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-disabled.bmp",scratch,i)>0);(void)capture(w,path,FALSE);
                        EnableWindow(w,TRUE);SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED);
                        queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);
                        CHECK(queue_state->paint_restored && !queue_state->paint_probe_error && (queue_state->graphical_state&DFCS_PUSHED));
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-pressed.bmp",scratch,i)>0);(void)capture(w,path,FALSE);
                        SendMessageW(w,BM_SETSTATE,FALSE,0);
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-restored.bmp",scratch,i)>0);
                        CHECK(capture(w,path,FALSE)==original_image);queue_survey(w,0);
                        CHECK(queue_state->commands==commands && !GetCapture());
                        RECT root_rect;CHECK(GetWindowRect(form,&root_rect));
                        printf("VB_GRAPHICAL_PICTURE_STATES_OK index=%u disabled=1 pressed=1 restored_pixels=1 no_click=1 root_offset=%ld,%ld size=%ld,%ld\n",
                            i,before.left-root_rect.left,before.top-root_rect.top,before.right-before.left,before.bottom-before.top);
                    }
                    queue_survey(w,VB_QUEUE_PAINT_SURVEY);
                    if(graphical_verify)CHECK(styled?queue_state->paint_system>0:!queue_state->paint_system);
                    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && SendMessageW(w,BM_GETSTATE,0,0)==button_state &&
                        GetWindowRect(w,&after) && GetClientRect(w,&client_after) && EqualRect(&before,&after) && EqualRect(&client_before,&client_after));
                    printf("VB_GRAPHICAL_PROPERTIES_OK index=%u source_font=%p geometry_state_preserved=1 dpi=%u\n",i,(void *)source,GetDpiForWindow(w));
                    /* These optional Mouse sample buttons have no Click
                     * handler; their native notification is safe to observe. */
                    if(graphical_verify && IsWindowEnabled(w)){
                        LONG commands=queue_state->commands;SendMessageW(w,BM_CLICK,0,0);queue_survey(w,0);
                        CHECK(queue_state->commands==commands+1 && queue_state->command_code==BN_CLICKED);
                        printf("VB_GRAPHICAL_CLICK_OK index=%u native_parent_notification=1\n",i);
                    }
                }
            }
            CHECK(found>0 && (!graphical_pictures || found==10));printf("VB_GRAPHICAL_INVENTORY_OK controls=%u\n",found);
        }
        if(about_print){
            CHECK(queue_state);HWND main=form,button=NULL;
            CHECK(!wcscpy_s(about_capture_dir,MAX_PATH,scratch));
            for(unsigned i=0;i<count;++i){WCHAR text[80];GetWindowTextW(checks[i],text,80);if(!wcscmp(text,L"About..."))button=checks[i];}
            CHECK(button);queue_survey(button,VB_QUEUE_PRINT_SAMPLE);
            CHECK(queue_state->print_calls>0 && queue_state->print_api==3);
            CHECK(EnumThreadWindows(queue_state->tid,close_about,(LPARAM)main) && about_closed==1);form=main;
        }
        if(containers){
            CHECK(queue_state);queue_survey(form,VB_QUEUE_PAINT_SURVEY);
            unsigned pictures=0,label_hosts=0;
            for(unsigned i=0;i<count;++i){WCHAR cls[100];GetClassNameW(checks[i],cls,100);
                if(wcsstr(cls,L"PictureBox") || wcsstr(cls,L"Frame")){
                    HFONT font=(HFONT)SendMessageW(checks[i],WM_GETFONT,0,0);RECT before,after,client_before,client_after;
                    CHECK(GetWindowRect(checks[i],&before) && GetClientRect(checks[i],&client_before));
                    queue_survey(checks[i],VB_QUEUE_PAINT_SURVEY);
                    if(wcsstr(cls,L"PictureBox")){
                        ++pictures;if(queue_state->paint_system>0)++label_hosts;
                        if(container_verify)CHECK(styled?queue_state->attached==1:queue_state->paint_system==0);
                    }
                    CHECK((HFONT)SendMessageW(checks[i],WM_GETFONT,0,0)==font && GetWindowRect(checks[i],&after) && GetClientRect(checks[i],&client_after) && EqualRect(&before,&after) && EqualRect(&client_before,&client_after));
                    CHECK(swprintf_s(path,MAX_PATH,L"%ls/container-%u.bmp",scratch,i)>0);(void)capture(checks[i],path,FALSE);
                }
            }
            if(container_verify){CHECK(pictures>0 && (styled?label_hosts>0:label_hosts==0));printf("VB_REAL_CONTAINERS_OK pictures=%u system_label_hosts=%u font_geometry_preserved=1\n",pictures,label_hosts);}
        }
        unsigned file_controls=0;CHECK(!file_verify || queue_state);
        if(queue_state)for(unsigned i=0;i<count;++i){
            WCHAR cls[100];GetClassNameW(checks[i],cls,100);
            if(wcsstr(cls,L"FileListBox") || wcsstr(cls,L"DirListBox") || wcsstr(cls,L"DriveListBox")){
                HWND w=checks[i];BOOL drive=wcsstr(cls,L"DriveListBox")!=NULL;
                HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT before,after;CHECK(GetWindowRect(w,&before));
                LRESULT selection=SendMessageW(w,drive?CB_GETCURSEL:LB_GETCURSEL,0,0);
                queue_survey(w,drive?VB_QUEUE_POPUP_SURVEY:VB_QUEUE_SURVEY);
                if(file_verify){
                    CHECK((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime);
                    CHECK(styled?(queue_state->attached==1 && queue_state->system_text>0):queue_state->system_text==0);
                    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && GetWindowRect(w,&after) && EqualRect(&before,&after));
                    CHECK(SendMessageW(w,drive?CB_GETCURSEL:LB_GETCURSEL,0,0)==selection);
                    if(drive){SendMessageW(w,CB_SHOWDROPDOWN,TRUE,0);queue_key(w,VK_ESCAPE);
                        CHECK(!SendMessageW(w,CB_GETDROPPEDSTATE,0,0) && SendMessageW(w,CB_GETCURSEL,0,0)==selection);}
                    if(filedir_verify && wcsstr(cls,L"FileListBox")){
                        LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);CHECK(n>1);
                        queue_key(w,VK_END);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==n-1 && queue_state->commands>0);
                        queue_key(w,VK_HOME);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==0 && queue_state->commands>0);
                        SendMessageW(w,LB_SETCURSEL,(WPARAM)selection,0);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==selection);
                        puts("VB_REAL_FILE_INPUT_OK queued_end_home=1 parent_notifications=1 selection_restored=1");
                    }
                    ++file_controls;
                }
            }
        }
        if(file_verify){CHECK(file_controls==(filedir_verify?2u:3u));printf("VB_REAL_FILES_OK runtime_owned=%u styled=%u actual_system_controls=%u font_geometry_selection_preserved=1 drive_escape=%u\n",file_controls,styled,styled?file_controls:0,!filedir_verify);}
        printf("VB_SAMPLE_INVENTORY_OK windows=%u selection_unchanged=1\n",count);
    }

    if(queue_state){CHECK(UnhookWindowsHookEx(queue_get) && UnhookWindowsHookEx(queue_sent));queue_get=queue_sent=NULL;}
    SendMessageW(form,WM_CLOSE,0,0);
    CHECK(WaitForSingleObject(target,5000)==WAIT_OBJECT_0 && WaitForSingleObject(p.hProcess,5000)==WAIT_OBJECT_0);
    DWORD code;
    CHECK(GetExitCodeProcess(target,&code) && !code);
    BOOL zero=FALSE;
    ULONGLONG deadline=GetTickCount64()+3000;
    while(!zero){
        ULONGLONG now=GetTickCount64();if(now>=deadline)break;
        DWORD message;
        ULONG_PTR key;
        OVERLAPPED *overlap;
        CHECK(GetQueuedCompletionStatus(port,&message,&key,&overlap,(DWORD)(deadline-now)));
        if(key==(ULONG_PTR)job && message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)zero=TRUE;
    }
    CHECK(zero);
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION active;
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&active,sizeof(active),NULL) && !active.ActiveProcesses);
    CloseHandle(target);
    CloseHandle(p.hProcess);
    CloseHandle(p.hThread);
    UnhookWinEvent(hook);
    CloseHandle(ready);
    CloseHandle(job);
    CloseHandle(port);
    if(queue_state){
        CHECK(FreeLibrary(queue_module) && UnmapViewOfFile(queue_state));queue_state=NULL;
        CHECK(CloseHandle(queue_ready) && CloseHandle(queue_mapping));
        CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,NULL));
        puts("VB_SAMPLE_QUEUE_CLEANUP_OK owned_thread_hooks=0 mappings=0 events=0 active=0");
    }
    printf("SAMPLE_PASS styled=%d count=%u active=0\n",styled,count);
    return 0;
}
int wmain(int argc,WCHAR **argv){
    return isolated_process(argc,argv,worker);
}
