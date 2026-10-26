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
#include <oleacc.h>
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
static DWORD pid;
static HWND checks[80];
static unsigned count;
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
    printf("SAMPLE_WINDOW hwnd=%p parent=%p class=%ls parent_class=%ls owner=%p style=%08lx ex=%08lx text=%ls font=%p\n",(void *)w,(void *)parent,cls,parent_class,(void *)GetClassLongPtrW(w,GCLP_HMODULE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),text,(void *)SendMessageW(w,WM_GETFONT,0,0));
    if(wcsstr(cls,L"OptionButton") || wcsstr(cls,L"CheckBox"))printf("SAMPLE_BUTTON hwnd=%p dlg=%llx check=%lld state=%lld\n",(void *)w,(unsigned long long)SendMessageW(w,WM_GETDLGCODE,0,0),(long long)SendMessageW(w,BM_GETCHECK,0,0),(long long)SendMessageW(w,BM_GETSTATE,0,0));
    if(wcsstr(cls,L"ComboBox")) {COMBOBOXINFO i={.cbSize=sizeof(i)};BOOL ok=GetComboBoxInfo(w,&i);printf("SAMPLE_COMBO hwnd=%p count=%lld selected=%lld info=%d edit=%p list=%p error=%lu\n",(void *)w,(long long)SendMessageW(w,CB_GETCOUNT,0,0),(long long)SendMessageW(w,CB_GETCURSEL,0,0),ok,(void *)i.hwndItem,(void *)i.hwndList,GetLastError());}
    if(wcsstr(cls,L"ListBox"))printf("SAMPLE_LIST hwnd=%p count=%lld selected=%lld\n",(void *)w,(long long)SendMessageW(w,LB_GETCOUNT,0,0),(long long)SendMessageW(w,LB_GETCURSEL,0,0));
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
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
    CHECK(snap!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W mod= {.dwSize=sizeof(mod)};
    HMODULE runtime=NULL;
    BOOL dll=FALSE;
    CHECK(Module32FirstW(snap,&mod));
    do{
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
    CHECK(runtime && dll==styled && EnumChildWindows(form,collect,0) && count>10);
    CHECK(swprintf_s(path,MAX_PATH,L"%ls/actor-%ls.bmp",scratch,styled?L"styled":L"bare")>0);
    printf("VB_SAMPLE_IMAGE hash=%llu\n",(unsigned long long)capture(form,path,FALSE));
    printf("SAMPLE_ROOT class_owner=%p windows=%u\n",(void *)GetClassLongPtrW(form,GCLP_HMODULE),count);
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
