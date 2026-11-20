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

static BOOL apply_frame(HWND frame,BOOL fullscreen,BOOL win2k,HMODULE module)
{
    HWND auxiliary=NULL,title=style_uwp_title(frame,&auxiliary);
    if(!title){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    style_uwp_frame_action actions[2];unsigned count=0;
    if(auxiliary)actions[count++]=(style_uwp_frame_action){L"Application menu...",style_uwp_application_menu,auxiliary};
    if(fullscreen)actions[count++]=(style_uwp_frame_action){L"Toggle full screen",style_uwp_fullscreen,title};
#ifdef NTWSHK_UNIFIED
    DWORD theme_error=classic_theme_validate_packaged(module,win2k?CLASSIC_THEME_WIN2K:CLASSIC_THEME_WIN16);
    if(theme_error){SetLastError(theme_error);return FALSE;}
#endif
    HWND surface=style_uwp_frame_surface(frame);
    BOOL previous2k=surface && win2k_uwp_frame_uses(frame);
    classic_theme model;
    const classic_theme *theme=(win2k || previous2k) && read_win2k_theme(module,&model)?&model:NULL;
    if(surface && !style_uwp_frame_remove(frame))return FALSE;
    BOOL ok=win2k?win2k_uwp_frame_apply(frame,title,actions,count,theme):
        win16_uwp_frame_apply_actions(frame,title,actions,count);
    if(!ok && surface){
        DWORD error=GetLastError();
        if(previous2k)(void)win2k_uwp_frame_apply(frame,title,actions,count,theme);
        else (void)win16_uwp_frame_apply_actions(frame,title,actions,count);
        SetLastError(error);
    }
    return ok;
}
static BOOL accept_action(WPARAM action,void *context)
{(void)context;return action>=NTW16S_UWP_APPLY && action<=NTW16S_UWP_APPLY_FULLSCREEN_2K;}
static DWORD perform_action(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)core;(void)context;HMODULE module=NULL;SetLastError(0);BOOL ok;
    BOOL win2k=action>=NTW16S_UWP_APPLY_2K;
    BOOL matches=win2k?win2k_uwp_frame_uses(frame):win16_uwp_frame_uses(frame);
    if(action==NTW16S_UWP_REMOVE || action==NTW16S_UWP_REMOVE_2K)
        ok=!matches || style_uwp_frame_remove(frame);
    else if(matches)ok=TRUE;
    else ok=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (PCWSTR)(uintptr_t)Ntw16sUwpFrameHook,&module)&&apply_frame(frame,
            action==NTW16S_UWP_APPLY_FULLSCREEN || action==NTW16S_UWP_APPLY_FULLSCREEN_2K,win2k,module);
    DWORD error=ok?ERROR_SUCCESS:GetLastError();return !ok&&!error?ERROR_NOT_SUPPORTED:error;
}
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    const style_uwp_hook_spec spec={NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,accept_action,perform_action,NULL};
    style_uwp_hook_handle(code,message,&busy,&spec);
    return CallNextHookEx(NULL,code,parameter,message);
}
