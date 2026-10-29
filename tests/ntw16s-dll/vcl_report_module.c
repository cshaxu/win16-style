#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_REPORT_FAIL line=%d error=%lu expr=%s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL,BOOL);typedef BOOL (WINAPI *unreg_fn)(void);typedef HWND (WINAPI *create_fn)(HWND,BOOL);typedef BOOL (__cdecl *window_fn)(HWND);
static int run(int argc,WCHAR **argv)
{
 CHECK(argc==2);INITCOMMONCONTROLSEX ic={sizeof(ic),ICC_TREEVIEW_CLASSES|ICC_LISTVIEW_CLASSES};CHECK(InitCommonControlsEx(&ic));
 HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll"),f=LoadLibraryW(argv[1]);CHECK(dll&&f);
 window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");reg_fn reg=(void*)GetProcAddress(f,"FixtureRegister");unreg_fn unreg=(void*)GetProcAddress(f,"FixtureUnregister");create_fn create=(void*)GetProcAddress(f,"FixtureCreate");CHECK(attached&&remove&&reg&&unreg&&create);
 HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,400,240,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);
 for(int is_tree=0;is_tree<2;is_tree++)for(int mode=0;mode<(is_tree?2:3);mode++){
  CHECK(reg(mode==0,is_tree));HWND w=create(root,mode==2);CHECK(w);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));
  if(mode==0 || mode==2)CHECK(!attached(w));else{
   CHECK(attached(w));CHECK(SendMessageW(w,is_tree?TVM_GETCOUNT:LVM_GETITEMCOUNT,0,0)==1);
   HWND other=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,300,160,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(other);CHECK(SetParent(w,other)==root);SendMessageW(w,WM_NULL,0,0);CHECK(!attached(w));CHECK(SendMessageW(w,is_tree?TVM_GETCOUNT:LVM_GETITEMCOUNT,0,0)==1);CHECK(SetParent(w,root)==other);CHECK(DestroyWindow(other));
  }
  CHECK(DestroyWindow(w));CHECK(unreg());
 }
 CHECK(DestroyWindow(root));CHECK(FreeLibrary(f));printf("VCL_REPORT_MODULE_OK bits=%u fake=1 ownerdraw=1 reparent=1 cleanup=1\n",(unsigned)sizeof(void*)*8);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
