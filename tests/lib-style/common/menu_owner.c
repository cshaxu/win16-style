#include <windows.h>
#include "lib-style/common/menu_owner.h"

static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
int main(void)
{
    static const WCHAR name[]=L"StyleMenuOwnerTest",property[]=L"StyleMenuOwnerTest.Marker";
    static int marker,other;
    WNDCLASSW cls={0};cls.lpfnWndProc=window_proc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=name;
    if(!RegisterClassW(&cls))return 1;
    HWND w=CreateWindowExW(WS_EX_TOOLWINDOW,name,L"",WS_POPUP,0,0,10,10,NULL,NULL,cls.hInstance,NULL);
    if(!w)return 2;
    int result=0;
    if(style_menu_owner_eligible(w,property,&marker))result=3;
    else if(style_menu_owner_mark(w,(const void *)window_proc,L"WrongClass",property,&marker))result=4;
    else if(style_menu_owner_mark(w,(const void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"GetCurrentProcessId"),name,property,&marker))result=5;
    else if(!style_menu_owner_mark(w,(const void *)window_proc,name,property,&marker))result=6;
    else if(!style_menu_owner_eligible(w,property,&marker) || style_menu_owner_eligible(w,property,&other))result=7;
    else {
        SetWindowLongPtrW(w,GWL_STYLE,WS_POPUP|WS_VISIBLE);
        if(style_menu_owner_eligible(w,property,&marker))result=8;
        SetWindowLongPtrW(w,GWL_STYLE,WS_POPUP);
        SetWindowLongPtrW(w,GWL_EXSTYLE,0);
        if(style_menu_owner_eligible(w,property,&marker))result=9;
    }
    RemovePropW(w,property);DestroyWindow(w);UnregisterClassW(name,cls.hInstance);
    return result;
}
