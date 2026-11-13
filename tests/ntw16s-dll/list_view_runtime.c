#include <windows.h>
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("LIST_VIEW_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    HMODULE style=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;
    CHECK(attached);
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,200,150,NULL,NULL,instance,NULL);
    HWND list=parent?CreateWindowW(L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY,
        0,0,120,60,parent,NULL,instance,NULL):NULL;
    CHECK(parent && list && attached(list));
    for(int i=0;i<20;++i)CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"item")!=LB_ERR);
    CHECK(SendMessageW(list,LB_SETCURSEL,4,0)!=LB_ERR);
    CHECK(SendMessageW(list,LB_SETTOPINDEX,8,0)!=LB_ERR);
    SendMessageW(list,WM_MOUSEMOVE,0,MAKELPARAM(4,4));
    SendMessageW(list,WM_MOUSEMOVE,0,MAKELPARAM(4,4));
    CHECK(attached(list) && SendMessageW(list,LB_GETCURSEL,0,0)==4 &&
        SendMessageW(list,LB_GETTOPINDEX,0,0)==8);
    CHECK(DestroyWindow(parent));puts("LIST_VIEW_RUNTIME_OK");return 0;
}
