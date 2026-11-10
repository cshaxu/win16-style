#include <windows.h>
#include <stdio.h>
static int failure,reads;
static BOOL WINAPI real_read(HANDLE file,void *buffer,DWORD bytes,DWORD *read,OVERLAPPED *overlap)
{return ReadFile(file,buffer,bytes,read,overlap);}
static BOOL WINAPI test_read(HANDLE file,void *buffer,DWORD bytes,DWORD *read,OVERLAPPED *overlap)
{if(++reads==failure){SetLastError(ERROR_READ_FAULT);return FALSE;}return real_read(file,buffer,bytes,read,overlap);}
static BOOL WINAPI test_seek(HANDLE file,LARGE_INTEGER distance,PLARGE_INTEGER position,DWORD method)
{if(failure==5){SetLastError(ERROR_SEEK);return FALSE;}return SetFilePointerEx(file,distance,position,method);}
#define ReadFile test_read
#define SetFilePointerEx test_seek
#include "../../../src/lib-win32/process-image/image.c"
#undef ReadFile
#undef SetFilePointerEx
int main(void)
{
    WCHAR path[MAX_PATH];if(!GetModuleFileNameW(NULL,path,MAX_PATH))return 1;
    WORD machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    for(failure=1;failure<=4;++failure){reads=0;if(win32_process_image_validate(path,machine,FALSE)!=ERROR_READ_FAULT)return 2;}
    failure=5;reads=0;if(win32_process_image_validate(path,machine,FALSE)!=ERROR_SEEK)return 3;
    puts("IMAGE_PATH_IO_OK read_stages=4 seek=1");return 0;
}
