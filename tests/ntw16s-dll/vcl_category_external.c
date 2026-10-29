#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CATEGORY_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
typedef BOOL (__cdecl *windowfn)(HWND);
static int compare(HWND w,windowfn remove){
    RECT outer,client;POINT origin={0};CHECK(GetWindowRect(w,&outer)&&GetClientRect(w,&client)&&ClientToScreen(w,&origin));
    int width=outer.right-outer.left,height=outer.bottom-outer.top;CHECK(width>0&&height>0&&width<1024&&height<1024);
    HDC window=GetWindowDC(w),dc=CreateCompatibleDC(window);CHECK(window&&dc);
    BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    DWORD *bits=NULL;HBITMAP bitmap=CreateDIBSection(window,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(bitmap&&bits);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);
    DWORD *before=malloc((size_t)width*height*sizeof(*before));CHECK(before);
    CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*sizeof(*before));
    CHECK(remove(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();
    unsigned edge=0,content=0;int x0=origin.x-outer.left,y0=origin.y-outer.top;for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(before[y*width+x]!=bits[y*width+x]){if(x>=x0&&x<x0+client.right&&y>=y0&&y<y0+client.bottom)++content;else ++edge;}
    printf("POPUP_PIXELS edge=%u client=%u\n",edge,content);CHECK(edge>0&&content==0);free(before);SelectObject(dc,old);CHECK(DeleteObject(bitmap)&&DeleteDC(dc)&&ReleaseDC(w,window));return 0;
}
static int worker(int argc,WCHAR **argv){CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);BOOL (WINAPI *reg)(void)=(void*)GetProcAddress(fixture,"FixtureRegister"), (WINAPI *unreg)(void)=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg&&unreg&&reg());HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);windowfn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove);
HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);HWND parent=CreateWindowW(L"CategoryOwnedParent",L"",WS_CHILD|WS_VISIBLE,0,0,220,180,root,NULL,fixture,NULL);CHECK(parent);SCROLLINFO si={.cbSize=sizeof(si),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=0};
HWND wrong=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,fixture,NULL);CHECK(wrong);SetScrollInfo(wrong,SB_VERT,&si,TRUE);CHECK(RedrawWindow(wrong,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(wrong));CHECK(DestroyWindow(wrong));
HWND noedge=CreateWindowW(L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(noedge);SetScrollInfo(noedge,SB_VERT,&si,TRUE);CHECK(RedrawWindow(noedge,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(noedge));CHECK(DestroyWindow(noedge));
HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(w);SetScrollInfo(w,SB_VERT,&si,TRUE);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(w));CHECK(compare(w,remove)==0);CHECK(DestroyWindow(w));
HWND moved=CreateWindowExW(WS_EX_CLIENTEDGE,L"TCategoryButtons",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,parent,NULL,fixture,NULL);CHECK(moved);SetScrollInfo(moved,SB_VERT,&si,TRUE);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(moved));CHECK(SetParent(moved,root)==parent);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(moved));CHECK(DestroyWindow(moved));CHECK(DestroyWindow(parent));CHECK(unreg());CHECK(FreeLibrary(fixture));puts("CATEGORY_EXTERNAL_OK positive=1 owner=1 noedge=1 reparent=1 client=1 root_os_cleanup=1");return 0;}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
