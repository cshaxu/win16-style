#include "lib-win32-style/application/control/dialog_combo.h"
#include <stdio.h>
static HWND parent,outer,inner;
static unsigned calls[3],errors;
static BOOL nested,fail_main,fail_children,fail_info,destroy_main;
static int owner_token;
static const style_dialog_combo_ops operations;
static BOOL apply(HWND window,style_dialog_combo_part part,BOOL border,const void *owner)
{
    ++calls[part];
    if(owner!=&owner_token)++errors;
    if(part==STYLE_DIALOG_COMBO){
        if(style_control_dialog_combo_pending()!=window || border)++errors;
        if(nested){nested=FALSE;if(!style_control_dialog_combo_install(inner,owner,&operations) ||
            style_control_dialog_combo_pending()!=window)++errors;}
        if(destroy_main){DestroyWindow(window);return TRUE;}
        if(fail_main){SetLastError(ERROR_BAD_FORMAT);return FALSE;}
    }else{
        /* Inner child work runs after its pending identity restores outer. */
        HWND pending=style_control_dialog_combo_pending();
        if(pending && pending!=outer)++errors;
        if(part==STYLE_DIALOG_COMBO_EDIT && border!=((GetWindowLongPtrW(window,GWL_STYLE)&WS_BORDER)!=0))++errors;
        if(fail_children){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    }
    return TRUE;
}
static BOOL get_info(HWND window,PCOMBOBOXINFO info)
{if(fail_info){SetLastError(ERROR_INVALID_DATA);return FALSE;}return GetComboBoxInfo(window,info);}
static const style_dialog_combo_ops operations={apply,get_info};
static HWND combo(int id)
{return CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWN|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,
    0,0,150,100,parent,(HMENU)(UINT_PTR)id,GetModuleHandleW(NULL),NULL);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DIALOG_COMBO_FAIL line=%d error=%lu checks=%u\n",__LINE__,GetLastError(),errors);return 1;}}while(0)
int main(void)
{
    parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    outer=combo(1136);inner=combo(1137);CHECK(outer && inner && !style_control_dialog_combo_pending());
    CHECK(style_control_dialog_combo_install(outer,&owner_token,&operations));
    CHECK(calls[0]==1 && calls[1]==1 && calls[2]==1 && !errors && !style_control_dialog_combo_pending());
    unsigned before=calls[0];SetWindowLongPtrW(outer,GWLP_ID,1141);
    CHECK(!style_control_dialog_combo_install(outer,&owner_token,&operations) && GetLastError()==ERROR_NOT_SUPPORTED && calls[0]==before);
    SetWindowLongPtrW(outer,GWLP_ID,1136);fail_main=TRUE;before=calls[1];
    CHECK(!style_control_dialog_combo_install(outer,&owner_token,&operations) && GetLastError()==ERROR_BAD_FORMAT && calls[1]==before && !style_control_dialog_combo_pending());
    fail_main=FALSE;fail_children=TRUE;
    CHECK(style_control_dialog_combo_install(outer,&owner_token,&operations) && GetLastError()==ERROR_ACCESS_DENIED && !style_control_dialog_combo_pending());
    fail_children=FALSE;fail_info=TRUE;before=calls[1];
    CHECK(style_control_dialog_combo_install(outer,&owner_token,&operations) && GetLastError()==ERROR_INVALID_DATA && calls[1]==before);
    fail_info=FALSE;nested=TRUE;before=calls[0];
    CHECK(style_control_dialog_combo_install(outer,&owner_token,&operations) && calls[0]==before+2 && !errors && !style_control_dialog_combo_pending());
    destroy_main=TRUE;before=calls[1];
    CHECK(style_control_dialog_combo_install(outer,&owner_token,&operations) && !IsWindow(outer) && calls[1]==before && !style_control_dialog_combo_pending());
    CHECK(DestroyWindow(parent));
    printf("DIALOG_COMBO_OK bits=%u native_children=1 pending=1 nested=1 main_failure=1 advisory_errors=1 destroyed=1\n",(unsigned)(sizeof(void *)*8));return 0;
}
