#include <windows.h>
#include <stdio.h>
#include "../../src/run16s-exe/tray_controller.c"

static HANDLE stop_event;
static DWORD observed_style,forced_result;
static DWORD theme_error;
unsigned classic_theme_profile(PCWSTR folder,classic_theme_family family)
{(void)folder;return (unsigned)family;}
HRESULT classic_theme_profile_folder(HMODULE renderer,unsigned profile,WCHAR folder[MAX_PATH])
{(void)renderer;wcscpy(folder,profile?L"Win2K":L"Win16");return S_OK;}
HRESULT classic_theme_open_profile(HMODULE renderer,unsigned profile,classic_theme_mapping *source)
{(void)renderer;(void)profile;(void)source;return HRESULT_FROM_WIN32(theme_error);}
void classic_theme_close(classic_theme_mapping *source)
{(void)source;}
void classic_theme_visit_folders(HMODULE renderer,classic_theme_visit visit,void *context)
{
    (void)renderer;
    if(visit(L"Win16",L"Theme & Name",CLASSIC_THEME_WIN16,context))
        visit(L"Win2K",L"Theme & Name",CLASSIC_THEME_WIN2K,context);
}
int run16s_global_with_parent(HANDLE parent,DWORD style)
{
    (void)parent;
    observed_style=style;
    return WaitForSingleObject(stop_event,5000)==WAIT_OBJECT_0?forced_result:10;
}
static BOOL menu_has(controller *state,BOOL a,BOOL b)
{
    HMENU menu=make_menu(state);if(!menu)return FALSE;
    WCHAR first[64],second[64];
    BOOL result=GetMenuItemCount(menu)==3 &&
        GetMenuStringW(menu,0,first,ARRAYSIZE(first),MF_BYPOSITION) &&
        GetMenuStringW(menu,1,second,ARRAYSIZE(second),MF_BYPOSITION) &&
        !wcscmp(first,L"Theme && Name [Win16]") && !wcscmp(second,L"Theme && Name [Win2K]") &&
        !!(GetMenuState(menu,MENU_FIRST_THEME,MF_BYCOMMAND)&MF_CHECKED)==!!a &&
        !!(GetMenuState(menu,(MENU_FIRST_THEME+1),MF_BYCOMMAND)&MF_CHECKED)==!!b &&
        !(GetMenuState(menu,MENU_FIRST_THEME,MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED)) &&
        !(GetMenuState(menu,(MENU_FIRST_THEME+1),MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED)) &&
        !(GetMenuState(menu,MENU_EXIT,MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED));
    DestroyMenu(menu);return result;
}
static BOOL wait_stopped(controller *state,DWORD *code)
{
    if(!state->control.worker || WaitForSingleObject(state->control.worker,5000)!=WAIT_OBJECT_0 ||
       !GetExitCodeThread(state->control.worker,code))return FALSE;
    ResetEvent(stop_event);return TRUE;
}
int main(void)
{
    controller state={0};DWORD code=0;
    WCHAR prefix[80];swprintf(prefix,ARRAYSIZE(prefix),L"Local\\TrayControllerTest.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=7,.version=1};
    if(!style_global_control_init(&state.control,NULL,NTW16S_GLOBAL_STYLE_16,STYLE_NONE,&space,&operations,NULL))return 2;
    stop_event=CreateEventW(NULL,TRUE,FALSE,state.control.request_name);if(!stop_event)return 2;
    classic_theme_visit_folders(NULL,remember_theme,&state);
    state.themes[0].display_name[0]=0;
    HMENU menu=make_menu(&state);WCHAR label[64];
    BOOL fallback=menu && GetMenuStringW(menu,0,label,ARRAYSIZE(label),MF_BYPOSITION) &&
        !wcscmp(label,L"Win16");
    if(menu)DestroyMenu(menu);
    if(!fallback)return 23;
    wcscpy(state.themes[0].display_name,L"Theme & Name");
    if(!menu_has(&state,FALSE,FALSE))return 3;
    action(MENU_FIRST_THEME,&state);
    if(!state.control.worker || !state.control.busy || state.control.enabled)return 4;
    if(!menu_has(&state,TRUE,FALSE))return 14;
    theme_error=ERROR_INVALID_DATA;
    action((MENU_FIRST_THEME+1),&state);
    if(state.control.stopping || !menu_has(&state,TRUE,FALSE) ||
       WaitForSingleObject(stop_event,0)!=WAIT_TIMEOUT)return 20;
    theme_error=ERROR_SUCCESS;
    action((MENU_FIRST_THEME+1),&state);
    if(!state.control.stopping || !menu_has(&state,FALSE,TRUE))return 15;
    action(MENU_FIRST_THEME,&state);
    if(!menu_has(&state,TRUE,FALSE) || !wait_stopped(&state,&code) || code)return 16;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || state.control.style!=NTW16S_GLOBAL_STYLE_16)return 17;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,TRUE,FALSE))return 5;
    action((MENU_FIRST_THEME+1),&state);
    if(!state.control.stopping || state.control.desired!=NTW16S_GLOBAL_STYLE_2K || !wait_stopped(&state,&code) || code)return 6;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.enabled || state.control.style!=NTW16S_GLOBAL_STYLE_2K)return 7;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,FALSE,TRUE))return 8;
    action(MENU_FIRST_THEME,&state);
    if(state.control.desired!=NTW16S_GLOBAL_STYLE_16 || !wait_stopped(&state,&code) || code || observed_style!=NTW16S_GLOBAL_STYLE_2K)return 9;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.style!=NTW16S_GLOBAL_STYLE_16)return 10;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    theme_error=ERROR_FILE_NOT_FOUND;
    action(MENU_FIRST_THEME,&state);
    theme_error=ERROR_SUCCESS;
    if(state.control.desired!=STYLE_NONE || !wait_stopped(&state,&code) || code)return 11;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(state.control.worker || state.control.enabled || state.control.busy || !menu_has(&state,FALSE,FALSE))return 12;
    forced_result=ERROR_ACCESS_DENIED;action((MENU_FIRST_THEME+1),&state);action((MENU_FIRST_THEME+1),&state);
    if(!wait_stopped(&state,&code) || code!=ERROR_ACCESS_DENIED || style_global_control_finished(&state.control,&code))return 19;
    if(state.control.worker || state.control.enabled || state.control.busy || state.control.desired!=STYLE_NONE)return 13;
    CloseHandle(stop_event);free(state.themes);
    printf("TRAY CONTROLLER PASS bits=%u switch=both toggle=off failure=1 mutually_exclusive=1 metadata=1 fallback=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
