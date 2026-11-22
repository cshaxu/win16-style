#include "target.h"
#include "hook.h"
#include "lib-win32-style/application/adapters/uwp/native.h"
#include "lib-win32-style/access/runtime/uwp_dispatch.h"
static DWORD select_action(const style_uwp_target *app,BOOL apply,WPARAM *action,void *context)
{
    const classic_theme_selection *selection=context;
    WPARAM operation=NTW16S_UWP_REMOVE;
    if(apply){
        HWND auxiliary=NULL,title=style_uwp_title(app->frame,&auxiliary);
        if(!title)return ERROR_NOT_SUPPORTED;
        HRESULT capability=style_uwp_fullscreen_available(title);
        if(FAILED(capability))return (DWORD)capability;
        operation=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    }
    if(classic_theme_is_folder_id(selection->id)){
        operation|=NTW16S_UWP_DIRECTORY|((WPARAM)selection->id<<NTW16S_UWP_PROFILE_SHIFT);
    }else{
#ifdef STYLE_THEME_EXTERNAL_ONLY
        return ERROR_INVALID_PARAMETER;
#else
        if(selection->family==CLASSIC_THEME_WIN2K)operation+=NTW16S_UWP_APPLY_2K-NTW16S_UWP_APPLY;
        if(selection->id>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM)
            operation|=(WPARAM)selection->id<<NTW16S_UWP_PROFILE_SHIFT;
#endif
    }
    *action=operation;
    return ERROR_SUCCESS;
}
DWORD ntw16s_uwp_target_change(const style_uwp_target *app,PCWSTR path,BOOL apply,classic_theme_selection selection)
{
    const style_uwp_dispatch_spec spec={IMAGE_FILE_MACHINE_AMD64,"Ntw16sUwpFrameHook",
        NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,3000,select_action,&selection};
    return style_uwp_dispatch_change(app,path,apply,&spec);
}
