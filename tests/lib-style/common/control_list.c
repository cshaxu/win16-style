#include "lib-style/common/control_list.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("CONTROL_LIST_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,120,100,NULL,NULL,instance,NULL);
    HWND list=parent?CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_NOTIFY,0,0,100,50,parent,NULL,instance,NULL):NULL;
    CHECK(parent && list);
    for(int i=0;i<20;++i)CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"item")!=LB_ERR);
    style_control_list_view before=style_control_list_view_read(list),after=style_control_list_view_read(list);
    CHECK(!style_control_list_view_changed(&before,&after));
    CHECK(SendMessageW(list,LB_SETCURSEL,4,0)!=LB_ERR);
    after=style_control_list_view_read(list);
    CHECK(after.selection==4 && style_control_list_view_changed(&before,&after));
    before=after;CHECK(SendMessageW(list,LB_SETTOPINDEX,8,0)!=LB_ERR);
    after=style_control_list_view_read(list);
    CHECK(after.top==8 && style_control_list_view_changed(&before,&after));
    HWND multi=CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_MULTIPLESEL,0,0,100,50,parent,NULL,instance,NULL);
    CHECK(multi);
    for(int i=0;i<20;++i)CHECK(SendMessageW(multi,LB_ADDSTRING,0,(LPARAM)L"item")!=LB_ERR);
    before=style_control_list_view_read(multi);CHECK(SendMessageW(multi,LB_SETCARETINDEX,10,FALSE)!=LB_ERR);
    after=style_control_list_view_read(multi);
    CHECK(after.caret==10 && style_control_list_view_changed(&before,&after));
    CHECK(DestroyWindow(parent));puts("CONTROL_LIST_OK");return 0;
}
