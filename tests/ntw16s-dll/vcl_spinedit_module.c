#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(int);
typedef void (WINAPI *foreign_set_fn)(void *);
__attribute__((noinline)) static LRESULT foreign_paint(WNDPROC proc,HWND w,UINT m,WPARAM p,LPARAM l)
{volatile LRESULT value=CallWindowProcW(proc,w,m,p,l);return value;}
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
static window_fn remove_style;
static HWND observed;
static BOOL remove_on_color,removed;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_CTLCOLOREDIT && (HWND)l==observed && remove_on_color){remove_on_color=FALSE;removed=remove_style(observed);}
    return DefWindowProcW(w,m,p,l);
}
static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
    reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
    unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");foreign_set_fn set_foreign=(void*)GetProcAddress(fixture,"FixtureSetForeign");CHECK(reg && create && unreg && set_foreign);set_foreign((void*)foreign_paint);
    HWND (WINAPI *add_button)(HWND)=(void*)GetProcAddress(fixture,"FixtureAddButton");
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove_style);
    HWND startup=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(startup);ShowWindow(startup,SW_HIDE);CHECK(DestroyWindow(startup));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclModuleEditParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Module Edit",WS_OVERLAPPEDWINDOW,20,20,400,200,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    for(unsigned mode=0;mode<3;++mode){
        CHECK(reg(mode==0?1:mode==1?2:0));HWND edit=create(root,FALSE);CHECK(edit && IsWindowVisible(edit));
        CHECK((DWORD)SendMessageW(edit,EM_GETLIMITTEXT,0,0)>0);CHECK(!attached(edit));
        HWND child=add_button?add_button(edit):NULL;RECT before={0},after={0},child_rect={0};
        if(add_button){CHECK(child);SendMessageW(edit,EM_GETRECT,0,(LPARAM)&before);CHECK(GetWindowRect(child,&child_rect));MapWindowPoints(NULL,edit,(POINT*)&child_rect,2);CHECK(before.right<=child_rect.left);}
        CHECK(RedrawWindow(edit,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));
        if(mode<2)CHECK(!attached(edit));
        else {
            CHECK(attached(edit));
            if(add_button){
                SendMessageW(edit,EM_GETRECT,0,(LPARAM)&after);CHECK(after.right<=child_rect.left);CHECK(after.right<=before.right);
                SendMessageW(edit,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),FALSE);
                SendMessageW(edit,EM_GETRECT,0,(LPARAM)&after);CHECK(after.right<=child_rect.left);CHECK(after.right<=before.right);
            }
            RECT client;CHECK(GetClientRect(edit,&client));
            CHECK(ValidateRect(edit,NULL));
            int marker_x=client.right-3,marker_y=client.bottom-3;
            HDC dc=GetDC(edit);CHECK(dc && SetPixelV(dc,marker_x,marker_y,RGB(3,254,5)));CHECK(ReleaseDC(edit,dc));
            RECT dirty={1,1,10,10};CHECK(RedrawWindow(edit,&dirty,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
            dc=GetDC(edit);CHECK(dc && GetPixel(dc,marker_x,marker_y)==RGB(3,254,5));CHECK(ReleaseDC(edit,dc));
            CHECK(SendMessageW(edit,WM_SETTEXT,0,(LPARAM)L"probe"));
            SendMessageW(edit,EM_SETSEL,1,3);SendMessageW(edit,WM_CHAR,L'X',1);
            WCHAR text[32];CHECK(GetWindowTextW(edit,text,32)>0 && !wcscmp(text,L"pXbe"));
            CHECK(SendMessageW(edit,EM_SETREADONLY,TRUE,0));SendMessageW(edit,WM_CHAR,L'Y',1);
            CHECK(GetWindowTextW(edit,text,32)>0 && !wcscmp(text,L"pXbe"));CHECK(SendMessageW(edit,EM_SETREADONLY,FALSE,0));
            observed=edit;removed=FALSE;remove_on_color=TRUE;
            CHECK(RedrawWindow(edit,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
            CHECK(removed && !attached(edit));observed=NULL;
            CHECK(GetWindowTextW(edit,text,32)>0 && !wcscmp(text,L"pXbe"));
        }
        CHECK(DestroyWindow(edit));CHECK(unreg());
    }
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    if(add_button)printf("SPINEDIT_LAYOUT_OK reserved=1 font_change=1\n");
    printf("VCL_LABELED_EDIT_OK bits=%u fake_paint_rejected=1 foreign_caller_rejected=1 clip=1 selection=1 readonly=1 reentrant_remove=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
