#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include <dlgs.h>
#include "lib-win32-style/application/dialog/classic_dialog_controls.h"
#include "lib-win32-style/application/dialog/dialog_ole_window.h"
#include "lib-win32-style/application/dialog/classic_file_hook.h"
#include "lib-win32-style/application/dialog/classic_file_live.h"
#include "lib-win32-style/application/dialog/classic_com_hook.h"
#include "lib-win32-style/application/dialog/classic_dialog_proxy.h"
static int grids,scrolls,removes,combos;static const void *grid_owner,*expected_owner;static BOOL same_owner=TRUE;
static BOOL fake_grid(HWND w,const void *owner){(void)w;++grids;grid_owner=owner;if(owner!=expected_owner)same_owner=FALSE;return TRUE;}
static BOOL fake_scroll(HWND w){(void)w;++scrolls;return FALSE;}
static BOOL fake_remove(HWND w,const void *owner){(void)w;++removes;if(owner!=grid_owner)same_owner=FALSE;return TRUE;}
static BOOL fake_combo(HWND w,const void *owner){(void)w;if(owner!=expected_owner)same_owner=FALSE;++combos;return TRUE;}
static INT_PTR CALLBACK fake_dialog_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return FALSE;}
static int chrome_calls;
static void fake_chrome(HWND w){(void)w;++chrome_calls;}
static int color_found,color_removed;static BOOL color_order=TRUE;
static void fake_color_found(HWND w){(void)w;++color_found;}
static void fake_color_remove(HWND w){(void)w;if(!color_found)color_order=FALSE;++color_removed;}
static style_classic_dialog_proxy com_owner;
static unsigned com_commands,com_selected,com_navigated,com_before,com_after;
static BOOL fake_com_change(HWND w,style_classic_dialog_proxy *owner)
{(void)w;if(owner==&com_owner)++com_commands;return FALSE;}
static HRESULT fake_com_event(style_classic_dialog_proxy *owner,enum style_classic_event_kind kind,IShellItem *item)
{(void)item;if(owner==&com_owner && kind==STYLE_CLASSIC_FOLDER_CHANGED)++com_selected;return S_OK;}
static UINT_PTR fake_com_file_ok(style_classic_dialog_proxy *owner,OPENFILENAMEW *ofn)
{(void)ofn;if(owner==&com_owner)++com_after;return 7;}
static void fake_com_schedule(HWND w,style_classic_dialog_proxy *owner)
{(void)w;if(owner==&com_owner)++com_before;}
static void fake_com_initial(HWND w,style_classic_dialog_proxy *owner,PCWSTR key)
{(void)w;(void)owner;(void)key;}
static void fake_com_navigate(HWND w,style_classic_dialog_proxy *owner,PCWSTR key)
{(void)w;(void)key;if(owner==&com_owner)++com_navigated;}
static style_classic_dialog_proxy *live_current;
static HRESULT native_live_result;
static BOOL native_live_detach;
static HRESULT STDMETHODCALLTYPE native_set_index(IFileDialog *self,UINT index)
{(void)self;(void)index;return native_live_result;}
static HRESULT STDMETHODCALLTYPE native_get_index(IFileDialog *self,UINT *index)
{(void)self;if(!index)return E_POINTER;*index=99;return S_FALSE;}
static HRESULT STDMETHODCALLTYPE native_set_name(IFileDialog *self,LPCWSTR name)
{(void)self;(void)name;if(native_live_detach)live_current->active_window=NULL;return native_live_result;}
static HRESULT STDMETHODCALLTYPE native_get_name(IFileDialog *self,LPWSTR *name)
{(void)self;if(!name)return E_POINTER;*name=NULL;return E_ABORT;}
static BOOL test_live_setters(style_classic_dialog_proxy *proxy,HWND edit)
{
    IFileDialogVtbl vtable={.SetFileTypeIndex=native_set_index,.GetFileTypeIndex=native_get_index,
        .SetFileName=native_set_name,.GetFileName=native_get_name};
    IFileDialog native={.lpVtbl=&vtable};HWND window=proxy->active_window;WCHAR text[64];
    proxy->native=&native;live_current=proxy;native_live_result=S_FALSE;
    if(style_classic_file_live_set_filter(proxy,5)!=S_FALSE || proxy->filter_index!=5)return FALSE;
    native_live_result=E_ACCESSDENIED;
    if(style_classic_file_live_set_filter(proxy,7)!=E_ACCESSDENIED || proxy->filter_index!=5 ||
        style_classic_file_live_set_name(proxy,L"uncommitted")!=E_ACCESSDENIED)return FALSE;
    GetWindowTextW(edit,text,ARRAYSIZE(text));if(lstrcmpW(text,L"[Select current folder]"))return FALSE;
    native_live_result=S_FALSE;
    if(style_classic_file_live_set_name(proxy,L"changed")!=S_FALSE)return FALSE;
    GetWindowTextW(edit,text,ARRAYSIZE(text));if(lstrcmpW(text,L"changed"))return FALSE;
    native_live_detach=TRUE;
    if(style_classic_file_live_set_name(proxy,L"must stay native")!=S_FALSE || proxy->active_window)return FALSE;
    GetWindowTextW(edit,text,ARRAYSIZE(text));if(lstrcmpW(text,L"changed"))return FALSE;
    UINT index=0;LPWSTR name=(LPWSTR)(UINT_PTR)1;
    if(style_classic_file_live_get_filter(proxy,&index)!=S_FALSE || index!=99 ||
       style_classic_file_live_get_name(proxy,&name)!=E_ABORT || name)return FALSE;
    proxy->active_window=window;proxy->filter=NULL;index=0;
    if(style_classic_file_live_get_filter(proxy,&index)!=S_FALSE || index!=99)return FALSE;
    proxy->native=NULL;native_live_detach=FALSE;live_current=NULL;
    return SetWindowTextW(edit,L"[Select current folder]");
}
int main(void)
{
    HWND parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)return 15;
    HWND list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,0,0,20,20,parent,(HMENU)(INT_PTR)lst1,GetModuleHandleW(NULL),NULL);
    HWND plain=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    HWND combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)cmb2,GetModuleHandleW(NULL),NULL);
    if(!list||!plain||!combo){DestroyWindow(parent);return 16;}
    static int file_owner,font_owner;
    const style_classic_dialog_controls services={fake_grid,fake_scroll,fake_remove,fake_combo};
    expected_owner=&file_owner;style_classic_file_controls(parent,&file_owner,&services);
    expected_owner=&font_owner;style_classic_font_controls(parent,&font_owner,&services);
    style_dialog_ole_window adapter;HWND active=parent,actual=NULL;
    style_dialog_ole_window_init(&adapter,NULL,NULL,&active,NULL);
    HRESULT missing=IOleWindow_GetWindow(&adapter.iface,NULL);
    HRESULT current=IOleWindow_GetWindow(&adapter.iface,&actual);
    HWND edit=CreateWindowExW(0,L"EDIT",L"*.*",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)1152,GetModuleHandleW(NULL),NULL);
    HWND label=CreateWindowExW(0,L"STATIC",L"",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)1090,GetModuleHandleW(NULL),NULL);
    HWND cancel=CreateWindowExW(0,L"BUTTON",L"Cancel",WS_CHILD,0,0,20,20,parent,(HMENU)(INT_PTR)IDCANCEL,GetModuleHandleW(NULL),NULL);
    if(!edit||!label||!cancel){DestroyWindow(parent);return 19;}
    SetWindowTextW(edit,L"[Select current folder]");
    HWND filter_combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,0,20,100,parent,(HMENU)(INT_PTR)cmb1,GetModuleHandleW(NULL),NULL);
    if(!filter_combo){DestroyWindow(parent);return 24;}
    SendMessageW(filter_combo,CB_ADDSTRING,0,(LPARAM)L"First");SendMessageW(filter_combo,CB_ADDSTRING,0,(LPARAM)L"Second");
    SendMessageW(filter_combo,CB_SETCURSEL,1,0);
    WCHAR *filter_text=L"test",*live_name=NULL;UINT filter_index=0;
    style_classic_dialog_proxy live={.active_window=active,.filter=filter_text};
    HRESULT live_filter=style_classic_file_live_get_filter(&live,&filter_index);
    HRESULT live_name_hr=style_classic_file_live_get_name(&live,&live_name);
    BOOL live_ok=live_filter==S_OK && filter_index==2 && live_name_hr==S_OK && live_name &&
        !lstrcmpW(live_name,L"[Select current folder]") && style_classic_file_live_get_name(&live,NULL)==E_POINTER;
    if(live_name)CoTaskMemFree(live_name);
    live_ok=live_ok && test_live_setters(&live,edit);
    style_dialog_event_adapter event={0};
    com_owner.events=&event;com_owner.ok_label=L"Use";com_owner.cancel_label=L"Stop";com_owner.file_label=L"Name";
    const style_classic_dialog_message_services messages={.events_enabled=TRUE,.hidden_enabled=TRUE,
        .property=L"Style.Test.CustomDialog",.hidden_message=L"Style.Test.Hidden",
        .typed_changing=fake_com_change,.list_changing=fake_com_change,.drive_changing=fake_com_change,
        .overwrite_check=fake_com_change,.event_dispatch=fake_com_event,.file_ok=fake_com_file_ok,
        .initial=fake_com_initial,.hidden_schedule=fake_com_schedule};
    const style_classic_com_hook_services com_services={&messages,L"Style.Test.Navigation",fake_com_navigate};
    OPENFILENAMEW com_ofn={0};com_ofn.lCustData=(LPARAM)&com_owner;
    style_classic_com_hook(parent,WM_INITDIALOG,0,(LPARAM)&com_ofn,&com_services);
    WCHAR label_text[16];GetWindowTextW(cancel,label_text,16);
    BOOL com_attached=com_owner.active_window==parent && com_owner.classic_ofn==&com_ofn &&
        GetPropW(parent,messages.property)==&com_owner && !lstrcmpW(label_text,L"Stop");
    UINT_PTR command_result=style_classic_com_hook(parent,WM_COMMAND,MAKEWPARAM(IDOK,0),0,&com_services);
    UINT selection_message=RegisterWindowMessageW(LBSELCHSTRINGW),navigation_message=RegisterWindowMessageW(com_services.navigation_message);
    style_classic_com_hook(parent,selection_message,lst2,0,&com_services);
    UINT_PTR navigation_result=style_classic_com_hook(parent,navigation_message,0,0,&com_services);
    UINT file_ok=RegisterWindowMessageW(FILEOKSTRINGW);
    UINT_PTR file_result=style_classic_com_hook(parent,file_ok,0,(LPARAM)&com_ofn,&com_services);
    style_classic_com_hook(parent,WM_NCDESTROY,0,0,&com_services);
    BOOL com_ok=com_attached && !com_owner.active_window && !com_owner.classic_ofn && com_commands==1 &&
        com_selected==1 && com_navigated==1 && com_before==2 && com_after==1 &&
        command_result==1 && navigation_result==1 && file_result==7 && !GetPropW(parent,messages.property);
    style_classic_dialog_proxy proxy={0};proxy.active_window=parent;IShellItem *folder_item=(IShellItem *)(UINT_PTR)1;
    style_classic_dialog_proxy_capture_folder(&proxy,FALSE);
    BOOL folder_state_ok=proxy.live_folder_error==E_FAIL && !proxy.live_folder &&
        style_classic_dialog_proxy_get_folder(&proxy,NULL)==E_POINTER &&
        style_classic_dialog_proxy_get_folder(&proxy,&folder_item)==E_FAIL && !folder_item &&
        style_classic_dialog_proxy_set_folder(&proxy,NULL,L"Win16S.Test.Navigation")==E_INVALIDARG;
    WCHAR windows_dir[MAX_PATH],after_navigation[64];WCHAR *saved_name=NULL;
    BOOL nav_ready=GetWindowsDirectoryW(windows_dir,MAX_PATH)>0 && SetPropW(parent,L"Win16S.Test.Proxy",&proxy);
    if(nav_ready)style_classic_dialog_proxy_navigate_contents(parent,&proxy,L"Win16S.Test.Proxy",windows_dir,&saved_name,TRUE);
    GetWindowTextW(edit,after_navigation,64);
    BOOL nav_ok=nav_ready && saved_name && !lstrcmpW(saved_name,L"[Select current folder]") &&
        !lstrcmpW(after_navigation,L"[Select current folder]") && !proxy.classic_navigating;
    if(saved_name)CoTaskMemFree(saved_name);
    RemovePropW(parent,L"Win16S.Test.Proxy");
    DestroyWindow(parent);
    if(!live_ok || !com_ok || !folder_state_ok || !nav_ok)return 20;
    struct {DLGTEMPLATE dialog;WORD menu,window_class,title;} template={0};template.dialog.style=WS_POPUP;
    HWND dialog=CreateDialogIndirectParamW(GetModuleHandleW(NULL),&template.dialog,NULL,fake_dialog_proc,0);
    if(!dialog)return 21;
    RECT invalid_edge={0,0,10,10};
    if(style_classic_color_swatch(dialog,NULL,&invalid_edge) ||
       style_classic_color_swatch(NULL,NULL,NULL)){DestroyWindow(dialog);return 31;}
    HWND folder_list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    if(!folder_list){DestroyWindow(dialog);return 22;}
    if(style_classic_font_template(dialog)){DestroyWindow(dialog);return 25;}
    for(int id=1136;id<=1138;++id)
        if(!CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL)){DestroyWindow(dialog);return 26;}
    if(!style_classic_font_template(dialog) || style_classic_font_template(parent)){DestroyWindow(dialog);return 27;}
    if(style_classic_color_template(dialog)){fake_color_found(dialog);style_classic_color_controls(dialog,NULL,0,fake_color_remove);}
    if(color_found || color_removed){DestroyWindow(dialog);return 28;}
    const int color_ids[]={720,721,710,702,709};
    for(unsigned i=0;i<sizeof(color_ids)/sizeof(color_ids[0]);++i)
        if(!CreateWindowExW(0,L"STATIC",L"",WS_CHILD,0,0,20,20,dialog,(HMENU)(INT_PTR)color_ids[i],GetModuleHandleW(NULL),NULL)){DestroyWindow(dialog);return 29;}
    if(style_classic_color_template(dialog)){fake_color_found(dialog);style_classic_color_controls(dialog,color_ids,ARRAYSIZE(color_ids),fake_color_remove);}
    if(color_found!=1 || color_removed!=5 || !color_order){DestroyWindow(dialog);return 30;}
    int before=chrome_calls;style_classic_file_initialized(dialog,fake_chrome);
    style_classic_file_hook(dialog,WM_INITDIALOG,0,0,FALSE,fake_chrome);
    style_classic_file_hook(dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),0,FALSE,fake_chrome);
    DestroyWindow(dialog);
    if(chrome_calls!=before+2)return 23;
    if(missing!=E_POINTER || current!=S_OK || actual!=parent)return 18;
    if(grids!=1||scrolls!=1||removes!=1||combos!=2||!same_owner)return 17;
    puts("WIN16_DIALOG_CHROME_OK");return 0;
}
