#define UNICODE
#include <windows.h>
#include <stdio.h>
#include "isolated_process.h"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"PUSH_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef HWND (WINAPI *group_fn)(HWND);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef unsigned (WINAPI *count_fn)(void);
typedef LRESULT (WINAPI *foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);
typedef void (WINAPI *set_foreign_fn)(foreign_fn);
typedef BOOL (__cdecl *window_fn)(HWND);
static LRESULT WINAPI foreign_paint(WNDPROC proc,HWND w,UINT m,WPARAM p,LPARAM l)
{return CallWindowProcW(proc,w,m,p,l);}

static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll"),fixture=LoadLibraryW(argv[1]);
    CHECK(dll && fixture);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");
    reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");
    group_fn group=(void*)GetProcAddress(fixture,"FixtureGroup");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
    count_fn clicks=(void*)GetProcAddress(fixture,"FixtureClicks");set_foreign_fn foreign=(void*)GetProcAddress(fixture,"FixtureForeign");
    CHECK(attached && remove && reg && unreg && group && create && clicks && foreign);foreign(foreign_paint);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,400,180,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNA);
    for(int mode=0;mode<3;mode++){
        CHECK(reg(mode==0?1:mode==2?2:0));HWND parent=group(root),a=create(parent,TRUE),b=create(parent,FALSE);CHECK(parent && a && b);
        HFONT font=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");
        CHECK(font);SendMessageW(a,WM_SETFONT,(WPARAM)font,FALSE);
        CHECK(RedrawWindow(a,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(RedrawWindow(b,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(mode!=1)CHECK(!attached(a) && !attached(b));
        else {
            CHECK(attached(a) && attached(b));unsigned before=clicks();SendMessageW(a,BM_CLICK,0,0);CHECK(clicks()==before+1);
            SetFocus(a);CHECK(GetFocus()==a);SendMessageW(a,WM_KEYDOWN,VK_SPACE,0);SendMessageW(a,WM_KEYUP,VK_SPACE,0);CHECK(clicks()==before+2);
            HWND wrong_parent=create(root,TRUE);CHECK(wrong_parent);CHECK(RedrawWindow(wrong_parent,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(wrong_parent));CHECK(DestroyWindow(wrong_parent));
            HWND image=create(parent,FALSE);CHECK(image);SetWindowLongPtrW(image,GWL_STYLE,(DWORD)GetWindowLongPtrW(image,GWL_STYLE)|BS_BITMAP);
            CHECK(RedrawWindow(image,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(image));CHECK(DestroyWindow(image));
            CHECK(SetParent(b,root)==parent);CHECK(RedrawWindow(b,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(b));
            SetWindowLongPtrW(a,GWL_STYLE,((DWORD)GetWindowLongPtrW(a,GWL_STYLE)&~BS_TYPEMASK)|BS_OWNERDRAW);
            CHECK(RedrawWindow(a,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(a));CHECK((HFONT)SendMessageW(a,WM_GETFONT,0,0)==font);
        }
        CHECK(DestroyWindow(b));CHECK(DestroyWindow(parent));CHECK(DeleteObject(font));CHECK(unreg());
    }
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    printf("VCL_PUSH_MODULE_OK bits=%u fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 reparent=1 role=1 font_restore=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
