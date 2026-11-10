#include <windows.h>
#include <stdio.h>

static unsigned scenario;
static BOOL transfer(SIZE_T size,SIZE_T *done)
{
    *done=scenario==1?size-1:size;
    if(scenario==2){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    if(scenario==3){SetLastError(ERROR_SUCCESS);return FALSE;}
    SetLastError(ERROR_INVALID_HANDLE);return TRUE;
}
static BOOL WINAPI simulated_read(HANDLE p,LPCVOID s,LPVOID d,SIZE_T n,SIZE_T *done)
{(void)p;(void)s;(void)d;return transfer(n,done);}
static BOOL WINAPI simulated_write(HANDLE p,LPVOID d,LPCVOID s,SIZE_T n,SIZE_T *done)
{(void)p;(void)s;(void)d;return transfer(n,done);}
#define ReadProcessMemory simulated_read
#define WriteProcessMemory simulated_write
#include "../../../src/lib-win32/process-hook/payload.c"

int main(void)
{
    const DWORD expected[]={ERROR_SUCCESS,ERROR_PARTIAL_COPY,ERROR_ACCESS_DENIED,ERROR_PARTIAL_COPY};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        if(read_memory(NULL,NULL,NULL,4)!=expected[scenario])return (int)scenario+1;
        if(write_memory(NULL,NULL,NULL,4)!=expected[scenario])return (int)scenario+5;
    }
    puts("PAYLOAD_MEMORY_ERRORS_OK read=4 write=4");return 0;
}
