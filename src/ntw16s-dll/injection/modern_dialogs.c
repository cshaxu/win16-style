/* Public opt-in modern dialog entry point. No GUI work runs from DllMain. */
#include "../api.h"
#include "intercept.h"
#include "lib-style/common/modern_file_dialog.h"

HRESULT __cdecl Ntw16sShowModernFileDialog(HWND owner, UINT kind, PWSTR path, DWORD capacity)
{
    return style_dialog_show_modern(owner,kind,path,capacity,ntw16s_dialog_style_scope);
}
