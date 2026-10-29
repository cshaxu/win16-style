#define _WIN32_WINNT 0x0A00
#ifdef NTW16S_TEST_OUTLINE
#define TEST_LOOKUP_CLASS L"TOutline"
#else
#define TEST_LOOKUP_CLASS L"TDBLookupListBox"
#endif
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"POPUP_MODEL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
typedef BOOL (__cdecl *windowfn)(HWND);
static WNDPROC native_list;
static BOOL fake;
static LRESULT CALLBACK basic(HWND w,UINT m,WPARAM p,LPARAM l){
    WCHAR name[64];BOOL lookup=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,TEST_LOOKUP_CLASS);
    if(lookup && m==WM_APP+100){SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)p);return 0;}
    if(lookup && m==WM_APP+101)return GetWindowLongPtrW(w,GWLP_USERDATA);
    if(lookup && m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);SetDCBrushColor(dc,RGB(12,156,90));FillRect(dc,&r,GetStockObject(DC_BRUSH));SetTextColor(dc,RGB(245,240,80));SetBkMode(dc,TRANSPARENT);TextOutW(dc,10,12,L"Lookup content",14);EndPaint(w,&ps);return 0;}
    return DefWindowProcW(w,m,p,l);
}
static LRESULT CALLBACK list(HWND w,UINT m,WPARAM p,LPARAM l){
    if(m==WM_PAINT && fake){PAINTSTRUCT ps;BeginPaint(w,&ps);EndPaint(w,&ps);return 0;}
    return CallWindowProcW(native_list,w,m,p,l);
}
static HWND make_list(HWND parent,DWORD extra){
    return CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,L"TPopupListbox",L"",WS_CHILD|WS_BORDER|WS_VSCROLL|LBS_HASSTRINGS|extra,0,0,160,100,parent,NULL,GetModuleHandleW(NULL),NULL);
}
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
static int worker(int argc,WCHAR **argv){
    (void)argc;(void)argv;HMODULE self=GetModuleHandleW(NULL),dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    windowfn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove);
    WNDCLASSEXW n={.cbSize=sizeof(n)};CHECK(GetClassInfoExW(NULL,L"LISTBOX",&n));native_list=n.lpfnWndProc;
    WNDCLASSEXW a={.cbSize=sizeof(a),.hInstance=self,.lpfnWndProc=basic,.lpszClassName=L"TValueListEditor"};CHECK(RegisterClassExW(&a));a.lpszClassName=L"TInplaceEditList";CHECK(RegisterClassExW(&a));a.lpszClassName=L"TDBGrid";CHECK(RegisterClassExW(&a));a.lpszClassName=TEST_LOOKUP_CLASS;CHECK(RegisterClassExW(&a));a.lpszClassName=L"LookupListHost";CHECK(RegisterClassExW(&a));
    n.hInstance=self;n.lpfnWndProc=list;n.lpszClassName=L"TPopupListbox";CHECK(RegisterClassExW(&n));
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,self,NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);
    HWND grid=CreateWindowW(L"TValueListEditor",L"",WS_CHILD|WS_VISIBLE,0,0,200,100,root,NULL,self,NULL);CHECK(grid);
    HWND edit=CreateWindowW(L"TInplaceEditList",L"",WS_CHILD|WS_VISIBLE,0,0,180,24,grid,NULL,self,NULL);CHECK(edit);
    HWND wrong=make_list(root,0);CHECK(wrong&&SetParent(wrong,GetDesktopWindow()));ShowWindow(wrong,SW_SHOWNA);CHECK(RedrawWindow(wrong,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(wrong));CHECK(DestroyWindow(wrong));
    HWND ownerdraw=make_list(edit,LBS_OWNERDRAWFIXED);CHECK(ownerdraw&&SetParent(ownerdraw,GetDesktopWindow()));ShowWindow(ownerdraw,SW_SHOWNA);CHECK(RedrawWindow(ownerdraw,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(ownerdraw));CHECK(DestroyWindow(ownerdraw));
    fake=TRUE;HWND w=make_list(edit,0);CHECK(w&&SetParent(w,GetDesktopWindow()));ShowWindow(w,SW_SHOWNA);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(w));
    fake=FALSE;for(int i=0;i<20;++i){WCHAR label[30];swprintf_s(label,30,L"Item %d",i);CHECK(SendMessageW(w,LB_ADDSTRING,0,(LPARAM)label)!=LB_ERR);}HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);
    CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(w));CHECK(SendMessageW(w,LB_GETCOUNT,0,0)==20);CHECK(SendMessageW(w,LB_SETCURSEL,5,0)==5);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==5);
    CHECK(SendMessageW(w,LB_SETTOPINDEX,8,0)!=LB_ERR && SendMessageW(w,LB_GETTOPINDEX,0,0)==8);CHECK(SendMessageW(w,LB_SETTOPINDEX,0,0)!=LB_ERR);
    CHECK(compare(w,remove)==0);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font && SendMessageW(w,LB_GETCURSEL,0,0)==5 && SendMessageW(w,LB_GETCOUNT,0,0)==20);CHECK(DestroyWindow(w));
    HWND shortlist=make_list(edit,0);CHECK(shortlist&&SetParent(shortlist,GetDesktopWindow()));
    CHECK(SendMessageW(shortlist,LB_ADDSTRING,0,(LPARAM)L"First")!=LB_ERR);CHECK(SendMessageW(shortlist,LB_ADDSTRING,0,(LPARAM)L"Second")!=LB_ERR);
    SendMessageW(shortlist,WM_SETFONT,(WPARAM)font,FALSE);ShowWindow(shortlist,SW_SHOWNA);
    CHECK(RedrawWindow(shortlist,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(shortlist));CHECK(!(GetWindowLongPtrW(shortlist,GWL_STYLE)&WS_VSCROLL));
    CHECK(SendMessageW(shortlist,LB_SETCURSEL,1,0)==1);
    for(int i=0;i<18;++i)CHECK(SendMessageW(shortlist,LB_ADDSTRING,0,(LPARAM)L"Extra")!=LB_ERR);
    CHECK(RedrawWindow(shortlist,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(shortlist));CHECK(GetWindowLongPtrW(shortlist,GWL_STYLE)&WS_VSCROLL);
    while(SendMessageW(shortlist,LB_GETCOUNT,0,0)>2)CHECK(SendMessageW(shortlist,LB_DELETESTRING,2,0)!=LB_ERR);
    CHECK(RedrawWindow(shortlist,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(shortlist));CHECK(!(GetWindowLongPtrW(shortlist,GWL_STYLE)&WS_VSCROLL));
    CHECK(SendMessageW(shortlist,LB_GETCURSEL,0,0)==1);CHECK(compare(shortlist,remove)==0);CHECK((HFONT)SendMessageW(shortlist,WM_GETFONT,0,0)==font && SendMessageW(shortlist,LB_GETCOUNT,0,0)==2);CHECK(DestroyWindow(shortlist));
    HWND moved=make_list(edit,0);CHECK(moved&&SetParent(moved,GetDesktopWindow()));for(int i=0;i<20;++i)CHECK(SendMessageW(moved,LB_ADDSTRING,0,(LPARAM)L"Item")!=LB_ERR);ShowWindow(moved,SW_SHOWNA);CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(moved));CHECK(SetParent(moved,root));CHECK(RedrawWindow(moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(moved));CHECK(DestroyWindow(moved));
    HWND wrong_grid=CreateWindowW(L"TValueListEditor",L"",WS_CHILD|WS_VISIBLE,0,0,180,100,root,NULL,self,NULL);CHECK(wrong_grid&&RedrawWindow(wrong_grid,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(wrong_grid));CHECK(DestroyWindow(wrong_grid));
    HWND value_grid=CreateWindowExW(WS_EX_CLIENTEDGE,L"TValueListEditor",L"",WS_CHILD|WS_VISIBLE,0,0,180,100,root,NULL,self,NULL);CHECK(value_grid&&RedrawWindow(value_grid,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(value_grid));
    SCROLLINFO grid_scroll={.cbSize=sizeof(grid_scroll),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=0};SetScrollInfo(value_grid,SB_VERT,&grid_scroll,TRUE);CHECK(ShowScrollBar(value_grid,SB_VERT,TRUE));CHECK(RedrawWindow(value_grid,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(value_grid));CHECK(GetWindowLongPtrW(value_grid,GWL_STYLE)&WS_VSCROLL);
    CHECK(ShowScrollBar(value_grid,SB_VERT,FALSE));CHECK(RedrawWindow(value_grid,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(value_grid));CHECK(!(GetWindowLongPtrW(value_grid,GWL_STYLE)&WS_VSCROLL));CHECK(compare(value_grid,remove)==0);CHECK(DestroyWindow(value_grid));
    HWND moved_grid=CreateWindowExW(WS_EX_CLIENTEDGE,L"TValueListEditor",L"",WS_CHILD|WS_VISIBLE,0,0,180,100,root,NULL,self,NULL);CHECK(moved_grid&&RedrawWindow(moved_grid,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(moved_grid));CHECK(SetParent(moved_grid,grid)==root);CHECK(RedrawWindow(moved_grid,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(moved_grid));CHECK(DestroyWindow(moved_grid));
    HWND db_no_edge=CreateWindowW(L"TDBGrid",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,self,NULL);CHECK(db_no_edge&&RedrawWindow(db_no_edge,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(db_no_edge));CHECK(DestroyWindow(db_no_edge));
    HWND db_no_scroll=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBGrid",L"",WS_CHILD|WS_VISIBLE,0,0,180,100,root,NULL,self,NULL);CHECK(db_no_scroll&&RedrawWindow(db_no_scroll,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(db_no_scroll));CHECK(DestroyWindow(db_no_scroll));
    HWND db_grid=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBGrid",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,self,NULL);CHECK(db_grid);SetScrollInfo(db_grid,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(db_grid,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(db_grid));CHECK(compare(db_grid,remove)==0);CHECK(DestroyWindow(db_grid));
    HWND db_hidden=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBGrid",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,self,NULL);CHECK(db_hidden);SetScrollInfo(db_hidden,SB_VERT,&grid_scroll,TRUE);CHECK(ShowScrollBar(db_hidden,SB_VERT,FALSE));CHECK(!(GetWindowLongPtrW(db_hidden,GWL_STYLE)&WS_VSCROLL));SCROLLINFO hidden_info={.cbSize=sizeof(hidden_info),.fMask=SIF_ALL};CHECK(GetScrollInfo(db_hidden,SB_VERT,&hidden_info));CHECK(RedrawWindow(db_hidden,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(db_hidden));CHECK(ShowScrollBar(db_hidden,SB_VERT,TRUE));CHECK(RedrawWindow(db_hidden,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(db_hidden));CHECK(ShowScrollBar(db_hidden,SB_VERT,FALSE));CHECK(RedrawWindow(db_hidden,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(db_hidden));CHECK(compare(db_hidden,remove)==0);CHECK(DestroyWindow(db_hidden));
    HWND db_moved=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBGrid",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,self,NULL);CHECK(db_moved);SetScrollInfo(db_moved,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(db_moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(db_moved));CHECK(SetParent(db_moved,grid)==root);CHECK(RedrawWindow(db_moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(db_moved));CHECK(DestroyWindow(db_moved));
    HWND lookup_host=CreateWindowW(L"LookupListHost",L"",WS_CHILD|WS_VISIBLE,0,0,200,120,root,NULL,self,NULL);CHECK(lookup_host);
    HWND lookup_wrong=CreateWindowExW(WS_EX_CLIENTEDGE,TEST_LOOKUP_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,root,NULL,self,NULL);CHECK(lookup_wrong);SetScrollInfo(lookup_wrong,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(lookup_wrong,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(lookup_wrong));CHECK(DestroyWindow(lookup_wrong));
    HWND lookup_no_edge=CreateWindowW(TEST_LOOKUP_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,lookup_host,NULL,self,NULL);CHECK(lookup_no_edge);SetScrollInfo(lookup_no_edge,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(lookup_no_edge,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(lookup_no_edge));CHECK(DestroyWindow(lookup_no_edge));
    HWND lookup_no_scroll=CreateWindowExW(WS_EX_CLIENTEDGE,TEST_LOOKUP_CLASS,L"",WS_CHILD|WS_VISIBLE,0,0,180,100,lookup_host,NULL,self,NULL);CHECK(lookup_no_scroll&&RedrawWindow(lookup_no_scroll,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(lookup_no_scroll));CHECK(DestroyWindow(lookup_no_scroll));
    HWND lookup_list=CreateWindowExW(WS_EX_CLIENTEDGE,TEST_LOOKUP_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,lookup_host,NULL,self,NULL);CHECK(lookup_list);SetScrollInfo(lookup_list,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(lookup_list,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW)&&attached(lookup_list));CHECK(SendMessageW(lookup_list,WM_APP+100,37,0)==0 && SendMessageW(lookup_list,WM_APP+101,0,0)==37);CHECK(compare(lookup_list,remove)==0);CHECK(SendMessageW(lookup_list,WM_APP+101,0,0)==37);CHECK(DestroyWindow(lookup_list));
    HWND lookup_moved=CreateWindowExW(WS_EX_CLIENTEDGE,TEST_LOOKUP_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,180,100,lookup_host,NULL,self,NULL);CHECK(lookup_moved);SetScrollInfo(lookup_moved,SB_VERT,&grid_scroll,TRUE);CHECK(RedrawWindow(lookup_moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(lookup_moved));CHECK(SetParent(lookup_moved,root)==lookup_host);CHECK(RedrawWindow(lookup_moved,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&!attached(lookup_moved));CHECK(DestroyWindow(lookup_moved)&&DestroyWindow(lookup_host));
    CHECK(DestroyWindow(edit)&&DestroyWindow(grid)&&DestroyWindow(root));CHECK(UnregisterClassW(L"LookupListHost",self)&&UnregisterClassW(TEST_LOOKUP_CLASS,self)&&UnregisterClassW(L"TDBGrid",self)&&UnregisterClassW(L"TPopupListbox",self)&&UnregisterClassW(L"TInplaceEditList",self)&&UnregisterClassW(L"TValueListEditor",self));
    puts("POPUP_MODEL_OK owner=1 ownerdraw=1 witness=1 selection=1 scroll=1 short=1 transition=1 pixels=1 font=1 reparent=1 valuegrid=1 gridreparent=1 dbgrid=1 dbhidden=1 lookup_list=1 lookup_content=1 cleanup=1");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
