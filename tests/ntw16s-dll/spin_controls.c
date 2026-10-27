#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../../src/ntw16s-dll/attachment.h"
#include "isolated_process.h"

#define CHECK(x) do { if(!(x)){fprintf(stderr,"SPIN_CONTROL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)
static unsigned paint_count;
static BOOL native_result[2];
static RECT native_rect[2];
static DWORD native_error[2];
static BOOL (__cdecl *remove_during_paint_api)(HWND);
static BOOL remove_during_paint,destroy_during_paint;
static BOOL nested_probe,nested_seen;
static unsigned nested_mismatch;

/* Models the observed public GDI contract, not a Delphi runtime object. */
static LRESULT CALLBACK spin_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_APP+0x31 && p){
        HDC screen=(HDC)p,reference=CreateCompatibleDC(screen);RECT r;GetClientRect(w,&r);r.bottom=(r.bottom+1)/2;
        HBITMAP bitmap=reference?CreateCompatibleBitmap(screen,r.right,r.bottom):NULL;
        if(!bitmap){nested_mismatch=~0u;if(reference)DeleteDC(reference);return 0;}
        HGDIOBJ old=SelectObject(reference,bitmap);FillRect(screen,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
        FillRect(reference,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));RECT a=r,b=r;
        BOOL result=DrawFrameControl(screen,&a,DFC_BUTTON,DFCS_BUTTONPUSH|DFCS_ADJUSTRECT);
        BOOL baseline=DrawFrameControl(reference,&b,DFC_BUTTON,DFCS_BUTTONPUSH|DFCS_ADJUSTRECT);
        if(!result || !baseline || !EqualRect(&a,&b))++nested_mismatch;
        for(int y=0;y<r.bottom;y++)for(int x=0;x<r.right;x++)nested_mismatch+=GetPixel(screen,x,y)!=GetPixel(reference,x,y);
        SelectObject(reference,old);DeleteObject(bitmap);DeleteDC(reference);nested_seen=TRUE;return 0;
    }
    if(m==WM_PAINT){
        if(destroy_during_paint){destroy_during_paint=FALSE;DestroyWindow(w);return 0;}
        if(remove_during_paint){remove_during_paint=FALSE;remove_during_paint_api(w);}
        PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT client;GetClientRect(w,&client);
        if(nested_probe){nested_probe=FALSE;SendMessageW(w,WM_APP+0x31,(WPARAM)dc,0);}
        int half=(client.bottom+1)/2;
        for(int i=0;i<2;i++){
            int saved=SaveDC(dc);RECT r={0,0,client.right,i?client.bottom-half+1:half};
            if(i)SetWindowOrgEx(dc,0,-(half-1),NULL);
            IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
            SetLastError(12345);native_result[i]=DrawFrameControl(dc,&r,DFC_BUTTON,DFCS_BUTTONPUSH|DFCS_ADJUSTRECT);
            native_error[i]=GetLastError();native_rect[i]=r;
            /* Application-owned content is drawn after its frame. */
            SetPixel(dc,client.right/2,i?2:half/2,RGB(21,67,129));
            RestoreDC(dc,saved);
        }
        ++paint_count;EndPaint(w,&ps);return 0;
    }
    return DefWindowProcW(w,m,p,l);
}
static BOOL pixels(HWND w,DWORD *out,unsigned width,unsigned height)
{
    HDC source=GetDC(w),dc=source?CreateCompatibleDC(source):NULL;void *bits=NULL;
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;
    info.bmiHeader.biHeight=-(LONG)height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bm=dc?CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0):NULL;BOOL ok=FALSE;
    if(bm && bits){HGDIOBJ old=SelectObject(dc,bm);ok=BitBlt(dc,0,0,width,height,source,0,0,SRCCOPY);
        GdiFlush();if(ok)CopyMemory(out,bits,width*height*sizeof(DWORD));SelectObject(dc,old);}
    if(bm)DeleteObject(bm);
    if(dc)DeleteDC(dc);
    if(source)ReleaseDC(w,source);
    return ok;
}
static int passive(const WCHAR *path)
{
    HMODULE dll=LoadLibraryW(path);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void *)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope *,ntw16s_creation_result *)=(void *)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *attached)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(begin&&end&&attached);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW edit={0};CHECK(GetClassInfoW(NULL,L"EDIT",&edit));
    edit.hInstance=instance;edit.lpszClassName=L"TSpinEdit";CHECK(RegisterClassW(&edit));
    WNDCLASSW spin={.hInstance=instance,.lpfnWndProc=spin_proc,.lpszClassName=L"TSpinButton"};CHECK(RegisterClassW(&spin));
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,300,150,NULL,NULL,instance,NULL);CHECK(root);
    ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
    HWND parent=CreateWindowW(L"TSpinEdit",L"0",WS_CHILD|WS_VISIBLE,0,0,80,30,root,NULL,instance,NULL);CHECK(parent);
    HWND w=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,0,0,15,19,parent,NULL,instance,NULL);CHECK(w);
    ntw16s_creation_result result={0};CHECK(end(scope,&result) && !result.failure && !attached(w));
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(L"TSpinButton",instance));CHECK(UnregisterClassW(L"TSpinEdit",instance));
    CHECK(FreeLibrary(dll));printf("SPIN_CONTROL_PASSIVE_OK bits=%u unattached=1\n",(unsigned)sizeof(void*)*8);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"passive"))return passive(argv[2]);
    CHECK(argc==1 || (argc==2 && !wcscmp(argv[1],L"aware")));
    if(argc==2)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    BOOL (__cdecl *attached)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove_style);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW edit={0};CHECK(GetClassInfoW(NULL,L"EDIT",&edit));
    edit.hInstance=instance;edit.lpszClassName=L"TSpinEdit";CHECK(RegisterClassW(&edit));
    WNDCLASSW spin={.hInstance=instance,.lpfnWndProc=spin_proc,.lpszClassName=L"TSpinButton"};CHECK(RegisterClassW(&spin));
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,300,150,NULL,NULL,instance,NULL);CHECK(root);
    HWND parent=CreateWindowW(L"TSpinEdit",L"0",WS_CHILD|WS_VISIBLE|WS_BORDER,10,10,80,30,root,NULL,instance,NULL);CHECK(parent);
    UINT dpi=GetDpiForWindow(root);unsigned width=MulDiv(15,dpi,96),height=MulDiv(19,dpi,96);CHECK(width<=64 && height<=64);
    HWND w=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,3,3,width,height,parent,NULL,instance,NULL);CHECK(w);
    if(!attached(w)){fprintf(stderr,"SPIN_CONTROL_UNATTACHED bits=%u\n",(unsigned)sizeof(void*)*8);return 1;}
    ShowWindow(root,SW_SHOWNA);nested_probe=TRUE;CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    CHECK(nested_seen && !nested_mismatch);puts("SPIN_CONTROL_NESTED_OK unchanged=1");
    DWORD styled[64*64]={0},plain[64*64]={0};CHECK(pixels(w,styled,width,height));
    RECT expected[2]={native_rect[0],native_rect[1]};BOOL returns[2]={native_result[0],native_result[1]};
    DWORD errors[2]={native_error[0],native_error[1]};CHECK(paint_count && returns[0] && returns[1]);
    CHECK(remove_style(w) && !attached(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));CHECK(pixels(w,plain,width,height));
    CHECK(EqualRect(&expected[0],&native_rect[0]) && EqualRect(&expected[1],&native_rect[1]));
    CHECK(returns[0]==native_result[0] && returns[1]==native_result[1] && errors[0]==native_error[0] && errors[1]==native_error[1]);
    unsigned changed=0,outside=0;unsigned boundary=(height+1)/2-1;
    for(unsigned y=0;y<height;y++)for(unsigned x=0;x<width;x++)if((styled[y*width+x]&0xffffff)!=(plain[y*width+x]&0xffffff)){
        ++changed;if(x && x+1<width && y && y+1<height && y!=boundary)++outside;
    }
    CHECK(changed && !outside);printf("SPIN_CONTROL_OK bits=%u dpi=%u changed=%u outside=%u native_result_error_rect=preserved\n",(unsigned)sizeof(void*)*8,dpi,changed,outside);
    HWND clipped=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,width+5,3,width,height,parent,NULL,instance,NULL);CHECK(clipped&&attached(clipped));
    RECT full={0,0,(LONG)width,(LONG)height},partial={0,0,(LONG)((width+1)/2),(LONG)((height+1)/2)};
    HDC dc1=GetDC(w),dc2=GetDC(clipped);CHECK(dc1 && dc2);
    CHECK(FillRect(dc1,&full,(HBRUSH)GetStockObject(WHITE_BRUSH)) && FillRect(dc2,&full,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    ReleaseDC(w,dc1);ReleaseDC(clipped,dc2);
    CHECK(ValidateRect(w,NULL) && ValidateRect(clipped,NULL));
    CHECK(RedrawWindow(w,&partial,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
    CHECK(RedrawWindow(clipped,&partial,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
    CHECK(pixels(w,plain,width,height) && pixels(clipped,styled,width,height));
    unsigned clipped_changed=0,clipped_outside=0;
    for(unsigned y=0;y<height;y++)for(unsigned x=0;x<width;x++)if((styled[y*width+x]&0xffffff)!=(plain[y*width+x]&0xffffff)){
        ++clipped_changed;if(x>=(unsigned)partial.right || y>=(unsigned)partial.bottom)++clipped_outside;
    }
    printf("SPIN_CONTROL_CLIP_RESULT changed=%u outside=%u\n",clipped_changed,clipped_outside);
    CHECK(clipped_changed && !clipped_outside);CHECK(DestroyWindow(clipped));
    puts("SPIN_CONTROL_CLIP_OK");
    HWND reentrant=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,3,3,width,height,parent,NULL,instance,NULL);CHECK(reentrant&&attached(reentrant));
    remove_during_paint_api=remove_style;remove_during_paint=TRUE;
    CHECK(RedrawWindow(reentrant,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    CHECK(!remove_during_paint && !attached(reentrant));CHECK(pixels(reentrant,styled,width,height));
    CHECK(RedrawWindow(reentrant,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));CHECK(pixels(reentrant,plain,width,height));
    CHECK(!memcmp(styled,plain,width*height*sizeof(DWORD)));CHECK(DestroyWindow(reentrant));
    reentrant=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,3,3,width,height,parent,NULL,instance,NULL);CHECK(reentrant&&attached(reentrant));
    destroy_during_paint=TRUE;RedrawWindow(reentrant,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
    CHECK(!destroy_during_paint && !IsWindow(reentrant));
    puts("SPIN_CONTROL_REENTRANT_OK remove=1 destroy=1 native_after_remove=1");
    HWND other=CreateWindowW(L"EDIT",L"0",WS_CHILD|WS_VISIBLE,100,10,80,30,root,NULL,instance,NULL);CHECK(other);
    HWND fake=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,0,0,width,height,other,NULL,instance,NULL);CHECK(fake && !attached(fake));
    CHECK(DestroyWindow(fake));CHECK(DestroyWindow(w));CHECK(UnregisterClassW(L"TSpinButton",instance));
    spin.lpfnWndProc=DefWindowProcW;CHECK(RegisterClassW(&spin));
    fake=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,0,0,width,height,parent,NULL,instance,NULL);CHECK(fake && !attached(fake));
    CHECK(DestroyWindow(fake));CHECK(UnregisterClassW(L"TSpinButton",instance));
    WNDCLASSA ansi={.hInstance=instance,.lpfnWndProc=DefWindowProcA,.lpszClassName="TSpinButton"};CHECK(RegisterClassA(&ansi));
    fake=CreateWindowA("TSpinButton","",WS_CHILD|WS_VISIBLE,0,0,width,height,parent,NULL,instance,NULL);
    WNDCLASSEXA probe={.cbSize=sizeof(probe)};GetClassInfoExA(instance,"TSpinButton",&probe);
    printf("SPIN_ANSI_ID unicode=%u proc=%p default=%p attached=%u\n",IsWindowUnicode(fake),(void *)probe.lpfnWndProc,(void *)DefWindowProcA,attached(fake));
    CHECK(fake && !attached(fake));
    CHECK(DestroyWindow(root));CHECK(UnregisterClassA("TSpinButton",instance));CHECK(UnregisterClassW(L"TSpinEdit",instance));
    puts("SPIN_CONTROL_BOUNDARY_OK wrong_parent=1 defproc_w_a_impostor=1 cleanup=1");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
