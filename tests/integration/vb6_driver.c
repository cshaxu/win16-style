#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include "../ntw16s-dll/isolated_process.h"
static HWND form;
static HANDLE job;
static BOOL CALLBACK find_form(HWND w,LPARAM ignored)
{
    DWORD pid;BOOL owned=FALSE;RECT r;WCHAR cls[80];(void)ignored;
    GetWindowThreadProcessId(w,&pid);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(process){IsProcessInJob(process,job,&owned);CloseHandle(process);}
    if(!owned)return TRUE;
    GetClassNameW(w,cls,80);GetWindowRect(w,&r);
    if(!wcscmp(cls,L"ThunderRT6FormDC") && !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) &&
            r.right-r.left>200 && r.bottom-r.top>200){form=w;return FALSE;}
    return TRUE;
}
static BOOL capture(HWND w,const WCHAR *path)
{
    DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext(w));
    RECT r;GetWindowRect(w,&r);int cx=r.right-r.left,cy=r.bottom-r.top;
    HDC dc=GetDC(NULL),memory=CreateCompatibleDC(dc);void *bits=NULL;
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=cx;
    info.bmiHeader.biHeight=-cy;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    HGDIOBJ old=SelectObject(memory,bitmap);BOOL ok=PrintWindow(w,memory,0);
    if(ok){FILE *file=_wfopen(path,L"wb");BITMAPFILEHEADER header={0};
        header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);
        header.bfSize=header.bfOffBits+(DWORD)(cx*cy*4);
        ok=file && fwrite(&header,sizeof(header),1,file)==1 && fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file)==1 &&
            fwrite(bits,(size_t)cx*cy*4,1,file)==1;
        if(file)fclose(file);}
    SelectObject(memory,old);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(NULL,dc);
    SetThreadDpiAwarenessContext(previous);return ok;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc!=5){fwprintf(stderr,L"usage: driver launcher-or-bare target probe output-prefix\n");return 2;}
    WCHAR command[2048],event_name[128],log[MAX_PATH],bmp[MAX_PATH];DWORD exit_code=1,pid=0;
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};HHOOK hook=NULL;HMODULE module=NULL;
    HANDLE ready=NULL,completion=NULL,target=NULL;int result=1;
    swprintf_s(event_name,128,L"Local\\VB6Ready-%lu",GetCurrentProcessId());
    swprintf_s(log,MAX_PATH,L"%ls.txt",argv[4]);swprintf_s(bmp,MAX_PATH,L"%ls.bmp",argv[4]);
    SetEnvironmentVariableW(L"WIN16STYLE_VB_LOG",log);SetEnvironmentVariableW(L"WIN16STYLE_VB_READY",event_name);
    ready=CreateEventW(NULL,TRUE,FALSE,event_name);job=CreateJobObjectW(NULL,NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!ready || !job || !SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)))goto cleanup;
    completion=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT associate={job,completion};
    if(!completion || !SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&associate,sizeof(associate)))goto cleanup;
    if(!wcscmp(argv[1],L"bare"))swprintf_s(command,2048,L"\"%ls\"",argv[2]);
    else swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\"",argv[1],argv[2]);
    si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_SHOWNORMAL;
    if(!CreateProcessW(NULL,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&pi))goto cleanup;
    if(!AssignProcessToJobObject(job,pi.hProcess)){TerminateProcess(pi.hProcess,1);goto cleanup;}
    ResumeThread(pi.hThread);ULONGLONG deadline=GetTickCount64()+15000;
    do {EnumWindows(find_form,0);if(form)break;WaitForSingleObject(pi.hProcess,50);}while(GetTickCount64()<deadline);
    if(!form)goto cleanup;
    DWORD tid=GetWindowThreadProcessId(form,&pid);target=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(!target)goto cleanup;
    WaitForInputIdle(target,5000);
    WCHAR actual[MAX_PATH];DWORD size=MAX_PATH;
    if(!QueryFullProcessImageNameW(target,0,actual,&size))goto cleanup;
    wprintf(L"TARGET pid=%lu creator-tree-root=%lu tid=%lu image=%ls\n",pid,pi.dwProcessId,tid,actual);
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    if(snapshot!=INVALID_HANDLE_VALUE){if(Module32FirstW(snapshot,&entry))do{
        if(wcsstr(entry.szModule,L"ntw16s") || !_wcsicmp(entry.szModule,L"msvbvm60.dll"))wprintf(L"MODULE %ls\n",entry.szExePath);
    }while(Module32NextW(snapshot,&entry));
        CloseHandle(snapshot);}
    module=LoadLibraryW(argv[3]);if(!module)goto cleanup;
    HOOKPROC callback=(void *)GetProcAddress(module,"VbInspect");
    if(!callback || !(hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,module,tid)))goto cleanup;
    DWORD_PTR reply;if(!SendMessageTimeoutW(form,WM_APP+0x371,0,0,SMTO_ABORTIFHUNG,10000,&reply) || WaitForSingleObject(ready,1000)!=WAIT_OBJECT_0)goto cleanup;
    UnhookWindowsHookEx(hook);hook=NULL;
    RedrawWindow(form,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN|RDW_UPDATENOW);
    if(!capture(form,bmp))goto cleanup;
    ShowWindow(form,SW_MINIMIZE);ShowWindow(form,SW_RESTORE);
    SetWindowPos(form,NULL,40,40,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    PostMessageW(form,WM_CLOSE,0,0);
    if(WaitForSingleObject(target,5000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(target,&exit_code) || exit_code)goto cleanup;
    if(WaitForSingleObject(pi.hProcess,5000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(pi.hProcess,&exit_code) || exit_code)goto cleanup;
    deadline=GetTickCount64()+5000;
    do{DWORD message;ULONG_PTR key;LPOVERLAPPED overlapped;
        if(GetQueuedCompletionStatus(completion,&message,&key,&overlapped,500) && message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)break;
    }while(GetTickCount64()<deadline);
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};
    if(!QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL) || accounting.ActiveProcesses)goto cleanup;
    printf("VB_CLEANUP_OK exit=0 active=0 total=%lu\n",accounting.TotalProcesses);result=0;
    FILE *report=_wfopen(log,L"r");BOOL summary=FALSE;WCHAR line[2048];
    const WCHAR *expected=!wcscmp(argv[1],L"bare")?
        L"SUMMARY buttons=4 edits=13 frames=2 attached=0 failures=0":
        L"SUMMARY buttons=4 edits=13 frames=2 attached=19 failures=0";
    if(report){while(fgetws(line,ARRAYSIZE(line),report))if(wcsstr(line,expected))summary=TRUE;fclose(report);}
    if(!summary){fwprintf(stderr,L"VB_SUMMARY_MISMATCH expected=%ls\n",expected);result=1;}

cleanup:
    if(result)fprintf(stderr,"VB6 PROBE FAIL error=%lu pid=%lu\n",GetLastError(),pid);
    if(hook)UnhookWindowsHookEx(hook);
    if(module)FreeLibrary(module);
    if(target)CloseHandle(target);
    if(pi.hThread)CloseHandle(pi.hThread);
    if(pi.hProcess)CloseHandle(pi.hProcess);
    if(job)CloseHandle(job);
    if(completion)CloseHandle(completion);
    if(ready)CloseHandle(ready);
    return result;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
