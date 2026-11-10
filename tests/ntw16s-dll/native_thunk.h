#ifndef NTW16S_TEST_NATIVE_THUNK_H
#define NTW16S_TEST_NATIVE_THUNK_H
#include <windows.h>
#include <string.h>

static void *test_create_native_thunk(void *target)
{
    DWORD old;
    void *thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!thunk)return NULL;
#ifdef _WIN64
    unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};
    memcpy(code+2,&target,sizeof(target));
#else
    unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};
    memcpy(code+1,&target,sizeof(target));
#endif
    memcpy(thunk,code,sizeof(code));
    if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){
        DWORD error=GetLastError();VirtualFree(thunk,0,MEM_RELEASE);SetLastError(error);return NULL;
    }
    FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));
    return thunk;
}
#endif
