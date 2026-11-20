#include "hook.h"
#include "lib-win32-style/application/adapters/uwp/native.h"
#include "lib-win32-style/access/runtime/uwp_hook.h"
#include "lib-win32-style/appearance/win3x/window/modern_frame.h"
#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#include <stdint.h>
#include <wchar.h>
static _Thread_local BOOL busy;

static HRESULT open_frame_profile(HMODULE module,classic_theme_selection selection,classic_theme_mapping *source)
{
#ifdef NTWSHK_UNIFIED
    return classic_theme_open_selected(module,selection,source);
#else
    BOOL win2k=selection.family==CLASSIC_THEME_WIN2K;
    unsigned previous=classic_theme_enter_profile(selection);
    HRESULT result=classic_theme_open_packaged(module,selection.family,!win2k,source);
    classic_theme_leave_profile(previous);
    if(SUCCEEDED(result))return S_OK;
    if(!win2k || result!=HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))return S_FALSE;
    WCHAR path[32768];
    DWORD length=GetModuleFileNameW(module,path,ARRAYSIZE(path));
    if(!length || length>=ARRAYSIZE(path))return S_FALSE;
    WCHAR *name=wcsrchr(path,L'\\');
    if(!name)return S_FALSE;
    ++name;
    if(wcscpy_s(name,ARRAYSIZE(path)-(size_t)(name-path),
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll"))return S_FALSE;
    /* Read the desktop theme resources without executing the companion DLL. */
    return SUCCEEDED(classic_theme_load(path,L"NormalSize",L"NormalColor",&source->model))?S_OK:S_FALSE;
#endif
}

static const WCHAR profile_property[]=L"Win16Style.UwpProfile";

static BOOL render_profile(HWND frame,HWND title,const style_uwp_frame_action *actions,unsigned count,
    classic_theme_selection selection,HRESULT loaded,classic_theme_mapping *source)
{
    if(FAILED(loaded)){SetLastError(HRESULT_CODE(loaded));return FALSE;}
    unsigned previous=classic_theme_enter_profile(selection);
    const classic_theme_mapping *theme=loaded==S_OK?source:NULL;
    BOOL ok=selection.family==CLASSIC_THEME_WIN2K?win2k_uwp_frame_apply(frame,title,actions,count,theme):
        win16_uwp_frame_apply_actions(frame,title,actions,count,theme);
    DWORD error=GetLastError();
    classic_theme_close(source);
    classic_theme_leave_profile(previous);SetLastError(error);return ok;
}

static BOOL apply_frame(HWND frame,BOOL fullscreen,classic_theme_selection selection,HMODULE module)
{
    HWND auxiliary=NULL,title=style_uwp_title(frame,&auxiliary);
    if(!title){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    style_uwp_frame_action actions[2];unsigned count=0;
    if(auxiliary)actions[count++]=(style_uwp_frame_action){L"Application menu...",style_uwp_application_menu,auxiliary};
    if(fullscreen)actions[count++]=(style_uwp_frame_action){L"Toggle full screen",style_uwp_fullscreen,title};
    classic_theme_mapping source={0};
    HRESULT loaded=open_frame_profile(module,selection,&source);
    if(FAILED(loaded)){SetLastError(HRESULT_CODE(loaded));return FALSE;}
    HWND surface=style_uwp_frame_surface(frame);
    UINT_PTR remembered=(UINT_PTR)GetPropW(frame,profile_property);
    classic_theme_family old_family=surface && win2k_uwp_frame_uses(frame)?CLASSIC_THEME_WIN2K:CLASSIC_THEME_WIN16;
    classic_theme_selection previous={remembered?(unsigned)(remembered-1u):(unsigned)old_family,old_family};
    if(surface && !style_uwp_frame_remove(frame)){
        DWORD error=GetLastError();classic_theme_close(&source);SetLastError(error);return FALSE;
    }
    BOOL ok=render_profile(frame,title,actions,count,selection,loaded,&source);
    if(ok)(void)SetPropW(frame,profile_property,(HANDLE)(UINT_PTR)(selection.id+1u));
    if(!ok && surface){
        DWORD error=GetLastError();
        loaded=open_frame_profile(module,previous,&source);
        (void)render_profile(frame,title,actions,count,previous,loaded,&source);
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
         (classic_theme_is_folder_id((unsigned)profile) ||
          classic_theme_profile_family((unsigned)profile)==(classic_theme_family)(action>=NTW16S_UWP_APPLY_2K))));
}
static DWORD perform_action(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)core;(void)context;HMODULE module=NULL;SetLastError(0);BOOL ok;
    unsigned profile=(unsigned)(action>>NTW16S_UWP_PROFILE_SHIFT);
    action&=NTW16S_UWP_ACTION_MASK;
    BOOL win2k=action>=NTW16S_UWP_APPLY_2K;
    classic_theme_family family=win2k?CLASSIC_THEME_WIN2K:CLASSIC_THEME_WIN16;
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
            action==NTW16S_UWP_APPLY_FULLSCREEN || action==NTW16S_UWP_APPLY_FULLSCREEN_2K,
            (classic_theme_selection){profile,family},module);
    DWORD error=ok?ERROR_SUCCESS:GetLastError();return !ok&&!error?ERROR_NOT_SUPPORTED:error;
}
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    const style_uwp_hook_spec spec={NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,accept_action,perform_action,NULL};
    style_uwp_hook_handle(code,message,&busy,&spec);
    return CallNextHookEx(NULL,code,parameter,message);
}
