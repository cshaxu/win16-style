#include "hook.h"
#include "lib-win32-style/application/adapters/uwp/native.h"
#include "lib-win32-style/access/runtime/uwp_hook.h"
#include "lib-win32-style/appearance/win3x/window/modern_frame.h"
#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#include <stdint.h>
#include <wchar.h>
static _Thread_local BOOL busy;

static BOOL read_win2k_theme(HMODULE module,classic_theme *theme)
{
    classic_theme_mapping source={0};
    HRESULT result=classic_theme_open_packaged(module,CLASSIC_THEME_WIN2K,FALSE,&source);
    if(SUCCEEDED(result)){
        *theme=source.model;
        classic_theme_close(&source);
        return TRUE;
    }
#ifdef NTWSHK_UNIFIED
    return FALSE;
#else
    if(result!=HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))return FALSE;
    WCHAR path[32768];
    DWORD length=GetModuleFileNameW(module,path,ARRAYSIZE(path));
    if(!length || length>=ARRAYSIZE(path))return FALSE;
    WCHAR *name=wcsrchr(path,L'\\');
    if(!name)return FALSE;
    ++name;
    if(wcscpy_s(name,ARRAYSIZE(path)-(size_t)(name-path),
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll"))return FALSE;
    /* Read the desktop theme resources without executing the companion DLL. */
    return SUCCEEDED(classic_theme_load(path,L"NormalSize",L"NormalColor",theme));
#endif
}

static const WCHAR profile_property[]=L"Win16Style.UwpProfile";

static BOOL render_profile(HWND frame,HWND title,const style_uwp_frame_action *actions,unsigned count,
    unsigned profile,HMODULE module)
{
    unsigned previous=classic_theme_enter_profile(profile);
    classic_theme model;
    BOOL win2k=(profile&1u)!=0;
    const classic_theme *theme=win2k && read_win2k_theme(module,&model)?&model:NULL;
    BOOL ok=win2k?win2k_uwp_frame_apply(frame,title,actions,count,theme):
        win16_uwp_frame_apply_actions(frame,title,actions,count);
    classic_theme_leave_profile(previous);return ok;
}

static BOOL apply_frame(HWND frame,BOOL fullscreen,unsigned profile,HMODULE module)
{
    HWND auxiliary=NULL,title=style_uwp_title(frame,&auxiliary);
    if(!title){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    style_uwp_frame_action actions[2];unsigned count=0;
    if(auxiliary)actions[count++]=(style_uwp_frame_action){L"Application menu...",style_uwp_application_menu,auxiliary};
    if(fullscreen)actions[count++]=(style_uwp_frame_action){L"Toggle full screen",style_uwp_fullscreen,title};
#ifdef NTWSHK_UNIFIED
    classic_theme_mapping source={0};
    HRESULT loaded=classic_theme_open_profile(module,profile,&source);
    if(FAILED(loaded)){SetLastError(HRESULT_CODE(loaded));return FALSE;}
    classic_theme_close(&source);
#endif
    HWND surface=style_uwp_frame_surface(frame);
    UINT_PTR remembered=(UINT_PTR)GetPropW(frame,profile_property);
    unsigned previous=remembered?(unsigned)(remembered-1u):(unsigned)(surface && win2k_uwp_frame_uses(frame));
    if(surface && !style_uwp_frame_remove(frame))return FALSE;
    BOOL ok=render_profile(frame,title,actions,count,profile,module);
    if(ok)(void)SetPropW(frame,profile_property,(HANDLE)(UINT_PTR)(profile+1u));
    if(!ok && surface){
        DWORD error=GetLastError();
        (void)render_profile(frame,title,actions,count,previous,module);
        SetLastError(error);
    }
    return ok;
}
static BOOL accept_action(WPARAM encoded,void *context)
{
    (void)context;
    WPARAM action=encoded&NTW16S_UWP_ACTION_MASK,profile=encoded>>NTW16S_UWP_PROFILE_SHIFT;
    return action>=NTW16S_UWP_APPLY && action<=NTW16S_UWP_APPLY_FULLSCREEN_2K &&
        (!profile || (profile>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM && profile<CLASSIC_THEME_PROFILE_CAP &&
         (profile&1u)==(action>=NTW16S_UWP_APPLY_2K)));
}
static DWORD perform_action(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)core;(void)context;HMODULE module=NULL;SetLastError(0);BOOL ok;
    unsigned profile=(unsigned)(action>>NTW16S_UWP_PROFILE_SHIFT);
    action&=NTW16S_UWP_ACTION_MASK;
    BOOL win2k=action>=NTW16S_UWP_APPLY_2K;
    if(!profile)profile=(unsigned)win2k;
    UINT_PTR remembered=(UINT_PTR)GetPropW(frame,profile_property);
    BOOL matches=(win2k?win2k_uwp_frame_uses(frame):win16_uwp_frame_uses(frame)) &&
        (remembered?remembered==profile+1u:profile<2);
    if(action==NTW16S_UWP_REMOVE || action==NTW16S_UWP_REMOVE_2K){
        ok=!matches || style_uwp_frame_remove(frame);
        if(matches && ok)RemovePropW(frame,profile_property);
    }
    else if(matches)ok=TRUE;
    else ok=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (PCWSTR)(uintptr_t)Ntw16sUwpFrameHook,&module)&&apply_frame(frame,
            action==NTW16S_UWP_APPLY_FULLSCREEN || action==NTW16S_UWP_APPLY_FULLSCREEN_2K,profile,module);
    DWORD error=ok?ERROR_SUCCESS:GetLastError();return !ok&&!error?ERROR_NOT_SUPPORTED:error;
}
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    const style_uwp_hook_spec spec={NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,accept_action,perform_action,NULL};
    style_uwp_hook_handle(code,message,&busy,&spec);
    return CallNextHookEx(NULL,code,parameter,message);
}
