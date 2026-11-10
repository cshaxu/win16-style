#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"VCL_MASK_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x); return 1; } } while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
typedef void (WINAPI *foreign_fn)(WNDPROC);
static LRESULT CALLBACK foreign_paint(HWND w,UINT m,WPARAM p,LPARAM l)
{WNDCLASSEXA native={.cbSize=sizeof(native)};if(!GetClassInfoExA(NULL,"EDIT",&native))return 0;return CallWindowProcA(native.lpfnWndProc,w,m,p,l);}
/* Return zero on any probe failure as well as a missing outer frame. */
#undef CHECK
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 0;}}while(0)
#include "vcl_edit_frame.h"

#undef CHECK
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE f=LoadLibraryW(argv[1]);CHECK(f);
    reg_fn reg=(void*)GetProcAddress(f,"FixtureRegister"),reg_db=(void*)GetProcAddress(f,"FixtureRegisterDbEdit");create_fn create=(void*)GetProcAddress(f,"FixtureCreate");unreg_fn unreg=(void*)GetProcAddress(f,"FixtureUnregister");foreign_fn set_foreign=(void*)GetProcAddress(f,"FixtureSetForeign");CHECK(reg && reg_db && create && unreg && set_foreign);set_foreign(foreign_paint);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);
    for(unsigned role=0;role<2;++role)for(unsigned fake=0;fake<(role?3u:2u);++fake){CHECK((role?reg_db:reg)(fake));HWND w=create(root,FALSE);CHECK(w);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));
        if(fake)CHECK(!attached(w));else{CHECK(attached(w));CHECK(SendMessageW(w,WM_SETTEXT,0,(LPARAM)L"probe"));SendMessageW(w,EM_SETSEL,1,3);SendMessageW(w,WM_CHAR,L'X',1);WCHAR text[32];CHECK(GetWindowTextW(w,text,32)>0 && !wcscmp(text,L"pXbe"));CHECK(SendMessageW(w,EM_SETREADONLY,TRUE,0));SendMessageW(w,WM_CHAR,L'Y',1);CHECK(GetWindowTextW(w,text,32)>0 && !wcscmp(text,L"pXbe"));CHECK(SendMessageW(w,EM_SETREADONLY,FALSE,0));CHECK(frame_pixels(w,remove));CHECK(!attached(w));CHECK(GetWindowTextW(w,text,32)>0 && !wcscmp(text,L"pXbe"));}
        CHECK(DestroyWindow(w)&&unreg());}
    CHECK(DestroyWindow(root)&&FreeLibrary(f));printf("VCL_MASK_MODEL_OK bits=%u fake=1 selection=1 readonly=1 frame=1 removal=1 ansi_dbedit=1 foreign=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
