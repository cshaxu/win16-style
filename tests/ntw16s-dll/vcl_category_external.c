#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CATEGORY_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
typedef BOOL (__cdecl *windowfn)(HWND);
#include "vcl_frame_compare.h"
static int worker(int argc,WCHAR **argv){CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);BOOL (WINAPI *reg)(void)=(void*)GetProcAddress(fixture,"FixtureRegister"), (WINAPI *unreg)(void)=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg&&unreg&&reg());HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);windowfn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove);
HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);HWND parent=CreateWindowW(L"CategoryOwnedParent",L"",WS_CHILD|WS_VISIBLE,0,0,220,180,root,NULL,fixture,NULL);CHECK(parent);SCROLLINFO si={.cbSize=sizeof(si),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=0};
HWND wrong=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,fixture,NULL);CHECK(wrong);SetScrollInfo(wrong,SB_VERT,&si,TRUE);CHECK(RedrawWindow(wrong,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(wrong));CHECK(DestroyWindow(wrong));
HWND noedge=CreateWindowW(L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(noedge);SetScrollInfo(noedge,SB_VERT,&si,TRUE);CHECK(RedrawWindow(noedge,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(noedge));CHECK(DestroyWindow(noedge));
HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(w);SetScrollInfo(w,SB_VERT,&si,TRUE);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(w));CHECK(compare(w,remove)==0);CHECK(DestroyWindow(w));
HWND moved=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(moved);SetScrollInfo(moved,SB_VERT,&si,TRUE);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(moved));CHECK(SetParent(moved,root)==parent);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(moved));CHECK(DestroyWindow(moved));CHECK(DestroyWindow(parent));CHECK(unreg());CHECK(FreeLibrary(fixture));puts("CATEGORY_EXTERNAL_OK positive=1 owner=1 noedge=1 reparent=1 client=1 root_os_cleanup=1");return 0;}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
