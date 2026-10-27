#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "../ntw16s-dll/isolated_process.h"
#include "vb_control_text.h"

#define CHECK(x) do { if(!(x)){fprintf(stderr,"VB_TEXT_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)

static int target(HANDLE map,HANDLE ready)
{
    HWND *shared=MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(HWND));
    CHECK(shared);
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(parent);
    HWND edit=CreateWindowW(L"EDIT",L"initial",WS_CHILD|WS_VISIBLE,0,0,100,25,parent,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(edit);*shared=edit;MemoryBarrier();CHECK(SetEvent(ready));
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);if(!IsWindow(edit))break;}
    if(IsWindow(parent))DestroyWindow(parent);
    CHECK(UnmapViewOfFile(shared));return 0;
}

static int worker(int argc,WCHAR **argv)
{
    (void)argv;CHECK(argc==1);SECURITY_ATTRIBUTES attr={sizeof(attr),NULL,TRUE};
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,&attr,PAGE_READWRITE,0,sizeof(HWND),NULL);
    HANDLE ready=CreateEventW(&attr,TRUE,FALSE,NULL);CHECK(map && ready);
    HWND *shared=MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(HWND));CHECK(shared);*shared=NULL;
    HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
    WCHAR self[MAX_PATH],command[MAX_PATH+100];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --target %llu %llu",self,(unsigned long long)(ULONG_PTR)map,(unsigned long long)(ULONG_PTR)ready)>0);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    CHECK(CreateProcessW(self,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&pi));
    CHECK(AssignProcessToJobObject(job,pi.hProcess) && ResumeThread(pi.hThread)==1);
    CHECK(WaitForSingleObject(ready,3000)==WAIT_OBJECT_0);MemoryBarrier();HWND edit=*shared;CHECK(IsWindow(edit));
    CHECK(SendMessageW(edit,WM_SETTEXT,0,(LPARAM)L"Updated \x03a9"));
    WCHAR actual[80],legacy[80];CHECK(vb_control_text(edit,actual,ARRAYSIZE(actual)));
    CHECK(!wcscmp(actual,L"Updated \x03a9"));GetWindowTextW(edit,legacy,ARRAYSIZE(legacy));
    CHECK(!vb_control_text(NULL,actual,ARRAYSIZE(actual)));
    CHECK(PostMessageW(edit,WM_CLOSE,0,0));CHECK(WaitForSingleObject(pi.hProcess,3000)==WAIT_OBJECT_0);
    DWORD exit_code=1;CHECK(GetExitCodeProcess(pi.hProcess,&exit_code) && !exit_code);
    CHECK(CloseHandle(pi.hThread) && CloseHandle(pi.hProcess) && CloseHandle(job));
    CHECK(UnmapViewOfFile(shared) && CloseHandle(ready) && CloseHandle(map));
    printf("VB_CONTROL_TEXT_OK bits=%u cross_process=1 unicode=1 invalid_window=1 cleanup=1 legacy_equal=%d\n",(unsigned)(sizeof(void*)*8),!wcscmp(legacy,L"Updated \x03a9"));return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"--target"))return target((HANDLE)(ULONG_PTR)_wcstoui64(argv[2],NULL,10),(HANDLE)(ULONG_PTR)_wcstoui64(argv[3],NULL,10));
    return isolated_process(argc,argv,worker);
}
