#include "tray.h"
#include "lib-win32/tray/tray.h"
#include "global.h"
#include "ntw16s-dll/global/session.h"
#include <stdio.h>
#include <wchar.h>

typedef struct controller {
    HANDLE parent,worker;tray_icon *tray;HICON icon;
    BOOL enabled,busy,stopping;
    WCHAR session_name[NTW16S_GLOBAL_NAME_CAP],request_name[NTW16S_GLOBAL_NAME_CAP+16];
} controller;
enum { MENU_ENABLE=1001, MENU_EXIT=1002 };
static HMENU make_menu(void *value)
{
    controller *state=(controller *)value;
    HMENU menu=CreatePopupMenu();if(!menu)return NULL;
    UINT flags=MF_STRING|(state->enabled?MF_CHECKED:MF_UNCHECKED)|(state->busy?MF_GRAYED:0);
    if(!AppendMenuW(menu,flags,MENU_ENABLE,L"Enable Win16 Style") ||
       !AppendMenuW(menu,MF_STRING,MENU_EXIT,L"Exit")){
        DestroyMenu(menu);return NULL;
    }
    return menu;
}
static DWORD WINAPI worker_main(void *value)
{controller *state=(controller *)value;return (DWORD)run16s_global_with_parent(state->parent);}
static BOOL start_worker(controller *state)
{
    state->worker=CreateThread(NULL,0,worker_main,state,0,NULL);
    if(!state->worker)return FALSE;
    state->enabled=TRUE;state->busy=TRUE;state->stopping=FALSE;
    return TRUE;
}
static BOOL session_ready(controller *state)
{
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,state->session_name);if(!mapping)return FALSE;
    ntw16s_global_session *s=(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s));
    BOOL ready=s && s->magic==NTW16S_GLOBAL_MAGIC && s->version==NTW16S_GLOBAL_VERSION &&
        s->owner_pid==GetCurrentProcessId() && s->phase==NTW16S_GLOBAL_ACTIVE && s->ready;
    if(s)UnmapViewOfFile(s);
    CloseHandle(mapping);return ready;
}
static void action(unsigned command,void *value)
{
    controller *state=(controller *)value;
    if(command==MENU_EXIT)ExitProcess(0);
    if(command!=MENU_ENABLE || state->busy)return;
    if(!state->enabled){if(!start_worker(state))MessageBeep(MB_ICONERROR);return;}
    HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state->request_name);
    if(!request){MessageBeep(MB_ICONERROR);return;}
    BOOL sent=SetEvent(request);CloseHandle(request);
    if(!sent){MessageBeep(MB_ICONERROR);return;}
    state->stopping=TRUE;state->busy=TRUE;
}
#ifdef RUN16S_TRAY_TEST
static LRESULT test_control(WPARAM request,void *value)
{
    controller *state=(controller *)value;
    if(request==1 && !state->busy)action(MENU_ENABLE,state);
    else if(request==2)action(MENU_EXIT,state);
    else if(request==3)return (state->enabled?1:0)|(state->busy?2:0);
    else if(request==4){
        WCHAR first[64]={0},second[64]={0};HMENU menu=make_menu(state);if(!menu)return 0;
        BOOL valid=GetMenuItemCount(menu)==2 && GetMenuStringW(menu,0,first,ARRAYSIZE(first),MF_BYPOSITION) &&
            GetMenuStringW(menu,1,second,ARRAYSIZE(second),MF_BYPOSITION) && !lstrcmpW(first,L"Enable Win16 Style") &&
            !lstrcmpW(second,L"Exit") && !!(GetMenuState(menu,MENU_ENABLE,MF_BYCOMMAND)&MF_CHECKED)==!!state->enabled;
        DestroyMenu(menu);return valid;
    }else if(request==5){
        ICONINFO info={0};BITMAP bitmap={0};if(!GetIconInfo(state->icon,&info))return 0;
        BOOL valid=info.hbmColor && GetObjectW(info.hbmColor,sizeof(bitmap),&bitmap)==sizeof(bitmap) &&
            bitmap.bmWidth==32 && bitmap.bmHeight==32;
        if(info.hbmColor)DeleteObject(info.hbmColor);
        if(info.hbmMask)DeleteObject(info.hbmMask);
        return valid;
    }
    return 0;
}
#endif
int run16s_tray_global(HANDLE parent)
{
    controller state={0};state.parent=parent;
    if(!ntw16s_global_name(state.session_name,ARRAYSIZE(state.session_name)) ||
       swprintf(state.request_name,ARRAYSIZE(state.request_name),L"%ls.request",state.session_name)<0){
        fwprintf(stderr,L"run16s: cannot prepare the tray session\n");return 10;
    }
    /* Group 40's 16x16 artwork has a gray base; its 32x32 artwork is the
     * standalone Windows flag used by the original tray prototype. */
    HICON icon=(HICON)LoadImageW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(101),IMAGE_ICON,
        32,32,LR_DEFAULTCOLOR);
    if(!icon){fwprintf(stderr,L"run16s: cannot load tray icon\n");return 10;}
    state.icon=icon;
    DWORD error=tray_icon_create(icon,L"Win16 Style",make_menu,action,&state,&state.tray);
    if(error){fwprintf(stderr,L"run16s: cannot create tray icon (%lu)\n",error);DestroyIcon(icon);return 10;}
#ifdef RUN16S_TRAY_TEST
    tray_test_set_control(state.tray,test_control);
#endif
    int result=10;
    for(;;){
        HANDLE waits[2];DWORD count=0;if(state.worker)waits[count++]=state.worker;if(parent)waits[count++]=parent;
        DWORD status=MsgWaitForMultipleObjects(count,waits,FALSE,200,QS_ALLINPUT);
        if(state.worker && status==WAIT_OBJECT_0){
            DWORD code=10;GetExitCodeThread(state.worker,&code);CloseHandle(state.worker);state.worker=NULL;
            if(!state.stopping){result=(int)code;break;}
            state.stopping=FALSE;state.enabled=FALSE;state.busy=FALSE;
        }else if(parent && status==WAIT_OBJECT_0+(state.worker?1:0)){
            if(!state.worker){result=0;break;}
        }else if(status==WAIT_OBJECT_0+count){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(status!=WAIT_TIMEOUT){result=10;break;}
        if(state.worker && state.busy && !state.stopping && session_ready(&state)){
            state.busy=FALSE;
        }
    }
    if(state.worker){HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state.request_name);
        if(request){SetEvent(request);CloseHandle(request);}
        if(WaitForSingleObject(state.worker,10000)==WAIT_OBJECT_0)CloseHandle(state.worker);
        else ExitProcess((UINT)result);
    }
    tray_icon_destroy(state.tray);DestroyIcon(icon);return result;
}
