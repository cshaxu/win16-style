#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
int wmain(int argc, WCHAR **argv)
{
    if(argc!=4 && argc!=5){fwprintf(stderr,L"usage: driver <run16s> <probe> <report> [timeout-ms]\n");return 2;}
    WCHAR *end=NULL;unsigned long timeout=argc==5?wcstoul(argv[4],&end,10):120000;
    if(!timeout || timeout>120000 || (end && *end))return 2;
    if(GetFileAttributesW(argv[3])!=INVALID_FILE_ATTRIBUTES)return 8;
    WCHAR command[32768];
    if(swprintf(command,32768,L"\"%ls\" --wait \"%ls\" --nohandle \"%ls\"",argv[1],argv[2],argv[3])<0)return 3;
    HANDLE job=CreateJobObjectW(NULL,NULL);if(!job)return 4;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){CloseHandle(job);return 5;}
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION child={0};
    if(!CreateProcessW(argv[1],command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&child)){CloseHandle(job);return 6;}
    if(!AssignProcessToJobObject(job,child.hProcess)){TerminateProcess(child.hProcess,1);CloseHandle(child.hThread);CloseHandle(child.hProcess);CloseHandle(job);return 7;}
    ResumeThread(child.hThread);CloseHandle(child.hThread);
    ULONGLONG deadline=GetTickCount64()+timeout;
    DWORD wait=WaitForSingleObject(child.hProcess,timeout),code=STILL_ACTIVE;
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
    wprintf(L"wait=%lu launcher_exit=%lu\n",wait,code);
    BOOL valid=FALSE;
    if(wait==WAIT_OBJECT_0 && !code){
        while(GetTickCount64()<deadline){
            FILE *file=_wfopen(argv[3],L"r, ccs=UTF-8");
            if(file){
                WCHAR line[32768]={0};
                BOOL complete=fgetws(line,32768,file) && wcschr(line,L'\n');
                if(complete){
                    valid=wcsstr(line,L"elevated=1 ") && wcsstr(line,L" attached=1") &&
                        wcsstr(line,L"dll=") && !wcsstr(line,L"dll= attached=");
                }
                fclose(file);if(complete)break;
            }
            Sleep(50);
        }
    }
    CloseHandle(child.hProcess);CloseHandle(job);
    if(wait!=WAIT_OBJECT_0)return ERROR_TIMEOUT;
    if(code)return (int)code;
    return valid?0:9;
}
