#ifndef WIN32_PROCESS_GLOBAL_REMOTE_MEMORY_H
#define WIN32_PROCESS_GLOBAL_REMOTE_MEMORY_H
#include <windows.h>

static inline DWORD win32_process_global_remote_read(HANDLE process,const void *source,void *destination,SIZE_T bytes)
{
    SIZE_T read=0;
    if(!ReadProcessMemory(process,source,destination,bytes,&read)){
        DWORD error=GetLastError();return error?error:ERROR_PARTIAL_COPY;
    }
    return read==bytes?ERROR_SUCCESS:ERROR_PARTIAL_COPY;
}

static inline DWORD win32_process_global_remote_write(HANDLE process,void *destination,const void *source,SIZE_T bytes)
{
    SIZE_T written=0;
    if(!WriteProcessMemory(process,destination,source,bytes,&written)){
        DWORD error=GetLastError();return error?error:ERROR_PARTIAL_COPY;
    }
    return written==bytes?ERROR_SUCCESS:ERROR_PARTIAL_COPY;
}

/* A running thread may still use the allocation; a dead process owns none. */
static inline DWORD win32_process_global_remote_release(HANDLE thread,HANDLE process,void *memory)
{
    if(thread){
        DWORD wait=WaitForSingleObject(thread,0);
        if(wait==WAIT_TIMEOUT)return ERROR_BUSY;
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    if(memory){
        if(!process)return ERROR_INVALID_STATE;
        if(!VirtualFreeEx(process,memory,0,MEM_RELEASE)){
            DWORD error=GetLastError();
            if(WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return error;
        }
    }
    return ERROR_SUCCESS;
}
#endif
