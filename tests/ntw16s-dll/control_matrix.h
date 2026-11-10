#ifndef NTW16S_TEST_CONTROL_MATRIX_H
#define NTW16S_TEST_CONTROL_MATRIX_H

/* The caller supplies CHECK and retains the control-specific child function. */
static int control_matrix(const WCHAR *launcher,const char *label,BOOL detailed)
{
    WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",launcher,self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD started=GetTickCount(),code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        if(label){
            if(detailed)printf("%s_WORKER aware=%u styled=%u child_pid=%lu creator_pid=%lu elapsed_ms=%lu wait=%lu exit=%lu\n",label,aware,styled,pi.dwProcessId,GetCurrentProcessId(),GetTickCount()-started,wait,code);
            else printf("%s_WORKER aware=%u styled=%u elapsed_ms=%lu wait=%lu exit=%lu\n",label,aware,styled,GetTickCount()-started,wait,code);
        }
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    return 0;
}
#endif
