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
