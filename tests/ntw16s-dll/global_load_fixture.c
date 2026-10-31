#include <windows.h>

/* Test-only bounded loader delay: proves the caller retains its path while a
 * target loader thread is still using it. Never included in product DLLs. */
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,void *reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_ATTACH){
        WCHAR name[128];HANDLE entered=NULL,gate=NULL;
        if(GetEnvironmentVariableW(L"WIN16_LOAD_TEST_ENTERED",name,ARRAYSIZE(name)))entered=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
        if(GetEnvironmentVariableW(L"WIN16_LOAD_TEST_GATE",name,ARRAYSIZE(name)))gate=OpenEventW(SYNCHRONIZE,FALSE,name);
        if(entered)SetEvent(entered);
        if(gate)WaitForSingleObject(gate,3000);
        if(gate)CloseHandle(gate);
        if(entered)CloseHandle(entered);
    }
    return TRUE;
}
__declspec(dllexport) DWORD WINAPI Ntw16sGlobalThread(void *packet)
{(void)packet;return ERROR_INVALID_PARAMETER;}
