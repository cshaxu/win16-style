#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
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
    unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg && create && unreg);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove_style);
    HWND startup=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(startup);ShowWindow(startup,SW_HIDE);CHECK(DestroyWindow(startup));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclModuleEditParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Module Edit",WS_OVERLAPPEDWINDOW,20,20,400,200,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    for(unsigned mode=0;mode<3;++mode){
        CHECK(reg(mode==0));HWND edit=create(root,mode==2);CHECK(edit && IsWindowVisible(edit));
        CHECK((DWORD)SendMessageW(edit,EM_GETLIMITTEXT,0,0)>0);CHECK(!attached(edit));
        CHECK(RedrawWindow(edit,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));
        if(!mode)CHECK(!attached(edit));
        else {
            CHECK(attached(edit));
            if(mode==2){CHECK(SendMessageW(edit,WM_SETTEXT,0,(LPARAM)L"a\r\nb\r\nc\r\nd\r\ne\r\nf\r\ng\r\nh\r\ni\r\nj\r\nk\r\nl"));
                CHECK(SendMessageW(edit,EM_GETLINECOUNT,0,0)>=12);SendMessageW(edit,EM_LINESCROLL,0,3);
                CHECK(SendMessageW(edit,EM_GETFIRSTVISIBLELINE,0,0)>0);}
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
    printf("VCL_EDIT_MODULE_OK bits=%u fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
