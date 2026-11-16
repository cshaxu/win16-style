#include <windows.h>
#include <stdio.h>
#include "../../src/run16s-exe/tray_controller.c"

static HANDLE stop_event;
static DWORD observed_style,forced_result;
int run16s_global_with_parent(HANDLE parent,DWORD style)
{
    (void)parent;
    observed_style=style;
    return WaitForSingleObject(stop_event,5000)==WAIT_OBJECT_0?forced_result:10;
}
static BOOL menu_has(controller *state,BOOL a,BOOL b)
{
    HMENU menu=make_menu(state);if(!menu)return FALSE;
    BOOL result=GetMenuItemCount(menu)==3 &&
        !!(GetMenuState(menu,MENU_ENABLE,MF_BYCOMMAND)&MF_CHECKED)==!!a &&
        !!(GetMenuState(menu,MENU_ENABLE_2K,MF_BYCOMMAND)&MF_CHECKED)==!!b &&
        !(GetMenuState(menu,MENU_ENABLE,MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED)) &&
        !(GetMenuState(menu,MENU_ENABLE_2K,MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED)) &&
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
    if(!menu_has(&state,FALSE,FALSE))return 3;
    action(MENU_ENABLE,&state);
    if(!state.control.worker || !state.control.busy || state.control.enabled)return 4;
    if(!menu_has(&state,TRUE,FALSE))return 14;
    action(MENU_ENABLE_2K,&state);
    if(!state.control.stopping || !menu_has(&state,FALSE,TRUE))return 15;
    action(MENU_ENABLE,&state);
    if(!menu_has(&state,TRUE,FALSE) || !wait_stopped(&state,&code) || code)return 16;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || state.control.style!=NTW16S_GLOBAL_STYLE_16)return 17;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,TRUE,FALSE))return 5;
    action(MENU_ENABLE_2K,&state);
    if(!state.control.stopping || state.control.desired!=NTW16S_GLOBAL_STYLE_2K || !wait_stopped(&state,&code) || code)return 6;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.enabled || state.control.style!=NTW16S_GLOBAL_STYLE_2K)return 7;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,FALSE,TRUE))return 8;
    action(MENU_ENABLE,&state);
    if(state.control.desired!=NTW16S_GLOBAL_STYLE_16 || !wait_stopped(&state,&code) || code || observed_style!=NTW16S_GLOBAL_STYLE_2K)return 9;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.style!=NTW16S_GLOBAL_STYLE_16)return 10;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    action(MENU_ENABLE,&state);
    if(state.control.desired!=STYLE_NONE || !wait_stopped(&state,&code) || code)return 11;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(state.control.worker || state.control.enabled || state.control.busy || !menu_has(&state,FALSE,FALSE))return 12;
    forced_result=ERROR_ACCESS_DENIED;action(MENU_ENABLE_2K,&state);action(MENU_ENABLE_2K,&state);
    if(!wait_stopped(&state,&code) || code!=ERROR_ACCESS_DENIED || style_global_control_finished(&state.control,&code))return 19;
    if(state.control.worker || state.control.enabled || state.control.busy || state.control.desired!=STYLE_NONE)return 13;
    CloseHandle(stop_event);
    printf("TRAY CONTROLLER PASS bits=%u switch=both toggle=off failure=1 mutually_exclusive=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
