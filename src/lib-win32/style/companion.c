#include "companion.h"
#include "lib-win32/process-global/module.h"
#include <wchar.h>

BOOL win32_style_companion(const void *anchor,PCWSTR filename,const char *const *names,
    FARPROC *exports,size_t count,size_t pin_index)
{
    HMODULE self=NULL,module=NULL,pinned=NULL;WCHAR path[32768],actual[32768];
    if(!anchor || !filename || !*filename || wcschr(filename,L'\\') || wcschr(filename,L'/') ||
       !names || !exports || !count || pin_index>=count){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    for(size_t i=0;i<count;++i)exports[i]=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)anchor,&self))return FALSE;
    DWORD length=GetModuleFileNameW(self,path,ARRAYSIZE(path));
    if(!length || length>=ARRAYSIZE(path)){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    WCHAR *slash=wcsrchr(path,L'\\');if(!slash){SetLastError(ERROR_INVALID_NAME);return FALSE;}
    size_t remaining=ARRAYSIZE(path)-(size_t)(slash+1-path);
    if(wcscpy_s(slash+1,remaining,filename)){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    module=LoadLibraryW(path);if(!module)return FALSE;
    length=GetModuleFileNameW(module,actual,ARRAYSIZE(actual));
    if(!length || length>=ARRAYSIZE(actual) || !win32_process_global_files_equal(path,actual)){
        FreeLibrary(module);SetLastError(ERROR_INVALID_IMAGE_HASH);return FALSE;
    }
    for(size_t i=0;i<count;++i){
        if(!names[i] || !(exports[i]=GetProcAddress(module,names[i]))){
            for(size_t j=0;j<count;++j)exports[j]=NULL;
            FreeLibrary(module);SetLastError(ERROR_PROC_NOT_FOUND);return FALSE;
        }
    }
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                           (LPCWSTR)(void *)exports[pin_index],&pinned)){
        DWORD error=GetLastError();for(size_t i=0;i<count;++i)exports[i]=NULL;
        FreeLibrary(module);SetLastError(error?error:ERROR_PROC_NOT_FOUND);return FALSE;
    }
    FreeLibrary(module);return TRUE;
}
