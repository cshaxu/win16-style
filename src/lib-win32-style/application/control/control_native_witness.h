#ifndef STYLE_CONTROL_NATIVE_WITNESS_H
#define STYLE_CONTROL_NATIVE_WITNESS_H
#include <windows.h>

/* Verify the forwarded native procedure and the application class owner.
 * The returned module is borrowed; no module reference is acquired. */
static inline HMODULE style_control_native_witness(HWND w,WNDPROC proc,const void *address,
                                                   const void *class_name,BOOL unicode)
{
    HMODULE caller=NULL,owner;
    if(unicode){
        WNDCLASSEXW native={.cbSize=sizeof(native)};
        if(!GetClassInfoExW(NULL,(const WCHAR *)class_name,&native) || proc!=native.lpfnWndProc)return NULL;
    }else{
        WNDCLASSEXA native={.cbSize=sizeof(native)};
        if(!GetClassInfoExA(NULL,(const char *)class_name,&native) || proc!=native.lpfnWndProc)return NULL;
    }
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)address,&caller))return NULL;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return owner && caller==owner?owner:NULL;
}
#endif
