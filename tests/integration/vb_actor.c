/* Optional real VB6 application observer; paths are explicit arguments.
 * Own private desktop/Job, native API observations, no existing-process access. */
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include "../ntw16s-dll/isolated_process.h"
#include "../../src/ntw16s-dll/control_glyphs.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ACTOR_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static void file_path(const WCHAR *path){
    HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    CHECK(f!=INVALID_HANDLE_VALUE);
    WCHAR actual[32768];
    CHECK(GetFinalPathNameByHandleW(f,actual,32768,FILE_NAME_NORMALIZED));
    printf("ACTOR_MODULE_FILE actual=%ls\n",actual);
    CloseHandle(f);
}
static HANDLE job,ready,port;
static HWND form;
static DWORD pid;
static HWND checks[4];
static unsigned count;
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
static void CALLBACK shown(HWINEVENTHOOK h,DWORD e,HWND w,LONG object,LONG child,DWORD thread,DWORD time){
    (void)h;
    (void)e;
    (void)thread;
    (void)time;
    WCHAR cls[80];
    if(object!=OBJID_WINDOW || child || !GetClassNameW(w,cls,80) || wcscmp(cls,L"ThunderRT6FormDC"))return;
    DWORD candidate;
    GetWindowThreadProcessId(w,&candidate);
    HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,candidate);
    BOOL own=FALSE;
    if(p){
        IsProcessInJob(p,job,&own);
        CloseHandle(p);
    }
    if(own){
        form=w;
        pid=candidate;
        SetEvent(ready);
    }
}
static BOOL CALLBACK collect(HWND w,LPARAM ignored){
    (void)ignored;
    WCHAR cls[80];
    GetClassNameW(w,cls,80);
    if(!wcscmp(cls,L"ThunderRT6CheckBox") && count<4)checks[count++]=w;
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
    if(verify){RECT client;CHECK(GetClientRect(w,&client));int dpi=(int)GetDpiForWindow(w);int top=(client.bottom-MulDiv(13,dpi,96))/2;CHECK(top>=0 && cx>=MulDiv(14,dpi,96) && cy>=top+MulDiv(13,dpi,96));
        LRESULT check=SendMessageW(w,BM_GETCHECK,0,0);CHECK(check==0 || check==1);DWORD *pixels=data;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){DWORD expected=control_checks[y][x+(check?14:0)]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,dpi,96))*cx+MulDiv(x,dpi,96)]&0xffffff)==expected);}
        printf("ACTOR_GLYPH_OK source_pixels=182 check=%lld\n",(long long)check);
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
    CHECK(argc==4);
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
    DWORD size=MAX_PATH;
    CHECK(QueryFullProcessImageNameW(target,0,path,&size));
    BOOL wow=FALSE;CHECK(IsWow64Process(target,&wow) && wow);
    HANDLE process_snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);CHECK(process_snapshot!=INVALID_HANDLE_VALUE);
    PROCESSENTRY32W entry= {.dwSize=sizeof(entry)};DWORD parent=0;CHECK(Process32FirstW(process_snapshot,&entry));
    do{if(entry.th32ProcessID==pid){parent=entry.th32ParentProcessID;break;}}while(Process32NextW(process_snapshot,&entry));CloseHandle(process_snapshot);
    CHECK(parent==(styled?p.dwProcessId:GetCurrentProcessId()));
    printf("ACTOR_TARGET pid=%lu creator=%lu initial_pid=%lu bits=32 path=%ls dpi=%u\n",pid,parent,p.dwProcessId,path,GetDpiForWindow(form));
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
    CHECK(snap!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W mod= {.dwSize=sizeof(mod)};
    HMODULE runtime=NULL;
    BOOL dll=FALSE;
    CHECK(Module32FirstW(snap,&mod));
    do{
        if(!_wcsicmp(mod.szModule,L"MSVBVM60.DLL")){
            runtime=mod.hModule;
            printf("ACTOR_RUNTIME base=%p path=%ls\n",(void *)runtime,mod.szExePath);
            file_path(mod.szExePath);
        }
        if(!_wcsicmp(mod.szModule,L"ntw16s32.dll")){
            dll=TRUE;
            printf("ACTOR_STYLE_MODULE base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);
            file_path(mod.szExePath);
        }
    }
    while(Module32NextW(snap,&mod));
    CloseHandle(snap);
    CHECK(runtime && dll==styled && EnumChildWindows(form,collect,0) && count==2);
    CHECK(swprintf_s(path,MAX_PATH,L"%ls/actor-%ls.bmp",scratch,styled?L"styled":L"bare")>0);
    printf("ACTOR_IMAGE hash=%llu\n",(unsigned long long)capture(form,path,FALSE));
    for(unsigned i=0; i<count; ++i){
        HWND w=checks[i];
        CHECK((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime);
        LRESULT original=SendMessageW(w,BM_GETCHECK,0,0);HFONT source_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);LONG_PTR source_style=GetWindowLongPtrW(w,GWL_STYLE);
        printf("ACTOR_CHECK index=%u hwnd=%p owner=%p style=%08lx dlgcode=%llx check=%lld state=%lld font=%p\n",i,(void *)w,(void *)GetClassLongPtrW(w,GCLP_HMODULE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(unsigned long long)SendMessageW(w,WM_GETDLGCODE,0,0),(long long)original,(long long)SendMessageW(w,BM_GETSTATE,0,0),(void *)SendMessageW(w,WM_GETFONT,0,0));
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/check-%ls-%u.bmp",scratch,styled?L"styled":L"bare",i)>0);
        printf("ACTOR_CHECK_IMAGE index=%u hash=%llu\n",i,(unsigned long long)capture(w,path,!wcscmp(argv[3],L"verify")));
        if(!wcscmp(argv[3],L"interact") || !wcscmp(argv[3],L"verify")){
            for(int j=0; j<4; ++j){
                SendMessageW(w,BM_CLICK,0,0);
                printf("ACTOR_CLICK index=%u round=%d check=%lld state=%lld\n",i,j,(long long)SendMessageW(w,BM_GETCHECK,0,0),(long long)SendMessageW(w,BM_GETSTATE,0,0));
            }
            CHECK(SendMessageW(w,BM_GETCHECK,0,0)==original && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==source_font && GetWindowLongPtrW(w,GWL_STYLE)==source_style);
        }
    }
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
    printf("ACTOR_PASS styled=%d count=%u active=0\n",styled,count);
    return 0;
}
int wmain(int argc,WCHAR **argv){
    return isolated_process(argc,argv,worker);
}
