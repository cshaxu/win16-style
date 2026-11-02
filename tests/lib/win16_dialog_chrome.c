#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include <dlgs.h>
#include "lib/win16-dialog/chrome.h"
#include "lib/win16-dialog/ole_window.h"
#include "lib/win16-dialog/classic_folder.h"
#include "lib/win16-dialog/file_hook.h"
#include "lib/win16-dialog/file_live.h"
#include "lib/win16-dialog/com_hook.h"
#include "lib/win16-dialog/proxy.h"
static int grids,scrolls,removes,combos;static const void *grid_owner;static BOOL same_owner=TRUE;
static BOOL fake_grid(HWND w,const void *owner){(void)w;++grids;grid_owner=owner;return TRUE;}
static BOOL fake_scroll(HWND w){(void)w;++scrolls;return FALSE;}
static BOOL fake_remove(HWND w,const void *owner){(void)w;++removes;if(owner!=grid_owner)same_owner=FALSE;return TRUE;}
static BOOL fake_combo(HWND w,const void *owner){(void)w;(void)owner;++combos;return TRUE;}
static UINT_PTR CALLBACK fake_folder_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return 0;}
static INT_PTR CALLBACK fake_dialog_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return FALSE;}
static int chrome_calls,observer_calls;
static void fake_chrome(HWND w){(void)w;++chrome_calls;}
static int color_found,color_removed;static BOOL color_order=TRUE;
static void fake_color_found(HWND w){(void)w;++color_found;}
static void fake_color_remove(HWND w){(void)w;if(!color_found)color_order=FALSE;++color_removed;}
static UINT_PTR fake_observer(HWND w,UINT message,WPARAM wp,LPARAM lp,void *context)
{(void)w;(void)message;(void)wp;(void)lp;if(context==&observer_calls)++observer_calls;return 0;}
static int com_attached,com_detached,com_selected,com_navigated,com_before,com_after,com_owner;
static void fake_com_attach(void *owner,HWND w,OPENFILENAMEW *ofn){(void)w;(void)ofn;if(owner==&com_owner)++com_attached;}
static PCWSTR fake_com_label(void *owner,int which){(void)owner;return which==0?L"Use":which==1?L"Stop":L"Name";}
static void fake_com_detach(void *owner,HWND w){(void)w;if(owner==&com_owner)++com_detached;}
static BOOL fake_com_command(void *owner,HWND w,WPARAM wp,UINT_PTR *result)
{(void)w;if(owner==&com_owner && LOWORD(wp)==123){*result=9;return TRUE;}return FALSE;}
static BOOL fake_com_before(void *owner,HWND w,UINT m,WPARAM wp,UINT_PTR *result)
{(void)owner;(void)w;(void)m;(void)wp;(void)result;++com_before;return FALSE;}
static void fake_com_selection(void *owner,HWND w,WPARAM wp){(void)w;if(owner==&com_owner && wp==lst2)++com_selected;}
static void fake_com_navigate(void *owner,HWND w,PCWSTR key){(void)w;(void)key;if(owner==&com_owner)++com_navigated;}
static BOOL fake_com_after(void *owner,HWND w,UINT m,LPARAM lp,UINT_PTR *result)
{(void)owner;(void)w;(void)m;(void)lp;(void)result;++com_after;return FALSE;}
int main(void)
{
    HWND parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)return 15;
    HWND list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,0,0,20,20,parent,(HMENU)(INT_PTR)lst1,GetModuleHandleW(NULL),NULL);
    HWND plain=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    HWND combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)cmb2,GetModuleHandleW(NULL),NULL);
    if(!list||!plain||!combo){DestroyWindow(parent);return 16;}
    const win16_dialog_chrome_services services={fake_grid,fake_scroll,fake_remove,fake_combo};
    win16_dialog_old_chrome(parent,&services);win16_dialog_font_chrome(parent,&services);
    win16_dialog_ole_window adapter;HWND active=parent,actual=NULL;
    win16_dialog_ole_window_init(&adapter,NULL,NULL,&active,NULL);
    HRESULT missing=IOleWindow_GetWindow(&adapter.iface,NULL);
    HRESULT current=IOleWindow_GetWindow(&adapter.iface,&actual);
    HWND edit=CreateWindowExW(0,L"EDIT",L"*.*",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)1152,GetModuleHandleW(NULL),NULL);
    HWND label=CreateWindowExW(0,L"STATIC",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)1090,GetModuleHandleW(NULL),NULL);
    HWND cancel=CreateWindowExW(0,L"BUTTON",L"Cancel",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)IDCANCEL,GetModuleHandleW(NULL),NULL);
    if(!edit||!label||!cancel){DestroyWindow(parent);return 19;}
    win16_dialog_classic_folder_context context={0};context.run_message=WM_APP+50;context.file_label=L"Folder test:";
    context.experimental=TRUE;context.observer=fake_observer;context.observer_context=&observer_calls;
    OPENFILENAMEW ofn={0};ofn.lCustData=(LPARAM)&context;
    win16_dialog_classic_folder_hook(parent,WM_INITDIALOG,0,(LPARAM)&ofn,&context,fake_chrome);
    win16_dialog_classic_folder_hook(parent,context.run_message,0,0,&context,fake_chrome);
    WCHAR label_text[64],edit_text[64];GetWindowTextW(label,label_text,64);GetWindowTextW(edit,edit_text,64);
    BOOL initialized=chrome_calls==1 && context.classic_window==parent && GetPropW(parent,L"Win16s.FolderBridge")==&context &&
        GetDlgItem(parent,0x5717)!=NULL && !lstrcmpW(label_text,L"Folder test:") && !lstrcmpW(edit_text,L"[Select current folder]");
    win16_dialog_classic_folder_context prepared_context={0};OPENFILENAMEW prepared_ofn;WCHAR file[MAX_PATH],path[MAX_PATH];
    HRESULT prepared=win16_dialog_classic_folder_prepare(&prepared_context,&prepared_ofn,file,parent,NULL,L"C:\\Windows",NULL,NULL,NULL,fake_folder_hook);
    BOOL prepare_ok=prepared==S_OK && prepared_ofn.lpfnHook==fake_folder_hook && prepared_ofn.lCustData==(LPARAM)&prepared_context &&
        !lstrcmpW(file,L"C:\\Windows\\[Select current folder]");
    HRESULT finished=win16_dialog_classic_folder_finish(TRUE,0,file,path,MAX_PATH);
    BOOL finish_ok=finished==S_OK && !lstrcmpW(path,L"C:\\Windows") &&
        win16_dialog_classic_folder_finish(FALSE,0,file,path,MAX_PATH)==HRESULT_FROM_WIN32(ERROR_CANCELLED);
    HWND filter_combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,0,20,100,parent,(HMENU)(INT_PTR)cmb1,GetModuleHandleW(NULL),NULL);
    if(!filter_combo){DestroyWindow(parent);return 24;}
    SendMessageW(filter_combo,CB_ADDSTRING,0,(LPARAM)L"First");SendMessageW(filter_combo,CB_ADDSTRING,0,(LPARAM)L"Second");
    SendMessageW(filter_combo,CB_SETCURSEL,1,0);
    WCHAR *filter_text=L"test",*live_name=NULL;UINT filter_index=0;
    HRESULT live_filter=win16_dialog_live_get_filter(NULL,&active,&filter_text,&filter_index);
    HRESULT live_name_hr=win16_dialog_live_get_name(NULL,&active,&live_name);
    BOOL live_ok=live_filter==S_OK && filter_index==2 && live_name_hr==S_OK && live_name &&
        !lstrcmpW(live_name,L"[Select current folder]") && win16_dialog_live_get_name(NULL,&active,NULL)==E_POINTER;
    if(live_name)CoTaskMemFree(live_name);
    const win16_dialog_com_hook_services com_services={L"Win16S.Test.Navigation",fake_com_attach,fake_com_label,fake_com_detach,
        fake_com_command,fake_com_before,fake_com_selection,fake_com_navigate,fake_com_after};
    OPENFILENAMEW com_ofn={0};com_ofn.lCustData=(LPARAM)&com_owner;
    win16_dialog_com_hook(parent,WM_INITDIALOG,0,(LPARAM)&com_ofn,&com_services);
    UINT_PTR command_result=win16_dialog_com_hook(parent,WM_COMMAND,MAKEWPARAM(123,0),0,&com_services);
    UINT selection_message=RegisterWindowMessageW(LBSELCHSTRINGW),navigation_message=RegisterWindowMessageW(com_services.navigation_message);
    win16_dialog_com_hook(parent,selection_message,lst2,0,&com_services);
    UINT_PTR navigation_result=win16_dialog_com_hook(parent,navigation_message,0,0,&com_services);
    win16_dialog_com_hook(parent,WM_NCDESTROY,0,0,&com_services);
    BOOL com_ok=com_attached==1 && com_detached==1 && com_selected==1 && com_navigated==1 &&
        com_before==2 && com_after==1 && command_result==9 && navigation_result==1 &&
        !GetPropW(parent,L"Win16Style.T20.ComDialog");
    win16_dialog_proxy proxy={0};proxy.active_window=parent;IShellItem *folder_item=(IShellItem *)(UINT_PTR)1;
    win16_dialog_proxy_capture_folder(&proxy,FALSE);
    BOOL folder_state_ok=proxy.live_folder_error==E_FAIL && !proxy.live_folder &&
        win16_dialog_proxy_get_folder(&proxy,NULL)==E_POINTER &&
        win16_dialog_proxy_get_folder(&proxy,&folder_item)==E_FAIL && !folder_item &&
        win16_dialog_proxy_set_folder(&proxy,NULL,L"Win16S.Test.Navigation")==E_INVALIDARG;
    WCHAR windows_dir[MAX_PATH],after_navigation[64];WCHAR *saved_name=NULL;
    BOOL nav_ready=GetWindowsDirectoryW(windows_dir,MAX_PATH)>0 && SetPropW(parent,L"Win16S.Test.Proxy",&proxy);
    if(nav_ready)win16_dialog_proxy_navigate_contents(parent,&proxy,L"Win16S.Test.Proxy",windows_dir,&saved_name,TRUE);
    GetWindowTextW(edit,after_navigation,64);
    BOOL nav_ok=nav_ready && saved_name && !lstrcmpW(saved_name,L"[Select current folder]") &&
        !lstrcmpW(after_navigation,L"[Select current folder]") && !proxy.classic_navigating;
    if(saved_name)CoTaskMemFree(saved_name);
    RemovePropW(parent,L"Win16S.Test.Proxy");
    win16_dialog_classic_folder_hook(parent,WM_NCDESTROY,0,0,&context,fake_chrome);
    BOOL released=!context.classic_window && !GetPropW(parent,L"Win16s.FolderBridge") && observer_calls==2;
    DestroyWindow(parent);
    if(!initialized || !released || !prepare_ok || !finish_ok || !live_ok || !com_ok || !folder_state_ok || !nav_ok)return 20;
    struct {DLGTEMPLATE dialog;WORD menu,window_class,title;} template={0};template.dialog.style=WS_POPUP;
    HWND dialog=CreateDialogIndirectParamW(GetModuleHandleW(NULL),&template.dialog,NULL,fake_dialog_proc,0);
    if(!dialog)return 21;
    HWND folder_list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    if(!folder_list){DestroyWindow(dialog);return 22;}
    if(win16_dialog_is_font_template(dialog)){DestroyWindow(dialog);return 25;}
    for(int id=1136;id<=1138;++id)
        if(!CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL)){DestroyWindow(dialog);return 26;}
    if(!win16_dialog_is_font_template(dialog) || win16_dialog_is_font_template(parent)){DestroyWindow(dialog);return 27;}
    win16_dialog_color_initialized(dialog,fake_color_found,fake_color_remove);
    if(color_found || color_removed){DestroyWindow(dialog);return 28;}
    const int color_ids[]={720,721,710,702,709};
    for(unsigned i=0;i<sizeof(color_ids)/sizeof(color_ids[0]);++i)
        if(!CreateWindowExW(0,L"STATIC",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)color_ids[i],GetModuleHandleW(NULL),NULL)){DestroyWindow(dialog);return 29;}
    win16_dialog_color_initialized(dialog,fake_color_found,fake_color_remove);
    if(color_found!=1 || color_removed!=5 || !color_order){DestroyWindow(dialog);return 30;}
    int before=chrome_calls;win16_dialog_file_initialized(dialog,fake_chrome);
    win16_dialog_file_hook(dialog,WM_INITDIALOG,0,0,FALSE,fake_chrome);
    win16_dialog_file_hook(dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),0,FALSE,fake_chrome);
    DestroyWindow(dialog);
    if(chrome_calls!=before+2)return 23;
    if(missing!=E_POINTER || current!=S_OK || actual!=parent)return 18;
    if(grids!=1||scrolls!=1||removes!=1||combos!=2||!same_owner)return 17;
    puts("WIN16_DIALOG_CHROME_OK");return 0;
}
