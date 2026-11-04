#include "process.h"
DWORD win32_process_create_suspended(PCWSTR application,PWSTR command,
    LPSECURITY_ATTRIBUTES process_attributes,LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit,DWORD flags,LPVOID environment,PCWSTR directory,
    LPSTARTUPINFOW startup,LPPROCESS_INFORMATION child)
{
    if(!CreateProcessW(application,command,process_attributes,thread_attributes,inherit,
        flags|CREATE_SUSPENDED,environment,directory,startup,child)){
        DWORD error=GetLastError();return error?error:ERROR_GEN_FAILURE;
    }
    return ERROR_SUCCESS;
}
DWORD win32_process_activate(PROCESS_INFORMATION *child,win32_process_prepare prepare,void *context,unsigned *phase)
{
    DWORD error,previous;
    if(!child || !child->hProcess || !child->hThread || !prepare || !phase)return ERROR_INVALID_PARAMETER;
    *phase=WIN32_PROCESS_PREPARE_FAILED;
    error=prepare(child->hProcess,context);
    if(!error){
        *phase=WIN32_PROCESS_RESUME_FAILED;previous=ResumeThread(child->hThread);
        if(previous!=1){error=previous==(DWORD)-1?GetLastError():ERROR_INVALID_STATE;if(!error)error=ERROR_GEN_FAILURE;}
    }
    if(error){
        TerminateProcess(child->hProcess,error);WaitForSingleObject(child->hProcess,5000);
        CloseHandle(child->hThread);CloseHandle(child->hProcess);
        child->hThread=NULL;child->hProcess=NULL;return error;
    }
    CloseHandle(child->hThread);child->hThread=NULL;return ERROR_SUCCESS;
}
DWORD win32_process_wait(HANDLE process,DWORD *exit_code)
{
    if(!process || !exit_code)return ERROR_INVALID_PARAMETER;
    if(WaitForSingleObject(process,INFINITE)!=WAIT_OBJECT_0 || !GetExitCodeProcess(process,exit_code)){
        DWORD error=GetLastError();return error?error:ERROR_GEN_FAILURE;
    }
    return ERROR_SUCCESS;
}
