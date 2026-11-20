#include "tray.h"
#include "lib-win32/tray/tray.h"
#include "global.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/access/global/control.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct tray_theme { unsigned profile,command;WCHAR folder[MAX_PATH]; } tray_theme;
typedef struct controller {
    style_global_control control;tray_icon *tray;HICON icon;
    tray_theme *themes;unsigned count;
} controller;

enum { MENU_ENABLE=1001, MENU_EXIT=1002, MENU_ENABLE_2K=1003, MENU_FIRST_THEME=1100, STYLE_NONE=2 };
static BOOL remember_theme(PCWSTR folder,classic_theme_family family,void *value)
{
    controller *state=value;
    unsigned profile=classic_theme_profile(folder,family);
    WCHAR confirmed[MAX_PATH];
    if(FAILED(classic_theme_profile_folder(GetModuleHandleW(NULL),profile,confirmed)) ||
       _wcsicmp(folder,confirmed))return TRUE;
    tray_theme *items=realloc(state->themes,((size_t)state->count+1)*sizeof(*items));
    if(!items)return FALSE;
    state->themes=items;
    tray_theme *item=&items[state->count];
    item->profile=profile;
    item->command=profile==0?MENU_ENABLE:profile==1?MENU_ENABLE_2K:MENU_FIRST_THEME+state->count;
    wcscpy(item->folder,folder);
    ++state->count;return TRUE;
}
static int order_theme(const void *left,const void *right)
{
    const tray_theme *a=left,*b=right;
    if(a->profile<2 || b->profile<2)return (a->profile>b->profile)-(a->profile<b->profile);
    return _wcsicmp(a->folder,b->folder);
}
static HMENU make_menu(void *value)
{
    controller *state=(controller *)value;
    HMENU menu=CreatePopupMenu();if(!menu)return NULL;
    for(unsigned i=0;i<state->count;++i){
        const tray_theme *item=&state->themes[i];
        UINT flags=MF_STRING|(state->control.desired==item->profile?MF_CHECKED:MF_UNCHECKED);
        WCHAR literal[2*MAX_PATH],*end=literal;
        for(PCWSTR name=item->folder;*name;++name){
            if(*name==L'&')*end++=L'&';
            *end++=*name;
        }
        *end=0;
        PCWSTR label=item->profile==0?L"Enable Win16 Style":item->profile==1?L"Enable Windows 2000 Style":literal;
        if(!AppendMenuW(menu,flags,item->command,label)){
            DestroyMenu(menu);return NULL;
        }
    }
    if(!AppendMenuW(menu,MF_STRING,MENU_EXIT,L"Exit")){DestroyMenu(menu);return NULL;}
    return menu;
}
static DWORD run_global(HANDLE parent,DWORD style,void *context)
{(void)context;return (DWORD)run16s_global_with_parent(parent,style);}
static void failed(DWORD error,void *context)
{(void)error;(void)context;MessageBeep(MB_ICONERROR);}
static const style_global_control_ops operations={run_global,failed,10};
static void action(unsigned command,void *value)
{
    controller *state=(controller *)value;
    if(command==MENU_EXIT)ExitProcess(0);
    unsigned i=0;
    while(i<state->count && state->themes[i].command!=command)++i;
    if(i==state->count)return;
    DWORD selected=state->themes[i].profile;
    if(selected!=state->control.desired){
        classic_theme_mapping source={0};
        HRESULT loaded=classic_theme_open_profile(GetModuleHandleW(NULL),selected,&source);
        if(FAILED(loaded)){failed(HRESULT_CODE(loaded),state);return;}
        classic_theme_close(&source);
    }
    style_global_control_select(&state->control,selected);
}
#ifdef RUN16S_TRAY_TEST
static LRESULT test_control(WPARAM request,void *value)
{
    controller *state=(controller *)value;
    if(request==1)action(MENU_ENABLE,state);
    else if(request==6)action(MENU_ENABLE_2K,state);
    else if(request==2)action(MENU_EXIT,state);
    else if(request==3)return (state->control.enabled?1:0)|(state->control.busy?2:0)|((state->control.enabled && (state->control.style&1))?4:0);
    else if(request==4){
        WCHAR first[64]={0},second[64]={0},third[64]={0};HMENU menu=make_menu(state);if(!menu)return 0;
        BOOL valid=GetMenuItemCount(menu)==3 && GetMenuStringW(menu,0,first,ARRAYSIZE(first),MF_BYPOSITION) &&
            GetMenuStringW(menu,1,second,ARRAYSIZE(second),MF_BYPOSITION) && GetMenuStringW(menu,2,third,ARRAYSIZE(third),MF_BYPOSITION) &&
            !lstrcmpW(first,L"Enable Win16 Style") && !lstrcmpW(second,L"Enable Windows 2000 Style") && !lstrcmpW(third,L"Exit") &&
            !!(GetMenuState(menu,MENU_ENABLE,MF_BYCOMMAND)&MF_CHECKED)==(state->control.desired==NTW16S_GLOBAL_STYLE_16) &&
            !!(GetMenuState(menu,MENU_ENABLE_2K,MF_BYCOMMAND)&MF_CHECKED)==(state->control.desired==NTW16S_GLOBAL_STYLE_2K);
        DestroyMenu(menu);return valid;
    }else if(request==7)return state->count;
    else if(request==5){
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
    controller state={0};
    if(!style_global_control_init(&state.control,parent,NTW16S_GLOBAL_STYLE_16,STYLE_NONE,
        &ntw16s_global_namespace,&operations,NULL)){
        fwprintf(stderr,L"run16s: cannot prepare the tray session\n");return 10;
    }
    /* Group 40's 16x16 artwork has a gray base; its 32x32 artwork is the
     * standalone Windows flag used by the original tray prototype. */
    HICON icon=(HICON)LoadImageW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(101),IMAGE_ICON,
        32,32,LR_DEFAULTCOLOR);
    if(!icon){fwprintf(stderr,L"run16s: cannot load tray icon\n");return 10;}
    state.icon=icon;
    classic_theme_visit_folders(GetModuleHandleW(NULL),remember_theme,&state);
    if(state.count>1)qsort(state.themes,state.count,sizeof(*state.themes),order_theme);
    DWORD error=tray_icon_create(icon,L"Win16 Style",make_menu,action,&state,&state.tray);
    if(error){fwprintf(stderr,L"run16s: cannot create tray icon (%lu)\n",error);free(state.themes);DestroyIcon(icon);return 10;}
#ifdef RUN16S_TRAY_TEST
    tray_test_set_control(state.tray,test_control);
#endif
    int result=10;
    for(;;){
        HANDLE waits[2];DWORD count=0;if(state.control.worker)waits[count++]=state.control.worker;if(parent)waits[count++]=parent;
        DWORD status=MsgWaitForMultipleObjects(count,waits,FALSE,200,QS_ALLINPUT);
        if(state.control.worker && status==WAIT_OBJECT_0){
            DWORD code=0;
            if(style_global_control_finished(&state.control,&code)){result=(int)code;break;}
        }else if(parent && status==WAIT_OBJECT_0+(state.control.worker?1:0)){
            if(!state.control.worker){result=0;break;}
        }else if(status==WAIT_OBJECT_0+count){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(status!=WAIT_TIMEOUT){result=10;break;}
        style_global_control_update(&state.control);
    }
    if(style_global_control_close(&state.control,10000))ExitProcess((UINT)result);
    tray_icon_destroy(state.tray);free(state.themes);DestroyIcon(icon);return result;
}
