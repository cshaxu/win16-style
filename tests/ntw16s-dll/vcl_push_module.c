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
#ifdef T18_CHECK
typedef void (WINAPI *reparent_fn)(HWND,HWND);
#endif
static LRESULT WINAPI foreign_paint(WNDPROC proc,HWND w,UINT m,WPARAM p,LPARAM l)
{return CallWindowProcW(proc,w,m,p,l);}

#ifdef T18_CHECK
static BOOL parent_background(HWND w)
{
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};void *pixels=NULL;info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=180;info.bmiHeader.biHeight=-30;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=dc?CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0):NULL;HGDIOBJ old=bitmap?SelectObject(dc,bitmap):NULL;
    if(!old){if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);return FALSE;}
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();COLORREF pixel=GetPixel(dc,170,15);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);return pixel==RGB(42,123,56);
}
#endif
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
#ifdef T18_CHECK
    reparent_fn reparent_on_background=(void*)GetProcAddress(fixture,"FixtureReparentOnBackground");CHECK(reparent_on_background);
#endif
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,400,180,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNA);
    for(int mode=0;mode<3;mode++){
        CHECK(reg(mode==0?1:mode==2?2:0));HWND parent=group(root),a=create(parent,TRUE),b=create(parent,FALSE);CHECK(parent && a && b);
        HFONT font=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");
        CHECK(font);SendMessageW(a,WM_SETFONT,(WPARAM)font,FALSE);
        CHECK(RedrawWindow(a,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(RedrawWindow(b,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(mode!=1)CHECK(!attached(a) && !attached(b));
        else {
            CHECK(attached(a) && attached(b));unsigned before=clicks();
#ifdef T18_CHECK
            CHECK(SendMessageW(a,BM_GETCHECK,0,0)==BST_UNCHECKED);CHECK(parent_background(a));
#endif
            SendMessageW(a,BM_CLICK,0,0);CHECK(clicks()==before+1);
#ifdef T18_CHECK
            CHECK(SendMessageW(a,BM_GETCHECK,0,0)==BST_CHECKED);
#endif
            SetFocus(a);CHECK(GetFocus()==a);SendMessageW(a,WM_KEYDOWN,VK_SPACE,0);SendMessageW(a,WM_KEYUP,VK_SPACE,0);CHECK(clicks()==before+2);
#ifdef T18_CHECK
            CHECK(SendMessageW(a,BM_GETCHECK,0,0)==BST_UNCHECKED);SendMessageW(a,BM_SETCHECK,BST_INDETERMINATE,0);CHECK(SendMessageW(a,BM_GETCHECK,0,0)==BST_INDETERMINATE);
#endif
#ifdef T18_CHECK
            HWND moved=create(parent,TRUE);CHECK(moved);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(moved));reparent_on_background(moved,root);(void)parent_background(moved);CHECK(GetParent(moved)==root);CHECK(!attached(moved));CHECK(DestroyWindow(moved));
#endif
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
#ifdef T18_CHECK
    printf("VCL_CHECK_MODULE_OK bits=%u fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 states=3 background=1 background_reparent=1 reparent=1 role=1 font_restore=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
#else
    printf("VCL_PUSH_MODULE_OK bits=%u fake=1 foreign=1 wrong_parent=1 bitmap=1 native=1 click=1 keyboard=1 reparent=1 role=1 font_restore=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
#endif
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
