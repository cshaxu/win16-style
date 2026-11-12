#include "target.h"
#include "hook.h"
#include "lib-style/uwp/native.h"
#include "lib-style/uwp/dispatch.h"
static DWORD select_action(const style_uwp_target *app,BOOL apply,WPARAM *action,void *context)
{
    (void)context;*action=NTW16S_UWP_REMOVE;
    if(apply){
        HWND auxiliary=NULL,title=style_uwp_title(app->frame,&auxiliary);
        if(!title)return ERROR_NOT_SUPPORTED;
        HRESULT capability=style_uwp_fullscreen_available(title);
        if(FAILED(capability))return (DWORD)capability;
        *action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    }
    return ERROR_SUCCESS;
}
DWORD ntw16s_uwp_target_change(const style_uwp_target *app,PCWSTR path,BOOL apply)
{
    const style_uwp_dispatch_spec spec={IMAGE_FILE_MACHINE_AMD64,"Ntw16sUwpFrameHook",
        NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,3000,select_action,NULL};
    return style_uwp_dispatch_change(app,path,apply,&spec);
}
