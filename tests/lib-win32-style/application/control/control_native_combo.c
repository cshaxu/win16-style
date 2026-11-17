#include "lib-win32-style/application/control/control_native_combo.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NATIVE COMBO FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static HWND accepted;
static style_instance *observed;
static LRESULT CALLBACK detach_query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;if(m==CB_GETCOUNT)observed->detached=TRUE;return DefSubclassProc(w,m,a,b);}
static BOOL accept_parent(HWND window){return window==accepted;}
int main(void)
{
    INITCOMMONCONTROLSEX common={.dwSize=sizeof(common),.dwICC=ICC_DATE_CLASSES};
    CHECK(InitCommonControlsEx(&common));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,0,0,150,100,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(combo);
    style_instance instance={0};
    style_control_native_combo_state state={.owner=(HMODULE)GetClassLongPtrW(combo,GCLP_HMODULE),.parent=parent};
    COMBOBOXINFO ci={.cbSize=sizeof(ci)};
    CHECK(style_control_color_identity(combo,&instance,&state,&ci));
    instance.detached=TRUE;CHECK(!style_control_color_identity(combo,&instance,&state,&ci));instance.detached=FALSE;
    state.parent=NULL;CHECK(style_control_color_identity(combo,&instance,&state,&ci));state.parent=combo;
    CHECK(!style_control_color_identity(combo,&instance,&state,&ci));state.parent=parent;
    state.candidate=TRUE;SetLastError(0x1a2b);
    style_control_native_combo_settle(combo,WM_PRINTCLIENT,&instance,&state,FALSE);
    CHECK(!state.attempted && !state.installed && GetLastError()==0x1a2b);
    style_control_native_combo_settle(combo,WM_PAINT,&instance,&state,FALSE);
    CHECK(state.attempted && state.installed && state.parent==parent && GetLastError()==0x1a2b);
    style_control_native_combo_state failed={.candidate=TRUE,.owner=state.owner,.parent=combo};
    style_control_native_combo_settle(combo,WM_PAINT,&instance,&failed,FALSE);CHECK(failed.attempted && !failed.installed);
    failed.parent=parent;style_control_native_combo_settle(combo,WM_PAINT,&instance,&failed,FALSE);CHECK(!failed.installed);
    failed=(style_control_native_combo_state){.candidate=TRUE,.owner=state.owner};observed=&instance;
    CHECK(SetWindowSubclass(combo,detach_query,1,0));
    style_control_native_combo_settle(combo,WM_PAINT,&instance,&failed,FALSE);
    CHECK(instance.detached && failed.attempted && !failed.installed);
    CHECK(RemoveWindowSubclass(combo,detach_query,1));instance.detached=FALSE;
    HWND date=CreateWindowW(DATETIMEPICK_CLASSW,L"",WS_CHILD,0,30,150,24,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(date);
    state.owner=(HMODULE)GetClassLongPtrW(date,GCLP_HMODULE);DATETIMEPICKERINFO di={.cbSize=sizeof(di)};
    CHECK(style_control_date_identity(date,&instance,&state,&di));
    instance.detached=TRUE;CHECK(!style_control_date_identity(date,&instance,&state,&di));instance.detached=FALSE;
    state.owner=NULL;CHECK(!style_control_date_identity(date,&instance,&state,&di));
    state=(style_control_native_combo_state){.candidate=TRUE,.owner=(HMODULE)GetClassLongPtrW(date,GCLP_HMODULE)};
    SetLastError(0x3b4c);style_control_native_combo_settle(date,WM_PAINT,&instance,&state,TRUE);
    CHECK(state.attempted && state.installed && state.parent==parent && GetLastError()==0x3b4c);
    HWND editable=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWN,0,60,150,100,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(editable);
    COMBOBOXINFO edit_info={.cbSize=sizeof(edit_info)};CHECK(GetComboBoxInfo(editable,&edit_info) && edit_info.hwndItem && edit_info.hwndItem!=editable);
    accepted=editable;CHECK(style_control_combo_edit(edit_info.hwndItem,accept_parent));
    CHECK(!style_control_combo_edit(edit_info.hwndItem,NULL) && !style_control_combo_edit(combo,accept_parent));
    accepted=combo;CHECK(!style_control_combo_edit(edit_info.hwndItem,accept_parent));
    HWND classic=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,
        0,90,150,80,parent,(HMENU)(INT_PTR)1136,GetModuleHandleW(NULL),NULL);CHECK(classic);
    accepted=classic;CHECK(style_control_dialog_combo_valid(classic,parent,accept_parent));
    CHECK(!style_control_dialog_combo_valid(classic,parent,NULL) && !style_control_dialog_combo_valid(classic,NULL,accept_parent));
    SetWindowLongPtrW(classic,GWLP_ID,1141);CHECK(!style_control_dialog_combo_valid(classic,parent,accept_parent));
    CHECK(DestroyWindow(parent));
    puts("NATIVE COMBO IDENTITY PASS combo=1 date=1 edit_relation=1 dialog=1 detach=1 parent=1 owner=1");return 0;
}
