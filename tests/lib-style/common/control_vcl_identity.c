#include "lib-style/common/control_vcl_identity.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("VCL_IDENTITY_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WCHAR *names[]={L"TControlBar",L"TToolBar",L"TStringGrid",L"TButton",L"TDBNavigator",L"TValueListEditor",L"TInplaceEditList",L"TDBLookupComboBox",L"TBitBtn"};
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=DefWindowProcW};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){cls.lpszClassName=names[i];CHECK(RegisterClassW(&cls));}
    HWND host=CreateWindowW(names[0],L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,instance,NULL);CHECK(host);
    HWND toolbar=CreateWindowW(names[1],L"",WS_CHILD,0,0,10,10,host,NULL,instance,NULL);
    HWND grid=CreateWindowExW(WS_EX_CLIENTEDGE,names[2],L"",WS_CHILD,0,0,10,10,host,NULL,instance,NULL);
    HWND button=CreateWindowW(names[3],L"",WS_CHILD|BS_PUSHBUTTON,0,0,10,10,host,NULL,instance,NULL);
    HWND nav=CreateWindowW(names[4],L"",WS_CHILD,0,0,10,10,host,NULL,instance,NULL);
    HWND editor=CreateWindowW(names[5],L"",WS_CHILD,0,0,10,10,host,NULL,instance,NULL);
    HWND inplace=CreateWindowW(names[6],L"",WS_CHILD,0,0,10,10,editor,NULL,instance,NULL);
    HWND lookup=CreateWindowExW(WS_EX_CLIENTEDGE,names[7],L"",WS_CHILD,0,0,10,10,host,NULL,instance,NULL);
    HWND bit=CreateWindowW(names[8],L"",WS_CHILD|BS_OWNERDRAW,0,0,10,10,host,NULL,instance,NULL);
    CHECK(toolbar && grid && button && nav && editor && inplace && lookup && bit);
    CHECK(style_control_vcl_toolbar_identity(toolbar,instance,host));
    CHECK(style_control_vcl_grid_identity(grid,instance,host));
    CHECK(style_control_vcl_group_button_identity(button));
    CHECK(style_control_vcl_nav_identity(nav,instance,host));
    CHECK(style_control_vcl_inplace_identity(inplace,instance,editor));
    CHECK(style_control_vcl_lookup_identity(lookup,instance,host));
    CHECK(style_control_vcl_inplace_or_lookup(lookup,instance,host));
    CHECK(style_control_vcl_bit_identity(bit,instance));
    CHECK(!style_control_vcl_toolbar_identity(toolbar,instance,editor));
    CHECK(!style_control_vcl_grid_identity(grid,instance,editor));
    CHECK(!style_control_vcl_checklist_identity(button));
    CHECK(!style_control_vcl_lookup_popup_identity(host,lookup,host,instance));
    SetWindowLongPtrW(toolbar,GWL_EXSTYLE,WS_EX_CLIENTEDGE);
    CHECK(!style_control_vcl_toolbar_identity(toolbar,instance,host));
    SetWindowLongPtrW(button,GWL_STYLE,WS_CHILD|BS_BITMAP);
    CHECK(!style_control_vcl_group_button_identity(button));
    CHECK(DestroyWindow(host));
    for(unsigned i=0;i<ARRAYSIZE(names);++i)CHECK(UnregisterClassW(names[i],instance));
    puts("VCL_IDENTITY_OK");return 0;
}
