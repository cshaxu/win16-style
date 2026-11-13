#include "lib-style/common/control_native_witness.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("NATIVE_WITNESS_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"StyleWitnessTest"};
    WNDCLASSEXW wide={.cbSize=sizeof(wide)};
    WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};
    CHECK(RegisterClassW(&cls));
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPED,0,0,10,10,NULL,NULL,instance,NULL);
    CHECK(w && GetClassInfoExW(NULL,L"EDIT",&wide) && GetClassInfoExA(NULL,"EDIT",&ansi));
    CHECK(style_control_native_witness(w,wide.lpfnWndProc,(const void *)main,L"EDIT",TRUE)==instance);
    CHECK(style_control_native_witness(w,ansi.lpfnWndProc,(const void *)main,"EDIT",FALSE)==instance);
    CHECK(!style_control_native_witness(w,DefWindowProcW,(const void *)main,L"EDIT",TRUE));
    CHECK(!style_control_native_witness(w,wide.lpfnWndProc,(const void *)DefWindowProcW,L"EDIT",TRUE));
    CHECK(!style_control_native_witness(w,wide.lpfnWndProc,(const void *)main,L"MissingStyleWitness",TRUE));
    CHECK(DestroyWindow(w));CHECK(UnregisterClassW(cls.lpszClassName,instance));
    puts("NATIVE_WITNESS_OK");return 0;
}
