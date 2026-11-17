#include "ntw16s-dll/global/vb_text.h"
#include "lib-win32-style/application/adapters/win32/runtime_text_imports.h"
#include "lib-win32-style/appearance/win3x/control/vb_text.h"

DWORD ntw16s_global_vb_text(HWND window,BOOL *changed)
{
    HMODULE runtime=NULL;DWORD process=0,error=ERROR_NOT_SUPPORTED;WCHAR name[64];
    if(changed)*changed=FALSE;
    if(!window || !changed)return ERROR_INVALID_PARAMETER;
    if(GetWindowThreadProcessId(window,&process)!=GetCurrentThreadId() || process!=GetCurrentProcessId() ||
       !GetClassNameW(window,name,ARRAYSIZE(name)) || lstrcmpW(name,L"ThunderRT6FormDC"))return ERROR_NOT_SUPPORTED;
    if(!GetModuleHandleExW(0,L"MSVBVM60.DLL",&runtime))return ERROR_NOT_SUPPORTED;
    if((HMODULE)GetClassLongPtrW(window,GCLP_HMODULE)!=runtime)goto done;
    error=style_runtime_text_imports(runtime,win16_control_vb_paint_font,changed);

done:
    FreeLibrary(runtime);return error;
}
