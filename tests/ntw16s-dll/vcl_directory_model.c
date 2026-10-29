#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define C(x) do{if(!(x)){fprintf(stderr,"DIR_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (__cdecl *windowfn)(HWND);
static WNDPROC native;static unsigned draws,measures;
static LRESULT CALLBACK listproc(HWND w,UINT m,WPARAM p,LPARAM l){return CallWindowProcW(native,w,m,p,l);}
static LRESULT CALLBACK rootproc(HWND w,UINT m,WPARAM p,LPARAM l){
 if(m==WM_MEASUREITEM&&l){MEASUREITEMSTRUCT *i=(void*)l;if(i->CtlType==ODT_LISTBOX){i->itemHeight=20;measures++;return TRUE;}}
 if(m==WM_DRAWITEM&&l){DRAWITEMSTRUCT*i=(void*)l;if(i->CtlType==ODT_LISTBOX){FillRect(i->hDC,&i->rcItem,GetSysColorBrush(COLOR_WINDOW));SetTextColor(i->hDC,RGB(200,50,20));TextOutW(i->hDC,i->rcItem.left+3,i->rcItem.top+2,L"owned",5);draws++;return TRUE;}}
 return DefWindowProcW(w,m,p,l);
}
static int compare(HWND w,windowfn remove){
 RECT outer,client;POINT origin={0};C(GetWindowRect(w,&outer)&&GetClientRect(w,&client)&&ClientToScreen(w,&origin));int width=outer.right-outer.left,height=outer.bottom-outer.top;C(width>0&&height>0&&width<1024&&height<1024);
 HDC win=GetWindowDC(w),dc=CreateCompatibleDC(win);C(win&&dc);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
 DWORD *bits=NULL;HBITMAP bm=CreateDIBSection(win,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);C(bm&&bits);HGDIOBJ old=SelectObject(dc,bm);C(old);DWORD *before=malloc((size_t)width*height*4);C(before);
 C(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));C(BitBlt(dc,0,0,width,height,win,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*4);
 C(remove(w));C(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));C(BitBlt(dc,0,0,width,height,win,0,0,SRCCOPY));GdiFlush();unsigned edge=0,content=0;int x0=origin.x-outer.left,y0=origin.y-outer.top;
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)if((before[y*width+x]&0xffffff)!=(bits[y*width+x]&0xffffff)){if(x>=x0&&x<x0+client.right&&y>=y0&&y<y0+client.bottom)content++;else edge++;}
 printf("DIR_PIXELS edge=%u content=%u\n",edge,content);free(before);SelectObject(dc,old);C(DeleteObject(bm)&&DeleteDC(dc)&&ReleaseDC(w,win));C(edge>0&&content==0);return 0;
}
static int worker(int argc,WCHAR**argv){
 (void)argc;(void)argv;setvbuf(stdout,NULL,_IONBF,0);HINSTANCE h=GetModuleHandleW(NULL);HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");C(dll);windowfn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");C(attached&&remove);
 WNDCLASSEXW wc={.cbSize=sizeof(wc)};C(GetClassInfoExW(NULL,L"LISTBOX",&wc));native=wc.lpfnWndProc;wc.hInstance=h;wc.lpfnWndProc=listproc;wc.style&=~CS_GLOBALCLASS;const WCHAR*names[]={L"TDirectoryListBox",L"TFileListBox"};for(unsigned i=0;i<2;i++){wc.lpszClassName=names[i];C(RegisterClassExW(&wc));}
 WNDCLASSEXW rootclass={.cbSize=sizeof(rootclass),.hInstance=h,.lpfnWndProc=rootproc,.lpszClassName=L"DirectoryModelHost"};C(RegisterClassExW(&rootclass));HWND root=CreateWindowW(L"DirectoryModelHost",L"",WS_OVERLAPPEDWINDOW,0,0,450,300,NULL,NULL,h,NULL);C(root);ShowWindow(root,SW_SHOWNOACTIVATE);
 for(unsigned n=0;n<2;n++){
  draws=measures=0;HWND list=CreateWindowExW(WS_EX_CLIENTEDGE,names[n],L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_NOTIFY,10,10,200,100,root,(HMENU)1,h,NULL);C(list);
  for(int i=0;i<20;i++){C(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"folder")!=LB_ERR);}C(SendMessageW(list,LB_SETCURSEL,2,0)==2);C(SendMessageW(list,LB_SETITEMDATA,2,12345)!=LB_ERR);
  C(RedrawWindow(root,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_FRAME|RDW_UPDATENOW));printf("DIR_MODEL index=%u attached=%d draw=%u measure=%u\n",n,attached(list),draws,measures);C(attached(list)&&draws&&measures);C(compare(list,remove)==0);C(!attached(list)&&SendMessageW(list,LB_GETCURSEL,0,0)==2&&SendMessageW(list,LB_GETITEMDATA,2,0)==12345&&SendMessageW(list,LB_GETCOUNT,0,0)==20);C(DestroyWindow(list));
  HWND noedge=CreateWindowW(names[n],L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,10,10,200,100,root,NULL,h,NULL);C(noedge);C(RedrawWindow(noedge,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(noedge));C(DestroyWindow(noedge));
  HWND noscroll=CreateWindowExW(WS_EX_CLIENTEDGE,names[n],L"",WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,10,10,200,100,root,NULL,h,NULL);C(noscroll);C(RedrawWindow(noscroll,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(noscroll));C(DestroyWindow(noscroll));
  HWND foreign=CreateWindowW(L"STATIC",L"foreign",WS_CHILD|WS_VISIBLE,0,0,250,150,root,NULL,h,NULL);C(foreign);
  HWND wrong=CreateWindowExW(WS_EX_CLIENTEDGE,names[n],L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,10,10,200,100,foreign,NULL,h,NULL);C(wrong);C(RedrawWindow(wrong,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(wrong));C(DestroyWindow(wrong));
  HWND moved=CreateWindowExW(WS_EX_CLIENTEDGE,names[n],L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,10,10,200,100,root,NULL,h,NULL);C(moved);for(int i=0;i<20;i++){C(SendMessageW(moved,LB_ADDSTRING,0,(LPARAM)L"folder")!=LB_ERR);}C(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(moved));C(SetParent(moved,foreign)==root);C(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(moved));C(DestroyWindow(moved)&&DestroyWindow(foreign));
 }
 C(DestroyWindow(root));C(UnregisterClassW(L"DirectoryModelHost",h));for(unsigned i=0;i<2;i++)C(UnregisterClassW(names[i],h));puts("DIR_OK");return 0;
}
int wmain(int argc,WCHAR**argv){return isolated_process(argc,argv,worker);}
