#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
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
/* Return zero on any probe failure as well as a missing outer frame. */
#undef CHECK
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 0;}}while(0)
static int frame_pixels(HWND w,window_fn remove)
{
 RECT wr,cr;POINT pt={0};CHECK(GetWindowRect(w,&wr)&&GetClientRect(w,&cr)&&ClientToScreen(w,&pt));
 int width=wr.right-wr.left,height=wr.bottom-wr.top,x0=pt.x-wr.left,y0=pt.y-wr.top;
 HDC src=GetWindowDC(w),dc=CreateCompatibleDC(src);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;void *bits=NULL;
 HBITMAP bmp=CreateDIBSection(src,&bi,DIB_RGB_COLORS,&bits,NULL,0);CHECK(src&&dc&&bmp&&bits);HGDIOBJ old=SelectObject(dc,bmp);CHECK(old);DWORD *before=malloc((size_t)width*height*4);CHECK(before);
 CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,src,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*4);
 CHECK(remove(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,src,0,0,SRCCOPY));GdiFlush();
 unsigned edge=0,client=0,outer=0;DWORD *after=bits;for(int y=0;y<height;y++)for(int x=0;x<width;x++)if(before[y*width+x]!=after[y*width+x]){if(x>=x0&&x<x0+cr.right&&y>=y0&&y<y0+cr.bottom)client++;else {edge++;if(x<2||x>=width-2||y<2||y>=height-2)outer++;}}
 free(before);SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);ReleaseDC(w,src);printf("MEMO_FRAME_PIXELS edge=%u client=%u outer=%u\n",edge,client,outer);return outer>0;
}

#undef CHECK
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_EDIT_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int nc_region(HWND w){RECT wr;CHECK(GetWindowRect(w,&wr));int width=wr.right-wr.left;HDC dc=GetWindowDC(w);CHECK(dc);SetDCBrushColor(dc,RGB(255,0,255));RECT top={0,0,width,2};CHECK(FillRect(dc,&top,GetStockObject(DC_BRUSH)));GdiFlush();ReleaseDC(w,dc);HRGN r=CreateRectRgn(wr.left,wr.top,wr.left+20,wr.bottom);CHECK(r);SendMessageW(w,WM_NCPAINT,(WPARAM)r,0);GdiFlush();dc=GetWindowDC(w);CHECK(dc);unsigned inside=0,outside=0;for(int x=0;x<width;x++){COLORREF c=GetPixel(dc,x,0);if(c!=RGB(255,0,255)){if(x<20)inside++;else outside++;}}ReleaseDC(w,dc);DeleteObject(r);printf("NC_REGION inside=%u outside=%u width=%d\n",inside,outside,width);CHECK(inside==20 && outside==0);return 0;}
/* Mark the top edge, then check only the area left of the vertical scrollbar. */
static int noedge_witness(HWND w)
{
    RECT rect;CHECK(GetWindowRect(w,&rect));int width=rect.right-rect.left;CHECK(width>40);
    HDC dc=GetWindowDC(w);CHECK(dc);SetDCBrushColor(dc,RGB(255,0,255));RECT top={0,0,width,1};
    CHECK(FillRect(dc,&top,GetStockObject(DC_BRUSH)));GdiFlush();ReleaseDC(w,dc);
    SendMessageW(w,WM_NCPAINT,1,0);GdiFlush();dc=GetWindowDC(w);CHECK(dc);
    unsigned changed=0;for(int x=0;x<width-40;++x)if(GetPixel(dc,x,0)!=RGB(255,0,255))++changed;
    ReleaseDC(w,dc);printf("NOEDGE_WITNESS changed=%u checked=%d\n",changed,width-40);CHECK(changed==0);return 0;
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
    CHECK(reg(FALSE));HWND frame=create(root,TRUE);CHECK(frame && RedrawWindow(frame,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW) && attached(frame));CHECK(nc_region(frame)==0);CHECK(frame_pixels(frame,remove_style));CHECK(DestroyWindow(frame));CHECK(unreg());
    CHECK(reg(FALSE));frame=create(root,TRUE);CHECK(frame && RedrawWindow(frame,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW) && attached(frame));
    LONG_PTR ex=GetWindowLongPtrW(frame,GWL_EXSTYLE);SetWindowLongPtrW(frame,GWL_EXSTYLE,ex & ~WS_EX_CLIENTEDGE);
    CHECK(SetWindowPos(frame,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED));
    CHECK(noedge_witness(frame)==0);CHECK(DestroyWindow(frame));CHECK(unreg());
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    printf("VCL_EDIT_MODULE_OK bits=%u fake_paint_rejected=1 selection=1 readonly=1 multiline=1 scroll=1 reentrant_remove=1 frame_pixels=1 nc_clip=1 noedge=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
