#include <windows.h>
#include <stdio.h>
#include "../../src/run16s-exe/tray_controller.c"

static HANDLE stop_event;
static DWORD observed_style,forced_result;
static DWORD theme_error;
static classic_theme_family first_family=CLASSIC_THEME_WIN16;
enum { first_id=15, second_id=31, color_first=22, color_second=38 };
unsigned classic_theme_folder_id(PCWSTR folder)
{return !wcscmp(folder,L"Win16")?first_id:!wcscmp(folder,L"Win2K")?second_id:47;}
unsigned classic_theme_color_id(PCWSTR folder,PCWSTR color)
{(void)folder;return !wcscmp(color,L"original")?color_first:color_second;}
HRESULT msstyles_v3_color_at(const msstyles_v3_theme *theme,size_t i,PCWSTR *token,
    msstyles_v3_text *label)
{
    if(!theme->resources || i>=2)return S_FALSE;
    static const PCWSTR tokens[]={L"original",L"alternate"}, labels[]={L"Warm & Bright",L"Silver"};
    *token=tokens[i];
    *label=(msstyles_v3_text){labels[i],wcslen(labels[i])};
    return S_OK;
}
HRESULT classic_theme_profile_folder(HMODULE renderer,unsigned profile,WCHAR folder[MAX_PATH])
{(void)renderer;wcscpy(folder,profile==first_id?L"Win16":profile==second_id?L"Win2K":L"Paint");return S_OK;}
HRESULT classic_theme_open_profile(HMODULE renderer,unsigned id,classic_theme_mapping *source)
{
    (void)renderer;
    if(id!=first_id && id!=second_id && id!=color_first && id!=color_second)return E_INVALIDARG;
    source->family=id==first_id?first_family:CLASSIC_THEME_WIN2K;
    return HRESULT_FROM_WIN32(theme_error);
}
void classic_theme_close(classic_theme_mapping *source)
{(void)source;}
void classic_theme_visit_folders(HMODULE renderer,classic_theme_visit visit,void *context)
{
    (void)renderer;
    classic_theme_mapping source={.display_name=L"Theme & Name",.family=CLASSIC_THEME_WIN16};
    if(visit(L"Win16",&source,context)){
        source.family=CLASSIC_THEME_WIN2K;
        visit(L"Win2K",&source,context);
    }
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
static BOOL color_menu_has(controller *state,int selected)
{
    HMENU menu=make_menu(state);if(!menu)return FALSE;
    HMENU colors=GetSubMenu(menu,0);
    WCHAR first[64],second[64];
    BOOL valid=GetMenuItemCount(menu)==2 && colors && GetMenuItemCount(colors)==2 &&
        GetMenuStringW(colors,0,first,ARRAYSIZE(first),MF_BYPOSITION) &&
        GetMenuStringW(colors,1,second,ARRAYSIZE(second),MF_BYPOSITION) &&
        !wcscmp(first,L"Warm && Bright") && !wcscmp(second,L"Silver") &&
        !!(GetMenuState(colors,0,MF_BYPOSITION)&MF_CHECKED)==(selected==0) &&
        !!(GetMenuState(colors,1,MF_BYPOSITION)&MF_CHECKED)==(selected==1);
    DestroyMenu(menu);
    return valid;
}

static BOOL check_color_menu(controller *state)
{
    classic_theme_mapping source={.v3.resources=(HMODULE)(UINT_PTR)1};
    if(!remember_theme(L"Paint",&source,state) || !color_menu_has(state,-1))return FALSE;
    DWORD code;
    DWORD expected=classic_theme_encode_selection((classic_theme_selection){color_second,CLASSIC_THEME_WIN2K});
    action(MENU_FIRST_THEME,state);
    if(!state->control.worker || !color_menu_has(state,0))return FALSE;
    action(MENU_FIRST_THEME+1,state);
    if(!color_menu_has(state,1) || !wait_stopped(state,&code) || code ||
       style_global_control_finished(&state->control,&code))return FALSE;
    if(!state->control.worker || state->control.style!=expected)return FALSE;
    action(MENU_FIRST_THEME+1,state);
    return color_menu_has(state,-1) && wait_stopped(state,&code) && !code &&
        !style_global_control_finished(&state->control,&code) && !state->control.worker &&
        observed_style==expected;
}

int main(void)
{
    controller state={0};DWORD code=0;
    WCHAR prefix[80];swprintf(prefix,ARRAYSIZE(prefix),L"Local\\TrayControllerTest.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=7,.version=1};
    DWORD first=classic_theme_encode_selection((classic_theme_selection){first_id,CLASSIC_THEME_WIN16});
    DWORD second=classic_theme_encode_selection((classic_theme_selection){second_id,CLASSIC_THEME_WIN2K});
    if(!style_global_control_init(&state.control,NULL,first,STYLE_NONE,&space,&operations,NULL))return 2;
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
    if(!state.control.worker || state.control.style!=first)return 17;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,TRUE,FALSE))return 5;
    action((MENU_FIRST_THEME+1),&state);
    if(!state.control.stopping || state.control.desired!=second || !wait_stopped(&state,&code) || code)return 6;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.enabled || state.control.style!=second)return 7;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    if(!menu_has(&state,FALSE,TRUE))return 8;
    action(MENU_FIRST_THEME,&state);
    if(state.control.desired!=first || !wait_stopped(&state,&code) || code || observed_style!=second)return 9;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(!state.control.worker || !state.control.busy || state.control.style!=first)return 10;
    state.control.enabled=TRUE;state.control.busy=FALSE;
    theme_error=ERROR_FILE_NOT_FOUND;
    action(MENU_FIRST_THEME,&state);
    theme_error=ERROR_SUCCESS;
    if(state.control.desired!=STYLE_NONE || !wait_stopped(&state,&code) || code)return 11;
    if(style_global_control_finished(&state.control,&code))return 18;
    if(state.control.worker || state.control.enabled || state.control.busy || !menu_has(&state,FALSE,FALSE))return 12;
    first_family=CLASSIC_THEME_WIN2K;
    action(MENU_FIRST_THEME,&state);
    DWORD replaced=classic_theme_encode_selection((classic_theme_selection){first_id,first_family});
    if(state.control.style!=replaced || !menu_has(&state,TRUE,FALSE))return 24;
    action(MENU_FIRST_THEME,&state);
    if(!wait_stopped(&state,&code) || code || style_global_control_finished(&state.control,&code))return 25;
    forced_result=ERROR_ACCESS_DENIED;action((MENU_FIRST_THEME+1),&state);action((MENU_FIRST_THEME+1),&state);
    if(!wait_stopped(&state,&code) || code!=ERROR_ACCESS_DENIED || style_global_control_finished(&state.control,&code))return 19;
    if(state.control.worker || state.control.enabled || state.control.busy || state.control.desired!=STYLE_NONE)return 13;
    free_themes(&state);
    state.themes=NULL;state.count=0;forced_result=0;
    if(!check_color_menu(&state))return 26;
    free_themes(&state);
    CloseHandle(stop_event);
    puts("TRAY COLOR MENU PASS one_submenu two_literal_names same_folder_switch toggle_off");
    printf("TRAY CONTROLLER PASS bits=%u switch=both toggle=off failure=1 mutually_exclusive=1 metadata=1 fallback=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
