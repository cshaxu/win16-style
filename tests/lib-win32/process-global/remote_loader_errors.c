#include <windows.h>
#include <stdio.h>
static int mode;
static DWORD WINAPI fake_module(HMODULE module,LPWSTR path,DWORD count)
{
    if(mode==1 || mode==5){SetLastError(mode==1?ERROR_ACCESS_DENIED:ERROR_SUCCESS);return 0;}
    if(mode==2)return count;
    return GetModuleFileNameW(module,path,count);
}
static UINT WINAPI fake_system(LPWSTR path,UINT count)
{
    if(mode==3 || mode==6){SetLastError(mode==3?ERROR_ACCESS_DENIED:ERROR_SUCCESS);return 0;}
    if(mode==4)return count;
    return GetSystemDirectoryW(path,count);
}
#define GetModuleFileNameW fake_module
#define GetSystemDirectoryW fake_system
#include "lib-win32/process-global/remote_loader.c"
int main(void)
{
    const DWORD expected[]={ERROR_ACCESS_DENIED,ERROR_INSUFFICIENT_BUFFER,ERROR_ACCESS_DENIED,
                            ERROR_INSUFFICIENT_BUFFER,ERROR_INVALID_DATA,ERROR_INVALID_DATA};
    for(mode=1;mode<=6;++mode){
        void *address=(void *)1;
        DWORD error=win32_process_global_remote_loadlibrary(GetCurrentProcess(),GetCurrentProcessId(),&address);
        if(error!=expected[mode-1] || address){
            fprintf(stderr,"LOADER_ERROR_FAIL mode=%d expected=%lu actual=%lu address=%p\n",mode,expected[mode-1],error,address);
            return mode;
        }
    }
    puts("LOADER_ERROR_OK module=3 system=3 address=cleared");return 0;
}
