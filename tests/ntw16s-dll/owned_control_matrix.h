#ifndef NTW16S_TEST_OWNED_CONTROL_MATRIX_H
#define NTW16S_TEST_OWNED_CONTROL_MATRIX_H

static int owned_control_matrix(int argc,WCHAR **argv,DWORD timeout,const char *label)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned mode=0;mode<2;++mode){
        if(mode)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"bare-aware":L"bare");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};
        limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
        si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        if(!AssignProcessToJobObject(job,pi.hProcess)){TerminateProcess(pi.hProcess,1);CHECK(FALSE);}
        ResumeThread(pi.hThread);DWORD code=1,wait=WaitForSingleObject(pi.hProcess,timeout);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        if(wait!=WAIT_OBJECT_0)TerminateJobObject(job,ERROR_TIMEOUT);
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};BOOL empty=FALSE;ULONGLONG deadline=GetTickCount64()+5000;
        do{CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL));
            if(!accounting.ActiveProcesses){empty=TRUE;break;}WaitForSingleObject(pi.hProcess,10);
        }while(GetTickCount64()<deadline);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code && empty);
    }
    printf("%s owned_jobs_empty=4\n",label);return 0;
}

#endif
