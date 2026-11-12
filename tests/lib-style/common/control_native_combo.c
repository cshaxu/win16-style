#include "lib-style/common/control_native_combo.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NATIVE COMBO FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static HWND accepted;
static BOOL accept_parent(HWND window){return window==accepted;}
int main(void)
{
    INITCOMMONCONTROLSEX common={.dwSize=sizeof(common),.dwICC=ICC_DATE_CLASSES};
    CHECK(InitCommonControlsEx(&common));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,0,0,150,100,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(combo);
    HMODULE owner=(HMODULE)GetClassLongPtrW(combo,GCLP_HMODULE);BOOL detached=FALSE;HWND expected=parent;
    style_control_native_combo_view view={&detached,&expected,&owner};COMBOBOXINFO ci={.cbSize=sizeof(ci)};
    CHECK(style_control_color_identity(combo,&view,&ci));
    detached=TRUE;CHECK(!style_control_color_identity(combo,&view,&ci));detached=FALSE;
    expected=NULL;CHECK(style_control_color_identity(combo,&view,&ci));expected=combo;
    CHECK(!style_control_color_identity(combo,&view,&ci));expected=parent;
    HWND date=CreateWindowW(DATETIMEPICK_CLASSW,L"",WS_CHILD,0,30,150,24,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(date);
    owner=(HMODULE)GetClassLongPtrW(date,GCLP_HMODULE);DATETIMEPICKERINFO di={.cbSize=sizeof(di)};
    CHECK(style_control_date_identity(date,&view,&di));
    detached=TRUE;CHECK(!style_control_date_identity(date,&view,&di));detached=FALSE;
    owner=NULL;CHECK(!style_control_date_identity(date,&view,&di));
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
