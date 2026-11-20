#include "target.h"
#include "hook.h"
#include "lib-win32-style/application/adapters/uwp/native.h"
#include "lib-win32-style/access/runtime/uwp_dispatch.h"
static DWORD select_action(const style_uwp_target *app,BOOL apply,WPARAM *action,void *context)
{
    unsigned profile=*(const unsigned *)context;
    BOOL win2k=(profile&1u)!=0;
    *action=win2k?NTW16S_UWP_REMOVE_2K:NTW16S_UWP_REMOVE;
    if(apply){
        HWND auxiliary=NULL,title=style_uwp_title(app->frame,&auxiliary);
        if(!title)return ERROR_NOT_SUPPORTED;
        HRESULT capability=style_uwp_fullscreen_available(title);
        if(FAILED(capability))return (DWORD)capability;
        if(win2k)*action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN_2K:NTW16S_UWP_APPLY_2K;
        else *action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    }
    if(profile>=2)*action|=(WPARAM)profile<<NTW16S_UWP_PROFILE_SHIFT;
    return ERROR_SUCCESS;
}
DWORD ntw16s_uwp_target_change(const style_uwp_target *app,PCWSTR path,BOOL apply,unsigned profile)
{
    const style_uwp_dispatch_spec spec={IMAGE_FILE_MACHINE_AMD64,"Ntw16sUwpFrameHook",
        NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,3000,select_action,&profile};
    return style_uwp_dispatch_change(app,path,apply,&spec);
}
