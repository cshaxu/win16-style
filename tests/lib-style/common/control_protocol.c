#include "lib-style/common/control_protocol.h"
#include "lib-style/common/control_class.h"
int main(int argc,char **argv)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSEXW native={.cbSize=sizeof(native)};
    if(!GetClassInfoExW(NULL,L"EDIT",&native))return 4;
    native.hInstance=instance;native.lpszClassName=L"StyleControlSuperclassTest";
    if(!RegisterClassExW(&native))return 5;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent) return 1;
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,parent,NULL,GetModuleHandleW(NULL),NULL);
    if(!child){DestroyWindow(parent);return 2;}
    BOOL ok=!style_control_protocol_bind(child) && !style_control_protocol_acquire(child) &&
        !style_control_protocol_is(child,L"BUTTON") &&
        !style_control_native_superclass(child,L"STATIC");
    HWND edit=CreateWindowW(native.lpszClassName,L"",WS_CHILD,0,0,20,20,parent,NULL,instance,NULL);
    ok=ok && edit && style_control_native_superclass(edit,L"EDIT") &&
        !style_control_native_superclass(edit,L"BUTTON") &&
        style_control_is_edit(edit) && !style_control_is_combo(edit) &&
        style_control_is_static(child) && !style_control_is_edit(child);
    WNDCLASSW fake={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"ThunderRT6ComboBox"};
    ATOM registered=RegisterClassW(&fake);
    HWND impostor=registered?CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,20,20,parent,NULL,instance,NULL):NULL;
    ok=ok && impostor && !style_control_vb_combo(impostor) && !style_control_is_combo(impostor);
    if(impostor)DestroyWindow(impostor);
    if(registered)UnregisterClassW(fake.lpszClassName,instance);
    WNDCLASSW vcl_parent={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"StyleVclParent"};
    WNDCLASSW vcl_combo={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"TComboBox"};
    WNDCLASSW vcl_db={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"TDBComboBox"};
    BOOL classes=RegisterClassW(&vcl_parent) && RegisterClassW(&vcl_combo) && RegisterClassW(&vcl_db);
    HWND host=classes?CreateWindowW(vcl_parent.lpszClassName,L"",WS_POPUP,0,0,20,20,NULL,NULL,instance,NULL):NULL;
    HWND combo=host?CreateWindowW(vcl_combo.lpszClassName,L"",WS_CHILD|CBS_DROPDOWN,0,0,10,10,host,NULL,instance,NULL):NULL;
    HWND db=host?CreateWindowW(vcl_db.lpszClassName,L"",WS_CHILD|CBS_DROPDOWN,0,0,10,10,host,NULL,instance,NULL):NULL;
    ok=ok && host && combo && db && style_control_vcl_plain_combo(combo,host) &&
        style_control_vcl_plain_combo(db,host) && !style_control_vcl_plain_combo(combo,parent);
    if(combo){SetWindowLongPtrW(combo,GWL_STYLE,WS_CHILD|CBS_DROPDOWN|CBS_OWNERDRAWFIXED);
        ok=ok && !style_control_vcl_plain_combo(combo,host);}
    if(db){SetWindowLongPtrW(db,GWL_STYLE,WS_CHILD|CBS_DROPDOWNLIST);
        ok=ok && !style_control_vcl_plain_combo(db,host);}
    if(host)DestroyWindow(host);
    if(classes){UnregisterClassW(vcl_db.lpszClassName,instance);UnregisterClassW(vcl_combo.lpszClassName,instance);UnregisterClassW(vcl_parent.lpszClassName,instance);}
    if(argc>1){
        HMODULE runtime=LoadLibraryA(argv[1]);
        BOOL (WINAPI *reg)(void)=runtime?(void *)GetProcAddress(runtime,"VbFileRegister"):NULL;
        BOOL (WINAPI *release)(void)=runtime?(void *)GetProcAddress(runtime,"VbFileRelease"):NULL;
        unsigned (WINAPI *version)(void)=runtime?(void *)GetProcAddress(runtime,"VbFileVersion"):NULL;
        BOOL ready=reg && release && version && reg();
        const WCHAR *name=ready && version()==5?L"ThunderRT5DirListBox":L"ThunderRT6DirListBox";
        HWND directory=ready?CreateWindowW(name,L"",WS_CHILD|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,
            0,0,20,20,parent,NULL,runtime,NULL):NULL;
        ok=ok && directory && style_control_vb_directory(directory) && style_control_list_class(directory) &&
            !style_control_is_combo(directory);
        if(directory)DestroyWindow(directory);
        if(ready)ok=release() && ok;
        if(runtime)FreeLibrary(runtime);
    }
    style_control_protocol_release(NULL);DestroyWindow(parent);return ok?0:3;
}
