#include "hook.h"
#include "lib-win32-style/application/adapters/uwp/native.h"
#include "lib-win32-style/access/runtime/uwp_hook.h"
#ifdef NTWSHK_UNIFIED
#include "lib-win32-style/appearance/msstyles/modern_frame_style.h"
#else
#include "lib-win32-style/appearance/win3x/window/modern_frame.h"
#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#endif
#include <stdint.h>
#include <wchar.h>
static _Thread_local BOOL busy;

#ifndef NTWSHK_UNIFIED
static HRESULT open_frame_profile(HMODULE module,classic_theme_selection selection,classic_theme_mapping *source)
{
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
}
#endif

static BOOL frame_selection(HWND frame,classic_theme_selection *selection)
{
    unsigned stored=style_uwp_frame_profile(frame);
    return stored && classic_theme_decode_selection(stored-1u,selection);
}

static BOOL same_selection(classic_theme_selection a,classic_theme_selection b)
{return a.id==b.id && a.family==b.family;}

static BOOL render_profile(HWND frame,HWND title,const style_uwp_frame_action *actions,unsigned count,
    classic_theme_selection selection,HRESULT loaded,classic_theme_mapping *source)
{
    if(FAILED(loaded)){SetLastError(HRESULT_CODE(loaded));return FALSE;}
    unsigned previous=classic_theme_enter_profile(selection);
    const classic_theme_mapping *theme=loaded==S_OK?source:NULL;
#ifdef NTWSHK_UNIFIED
    BOOL ok=classic_modern_frame_apply(frame,title,actions,count,theme);
#else
    BOOL ok=selection.family==CLASSIC_THEME_WIN2K?win2k_uwp_frame_apply(frame,title,actions,count,theme):
        win16_uwp_frame_apply_actions(frame,title,actions,count,theme);
#endif
    DWORD error=GetLastError();
    if(ok){
        unsigned key=classic_theme_is_folder_id(selection.id)?classic_theme_encode_selection(selection):selection.id;
        style_uwp_frame_set_profile(frame,key+1u);
    }
    classic_theme_close(source);
    classic_theme_leave_profile(previous);SetLastError(error);return ok;
}

static BOOL apply_frame(HWND frame,BOOL fullscreen,BOOL directory,classic_theme_selection selection,
    const classic_theme_selection *current,HMODULE module)
{
    classic_theme_mapping source={0};
#ifdef NTWSHK_UNIFIED
    HRESULT loaded=classic_theme_open_profile(module,selection.id,&source);
#else
    HRESULT loaded=directory?classic_theme_open_profile(module,selection.id,&source):
        open_frame_profile(module,selection,&source);
#endif
    if(FAILED(loaded)){SetLastError(HRESULT_CODE(loaded));return FALSE;}
    if(directory)selection.family=source.family;
    HWND surface=style_uwp_frame_surface(frame);
    if(current && same_selection(*current,selection)){
        classic_theme_close(&source);
        return TRUE;
    }
    HWND auxiliary=NULL,title=style_uwp_title(frame,&auxiliary);
    if(!title){classic_theme_close(&source);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    style_uwp_frame_action actions[2];unsigned count=0;
    if(auxiliary)actions[count++]=(style_uwp_frame_action){L"Application menu...",style_uwp_application_menu,auxiliary};
    if(fullscreen)actions[count++]=(style_uwp_frame_action){L"Toggle full screen",style_uwp_fullscreen,title};
    if(surface && !style_uwp_frame_remove(frame)){
        DWORD error=GetLastError();classic_theme_close(&source);SetLastError(error);return FALSE;
    }
    BOOL ok=render_profile(frame,title,actions,count,selection,loaded,&source);

    if(!ok && current){
        DWORD error=GetLastError();
#ifdef NTWSHK_UNIFIED
        loaded=classic_theme_open_selected(module,*current,&source);
#else
        loaded=open_frame_profile(module,*current,&source);
#endif
        (void)render_profile(frame,title,actions,count,*current,loaded,&source);
        SetLastError(error);
    }
    return ok;
}
static BOOL accept_action(WPARAM encoded,void *context)
{
    (void)context;
    WPARAM action=encoded&NTW16S_UWP_ACTION_MASK,profile=encoded>>NTW16S_UWP_PROFILE_SHIFT;
    if(action&NTW16S_UWP_DIRECTORY){
        action&=~(WPARAM)NTW16S_UWP_DIRECTORY;
        return action>=NTW16S_UWP_APPLY && action<=NTW16S_UWP_APPLY_FULLSCREEN &&
            profile<CLASSIC_THEME_PROFILE_CAP && classic_theme_is_folder_id((unsigned)profile);
    }
#ifdef NTWSHK_UNIFIED
    return FALSE;
#else
    return action>=NTW16S_UWP_APPLY && action<=NTW16S_UWP_APPLY_FULLSCREEN_2K &&
        (!profile || (profile>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM && profile<CLASSIC_THEME_PROFILE_CAP &&
         (classic_theme_is_folder_id((unsigned)profile) ||
          classic_theme_profile_family((unsigned)profile)==(classic_theme_family)(action>=NTW16S_UWP_APPLY_2K))));
#endif
}
static DWORD perform_action(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)core;(void)context;HMODULE module=NULL;SetLastError(0);BOOL ok;
    unsigned profile=(unsigned)(action>>NTW16S_UWP_PROFILE_SHIFT);
    action&=NTW16S_UWP_ACTION_MASK;
    BOOL directory=(action&NTW16S_UWP_DIRECTORY)!=0;
    action&=~(WPARAM)NTW16S_UWP_DIRECTORY;
    classic_theme_selection selection={.id=profile},current;
#ifndef NTWSHK_UNIFIED
    BOOL alternate=action>=NTW16S_UWP_APPLY_2K;
    if(alternate)action-=NTW16S_UWP_APPLY_2K-NTW16S_UWP_APPLY;
    if(!profile)profile=(unsigned)alternate;
    selection=(classic_theme_selection){profile,(classic_theme_family)alternate};
#endif
    BOOL installed=frame_selection(frame,&current);
    BOOL matches=installed && (directory?current.id==profile:same_selection(current,selection));
    if(action==NTW16S_UWP_REMOVE){
        ok=!matches || style_uwp_frame_remove(frame);
    }
    else if(matches && !classic_theme_is_folder_id(profile))ok=TRUE;
    else ok=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (PCWSTR)(uintptr_t)Ntw16sUwpFrameHook,&module)&&apply_frame(frame,
            action==NTW16S_UWP_APPLY_FULLSCREEN,directory,selection,installed?&current:NULL,module);
    DWORD error=ok?ERROR_SUCCESS:GetLastError();return !ok&&!error?ERROR_NOT_SUPPORTED:error;
}
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    const style_uwp_hook_spec spec={NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,accept_action,perform_action,NULL};
    style_uwp_hook_handle(code,message,&busy,&spec);
    return CallNextHookEx(NULL,code,parameter,message);
}
