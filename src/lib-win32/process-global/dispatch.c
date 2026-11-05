#include "dispatch.h"
DWORD WINAPI win32_process_global_dispatch_run(void *parameter)
{
    win32_process_global_dispatch *dispatch=(win32_process_global_dispatch *)parameter;
    if(!SetThreadDesktop(dispatch->desktop)){dispatch->error=GetLastError();SetEvent(dispatch->ready);return 1;}
    SetEvent(dispatch->ready);HANDLE waits[]={dispatch->quit,dispatch->stop,dispatch->work};
    for(;;){
        DWORD result=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,INFINITE);
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)break;
        if(result!=WAIT_OBJECT_0+2)break;
        while(dispatch->next(dispatch->context)){
            if(WaitForSingleObject(dispatch->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(dispatch->stop,0)==WAIT_OBJECT_0)break;
        }
    }
    return 0;
}
