#include "lib-style/win16-control/control_class.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"CONTROL_CLASS_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2);
    HMODULE runtime=LoadLibraryW(argv[1]);CHECK(runtime);
    BOOL (WINAPI *reg)(void)=(void *)GetProcAddress(runtime,"VbRegister");
    void (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbRelease");
    CHECK(reg && release && reg());
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND check=CreateWindowW(L"ThunderRT6CheckBox",L"",WS_CHILD|BS_AUTO3STATE,0,0,40,20,root,NULL,runtime,NULL);
    HWND option=CreateWindowW(L"ThunderRT6OptionButton",L"",WS_CHILD|BS_RADIOBUTTON,0,0,40,20,root,NULL,runtime,NULL);
    HWND wrong=CreateWindowW(L"ThunderRT5CheckBox",L"",WS_CHILD|BS_AUTO3STATE,0,0,40,20,root,NULL,runtime,NULL);
    HWND frame=CreateWindowW(L"ThunderRT6Frame",L"",WS_CHILD,0,0,40,20,root,NULL,runtime,NULL);
    HWND list=CreateWindowW(L"ThunderRT6ListBox",L"",WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL,0,0,40,20,root,NULL,runtime,NULL);
    CHECK(check && option && wrong && frame && list);
    CHECK(ntw16s_vb_plain_choice(check) && ntw16s_vb_plain_choice(option));
    CHECK(!ntw16s_vb_graphical_choice(check) && !ntw16s_vb_plain_choice(wrong));
    SetWindowLongPtrW(check,GWL_STYLE,GetWindowLongPtrW(check,GWL_STYLE)|BS_PUSHLIKE);
    CHECK(!ntw16s_vb_plain_choice(check) && ntw16s_vb_graphical_choice(check));
    SetWindowLongPtrW(option,GWL_STYLE,(GetWindowLongPtrW(option,GWL_STYLE)&~BS_TYPEMASK)|BS_OWNERDRAW);
    CHECK(ntw16s_vb_graphical_button(option) && !ntw16s_vb_graphical_choice(option));
    CHECK(style_control_button_type(frame,BS_PUSHBUTTON)==BS_GROUPBOX);
    CHECK(style_control_button_type(wrong,BS_AUTOCHECKBOX)==BS_AUTOCHECKBOX);
    UINT dpi=GetDpiForWindow(list);
    if(dpi==96 || dpi==144){
        CHECK(ntw16s_vb_checklist_owner(list) && ntw16s_vb_owner_file(list));
        SetWindowLongPtrW(list,GWL_EXSTYLE,GetWindowLongPtrW(list,GWL_EXSTYLE)|WS_EX_RTLREADING);
        CHECK(!ntw16s_vb_checklist_owner(list) && !ntw16s_vb_owner_file(list));
    }
    CHECK(DestroyWindow(root));release();CHECK(FreeLibrary(runtime));
    printf("CONTROL_CLASS_OK plain=1 graphical=1 ownerdraw=1 wrong_runtime=1 frame=1 checklist_dpi=%u\n",dpi);return 0;
}
