#include <windows.h>
#include <stdio.h>

static unsigned scenario;
static BOOL transfer(SIZE_T size,SIZE_T *written)
{
    *written=scenario==1?size-1:size;
    if(scenario==2){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    if(scenario==3){SetLastError(ERROR_SUCCESS);return FALSE;}
    SetLastError(ERROR_INVALID_HANDLE);
    return TRUE;
}
static BOOL WINAPI simulated_write(HANDLE process,LPVOID destination,LPCVOID source,SIZE_T size,SIZE_T *written)
{(void)process;(void)destination;(void)source;return transfer(size,written);}
static BOOL WINAPI simulated_read(HANDLE process,LPCVOID source,LPVOID destination,SIZE_T size,SIZE_T *read)
{(void)process;(void)destination;(void)source;return transfer(size,read);}
#define WriteProcessMemory simulated_write
#define ReadProcessMemory simulated_read
#include "../../../src/lib-win32/process-global/remote_memory.h"

int main(void)
{
    const DWORD expected[]={ERROR_SUCCESS,ERROR_PARTIAL_COPY,ERROR_ACCESS_DENIED,ERROR_PARTIAL_COPY};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        if(win32_process_global_remote_write(NULL,NULL,NULL,4)!=expected[scenario])return (int)scenario+1;
        if(win32_process_global_remote_read(NULL,NULL,NULL,4)!=expected[scenario])return (int)scenario+5;
    }
    puts("REMOTE_MEMORY_ERRORS_OK read=4 write=4");return 0;
}
