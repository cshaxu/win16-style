/* Public opt-in modern dialog entry point. No GUI work runs from DllMain. */
#include "../api.h"
#include "intercept.h"
#include "lib-style/common/modern_file_dialog.h"

static _Thread_local unsigned modern_depth;
BOOL ntw16s_modern_dialog_active(void) { return modern_depth != 0; }
static void modern_enter(void) { ++modern_depth; }
static void modern_leave(void) { --modern_depth; }

HRESULT __cdecl Ntw16sShowModernFileDialog(HWND owner, UINT kind, PWSTR path, DWORD capacity)
{
    const style_dialog_modern_services services={modern_enter,modern_leave,ntw16s_dialog_style_scope};
    return style_dialog_show_modern(owner,kind,path,capacity,&services);
}
