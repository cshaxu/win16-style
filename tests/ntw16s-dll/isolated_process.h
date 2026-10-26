#ifndef NTW16S_TEST_ISOLATED_PROCESS_H
#define NTW16S_TEST_ISOLATED_PROCESS_H
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#ifndef NTW16S_TEST_PROCESS_TIMEOUT
#define NTW16S_TEST_PROCESS_TIMEOUT 90000
#endif

/* USER/IME can keep hidden thread windows until process exit. The parent
 * owns the desktop and joins its worker before closing it; no busy desktop
 * reassignment or manual destruction of system-owned helper windows. */
static int isolated_process(int argc,WCHAR **argv,int (*worker)(int,WCHAR **))
{
    WCHAR name[80],image[MAX_PATH],command[MAX_PATH*3];HDESK desktop;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
    DWORD code=1,wait;int i;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    if(argc>1 && !wcscmp(argv[1],L"--private-worker"))return worker(argc-1,argv+1);
    swprintf_s(name,80,L"Win16CoreWorker-%lu",GetCurrentProcessId());
    desktop=CreateDesktopW(name,NULL,NULL,0,DESKTOP_CREATEWINDOW|DESKTOP_ENUMERATE|
        DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_SWITCHDESKTOP,NULL);
    if(!desktop){fprintf(stderr,"PRIVATE_DESKTOP_CREATE_FAIL error=%lu\n",GetLastError());return 2;}
    if(!GetModuleFileNameW(NULL,image,MAX_PATH)){fprintf(stderr,"PRIVATE_DESKTOP_IMAGE_FAIL error=%lu\n",GetLastError());CloseDesktop(desktop);return 2;}
    if(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --private-worker",image)<0)return 3;
    for(i=1;i<argc;++i){
        if(wcschr(argv[i],L'"') || wcscat_s(command,ARRAYSIZE(command),L" \"") ||
            wcscat_s(command,ARRAYSIZE(command),argv[i]) || wcscat_s(command,ARRAYSIZE(command),L"\""))return 4;
    }
    startup.lpDesktop=name;startup.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;startup.wShowWindow=SW_HIDE;
    startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    if(!CreateProcessW(image,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process)){
        CloseDesktop(desktop);return 5;
    }
    wait=WaitForSingleObject(process.hProcess,NTW16S_TEST_PROCESS_TIMEOUT);
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
    else {TerminateProcess(process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(process.hProcess,1000);code=6;}
    CloseHandle(process.hThread);CloseHandle(process.hProcess);
    if(!CloseDesktop(desktop))return 7;
    return (int)code;
}
#endif
