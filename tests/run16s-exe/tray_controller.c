#include <windows.h>
#include <stdio.h>
#include "../../src/run16s-exe/tray_controller.c"

static HANDLE stop_event;
static DWORD observed_style;
int run16s_global_with_parent(HANDLE parent,DWORD style)
{
    (void)parent;
    observed_style=style;
    return WaitForSingleObject(stop_event,5000)==WAIT_OBJECT_0?0:10;
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
    if(!state->worker || WaitForSingleObject(state->worker,5000)!=WAIT_OBJECT_0 ||
       !GetExitCodeThread(state->worker,code))return FALSE;
    CloseHandle(state->worker);state->worker=NULL;ResetEvent(stop_event);return TRUE;
}
int main(void)
{
    controller state={0};DWORD code=0;
    swprintf(state.request_name,ARRAYSIZE(state.request_name),L"Local\\TrayControllerTest.%lu",GetCurrentProcessId());
    stop_event=CreateEventW(NULL,TRUE,FALSE,state.request_name);if(!stop_event)return 2;
    state.next_style=STYLE_NONE;
    if(!menu_has(&state,FALSE,FALSE))return 3;
    action(MENU_ENABLE,&state);
    if(!state.worker || !state.busy || state.enabled)return 4;
    if(!menu_has(&state,TRUE,FALSE))return 14;
    action(MENU_ENABLE_2K,&state);
    if(!state.stopping || !menu_has(&state,FALSE,TRUE))return 15;
    action(MENU_ENABLE,&state);
    if(!menu_has(&state,TRUE,FALSE) || !wait_stopped(&state,&code) || code)return 16;
    worker_stopped(&state,code);
    if(!state.worker || state.style!=NTW16S_GLOBAL_STYLE_16)return 17;
    state.enabled=TRUE;state.busy=FALSE;
    if(!menu_has(&state,TRUE,FALSE))return 5;
    action(MENU_ENABLE_2K,&state);
    if(!state.stopping || state.next_style!=NTW16S_GLOBAL_STYLE_2K || !wait_stopped(&state,&code) || code)return 6;
    worker_stopped(&state,code);
    if(!state.worker || !state.busy || state.enabled || state.style!=NTW16S_GLOBAL_STYLE_2K)return 7;
    state.enabled=TRUE;state.busy=FALSE;
    if(!menu_has(&state,FALSE,TRUE))return 8;
    action(MENU_ENABLE,&state);
    if(state.next_style!=NTW16S_GLOBAL_STYLE_16 || !wait_stopped(&state,&code) || code || observed_style!=NTW16S_GLOBAL_STYLE_2K)return 9;
    worker_stopped(&state,code);
    if(!state.worker || !state.busy || state.style!=NTW16S_GLOBAL_STYLE_16)return 10;
    state.enabled=TRUE;state.busy=FALSE;
    action(MENU_ENABLE,&state);
    if(state.next_style!=STYLE_NONE || !wait_stopped(&state,&code) || code)return 11;
    worker_stopped(&state,code);
    if(state.worker || state.enabled || state.busy || !menu_has(&state,FALSE,FALSE))return 12;
    state.stopping=TRUE;state.busy=TRUE;state.enabled=TRUE;state.next_style=NTW16S_GLOBAL_STYLE_2K;
    worker_stopped(&state,ERROR_ACCESS_DENIED);
    if(state.worker || state.enabled || state.busy || state.next_style!=STYLE_NONE)return 13;
    CloseHandle(stop_event);
    printf("TRAY CONTROLLER PASS bits=%u switch=both toggle=off failure=1 mutually_exclusive=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
