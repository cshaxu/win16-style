#include "ntw16s-dll/injection/injection.h"
#include "ntw16s-dll/injection/staging.h"
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"INSTALL FAILURE TEST line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);goto failed;}}while(0)
typedef struct owned_child {PROCESS_INFORMATION p;} owned_child;
static void owned_close(owned_child *child)
{
    if(child->p.hProcess){
        if(WaitForSingleObject(child->p.hProcess,0)==WAIT_TIMEOUT){
            TerminateProcess(child->p.hProcess,ERROR_CANCELLED);WaitForSingleObject(child->p.hProcess,2000);
        }
        CloseHandle(child->p.hProcess);
    }
    if(child->p.hThread)CloseHandle(child->p.hThread);
    ZeroMemory(&child->p,sizeof(child->p));
}
static WCHAR self[NTW16S_PATH_CAP];
#ifdef _WIN64
static DWORD helper_mode,helper_calls;
static owned_child helper_observer;
static HANDLE controlled_target;
static BOOL WINAPI controlled_helper(LPCWSTR application,LPWSTR ignored_command,LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID environment,LPCWSTR directory,
    LPSTARTUPINFOW startup,LPPROCESS_INFORMATION process)
{
    (void)ignored_command;++helper_calls;
    const WCHAR suffix[]=L"\\syswow64\\rundll32.exe";size_t n=wcslen(application);
    if(n<wcslen(suffix) || _wcsicmp(application+n-wcslen(suffix),suffix) || !(flags&CREATE_SUSPENDED)){
        SetLastError(ERROR_INVALID_NAME);return FALSE;}
    if(helper_mode==1){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    if(helper_mode==3 && (!TerminateProcess(controlled_target,37) ||
        WaitForSingleObject(controlled_target,2000)!=WAIT_OBJECT_0))return FALSE;
    WCHAR command[32768];if(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --stall",self)<0){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    if(!CreateProcessW(self,command,pa,ta,inherit,flags,environment,directory,startup,process))return FALSE;
    return DuplicateHandle(GetCurrentProcess(),process->hProcess,GetCurrentProcess(),&helper_observer.p.hProcess,0,FALSE,DUPLICATE_SAME_ACCESS);
}
#endif
static BOOL spawn(owned_child *child,PCWSTR image,PCWSTR tail)
{
    WCHAR command[32768];STARTUPINFOW startup={.cb=sizeof(startup)};
    if(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,tail)<0)return FALSE;
    return CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child->p);
}
static BOOL suspended(owned_child *child)
{
    DWORD wait=WaitForSingleObject(child->p.hProcess,0),before=SuspendThread(child->p.hThread);
    DWORD during=before==(DWORD)-1?(DWORD)-1:ResumeThread(child->p.hThread);
    if(wait!=WAIT_TIMEOUT || before!=1 || during!=2)
        fprintf(stderr,"SUSPENSION wait=%lu before=%lu during=%lu\n",wait,before,during);
    return wait==WAIT_TIMEOUT && before==1 && during==2;
}
int wmain(int argc,WCHAR **argv)
{
    owned_child children[16]={0};unsigned count=0;int result=1;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    if(argc==3 && !wcscmp(argv[1],L"--entry")){
        HANDLE marker=CreateFileW(argv[2],GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(marker!=INVALID_HANDLE_VALUE)CloseHandle(marker);
        return 42;
        }
    if(argc==2 && !wcscmp(argv[1],L"--stall")){
        HANDLE never=CreateEventW(NULL,TRUE,FALSE,NULL);if(!never)return 1;
        WaitForSingleObject(never,INFINITE);CloseHandle(never);return 0;}
    CHECK(argc==2 && GetModuleFileNameW(NULL,self,NTW16S_PATH_CAP));
    ntw16s_context context;CHECK(!ntw16s_context_paths(&context,NULL));
    {owned_child *warm=&children[count++];CHECK(spawn(warm,self,L""));owned_close(warm);}
    DWORD before=0,after=0;CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    {
        owned_child *child=&children[count++];HANDLE restricted=NULL;CHECK(spawn(child,self,L""));
        restricted=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,child->p.dwProcessId);CHECK(restricted);
        DWORD error=ntw16s_install(restricted,&context,CreateProcessW);CloseHandle(restricted);
        if(error!=ERROR_ACCESS_DENIED)fprintf(stderr,"RESTRICTED install error=%lu\n",error);
        CHECK(error==ERROR_ACCESS_DENIED && suspended(child));
    }
    {
        WCHAR scratch[MAX_PATH],marker[MAX_PATH],tail[MAX_PATH+20];owned_child *child=&children[count++];
        CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
        CHECK(swprintf_s(marker,MAX_PATH,L"%ls\\invalid-payload-entry.txt",scratch)>0);
        CHECK(GetFileAttributesW(marker)==INVALID_FILE_ATTRIBUTES);
        CHECK(swprintf_s(tail,ARRAYSIZE(tail),L"--entry \"%ls\"",marker)>0 && spawn(child,self,tail));
        BYTE data[65536];ntw16s_packet h={0};h.version=99;h.flags=1;
        h.machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
        h.directory_offset=sizeof(h);h.directory_bytes=(DWORD)((wcslen(context.directory)+1)*sizeof(WCHAR));h.bytes=sizeof(h)+h.directory_bytes;
        memcpy(data,&h,sizeof(h));memcpy(data+sizeof(h),context.directory,h.directory_bytes);
        WCHAR dll[NTW16S_PATH_CAP];char dll_a[4096];
        CHECK(swprintf_s(dll,NTW16S_PATH_CAP,L"%lsntw16s%u.dll",context.directory,(unsigned)(sizeof(void *)*8))>0);
        CHECK(WideCharToMultiByte(CP_ACP,0,dll,-1,dll_a,sizeof(dll_a),NULL,NULL));
        CHECK(!ntw16s_stage_process(child->p.hProcess,dll_a,data,h.bytes));
        CHECK(ResumeThread(child->p.hThread)==1 && WaitForSingleObject(child->p.hProcess,10000)==WAIT_OBJECT_0);
        DWORD code;CHECK(GetExitCodeProcess(child->p.hProcess,&code) && code==0xc0000142u);
        CHECK(GetFileAttributesW(marker)==INVALID_FILE_ATTRIBUTES);
    }
#ifdef _WIN64
    ntw16s_context foreign;CHECK(GetFullPathNameW(argv[1],NTW16S_PATH_CAP,foreign.directory,NULL));
    WCHAR *slash=wcsrchr(foreign.directory,L'\\');CHECK(slash);slash[1]=0;foreign.flags=1;
    for(helper_mode=1;helper_mode<=3;++helper_mode){
        owned_child *child=&children[count++];CHECK(spawn(child,argv[1],L""));helper_calls=0;
        controlled_target=child->p.hProcess;
        ULONGLONG start=GetTickCount64();DWORD error=ntw16s_install(child->p.hProcess,&foreign,controlled_helper);
        ULONGLONG elapsed=GetTickCount64()-start;
        CHECK(helper_calls==1);
        if(helper_mode!=3)CHECK(suspended(child));
        DWORD expected_error=helper_mode==1?ERROR_ACCESS_DENIED:helper_mode==2?ERROR_TIMEOUT:ERROR_PROCESS_ABORTED;CHECK(error==expected_error);
        if(helper_mode==2){
            CHECK(elapsed>=9000 && elapsed<15000 && helper_observer.p.hProcess);
            CHECK(WaitForSingleObject(helper_observer.p.hProcess,5000)==WAIT_OBJECT_0);
            owned_close(&helper_observer);printf("HELPER timeout PASS elapsed=%llu target suspension retained, owned helper exited\n",elapsed);
        }
        if(helper_mode==3){
            DWORD code;CHECK(elapsed<5000 && WaitForSingleObject(child->p.hProcess,0)==WAIT_OBJECT_0 &&
                GetExitCodeProcess(child->p.hProcess,&code) && code==37);
            CHECK(helper_observer.p.hProcess && WaitForSingleObject(helper_observer.p.hProcess,5000)==WAIT_OBJECT_0);
            owned_close(&helper_observer);puts("HELPER target-first-exit PASS error=1067, target exit=37, owned helper exited");
        }
    }
#endif
    for(unsigned i=0;i<count;++i)owned_close(&children[i]);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && after==before);
    printf("INSTALL FAILURE PASS width=%u restricted handle, refused DLL initialization/no entry, handle count=%lu/%lu\n",(unsigned)(sizeof(void *)*8),before,after);result=0;
failed:
    for(unsigned i=0;i<count;++i)owned_close(&children[i]);
#ifdef _WIN64
    owned_close(&helper_observer);
#endif
    return result;
}
