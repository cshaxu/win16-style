#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "isolated_process.h"
#include "ntw16s-dll/injection/unwind.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_COLOR_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
static HWND observed;
static window_fn remove_style;
static BOOL retire,destroy_item,removed,destroyed,nest_item,nested_done;
static BOOL raise_item,exception_seen;
static unsigned draws,commands;
#define COLOR_TEST_EXCEPTION 0xe0425339u
#ifndef _WIN64
typedef struct color_catcher {ntw16s_unwind_record record;jmp_buf saved;} color_catcher;
static EXCEPTION_DISPOSITION NTAPI color_catch(EXCEPTION_RECORD *exception,void *frame,CONTEXT *context,void *dispatcher)
{
    (void)context;(void)dispatcher;
    if(!(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && exception->ExceptionCode==COLOR_TEST_EXCEPTION){
        exception_seen=TRUE;RtlUnwind(frame,NULL,exception,NULL);longjmp(((color_catcher *)frame)->saved,1);
    }
    return ExceptionContinueSearch;
}
#else
static LONG __attribute__((used)) color_filter(EXCEPTION_POINTERS *exception,void *frame)
{
    (void)frame;
    if(exception->ExceptionRecord->ExceptionCode==COLOR_TEST_EXCEPTION){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_MEASUREITEM && l && ((MEASUREITEMSTRUCT*)l)->CtlType==ODT_COMBOBOX){((MEASUREITEMSTRUCT*)l)->itemHeight=18;return TRUE;}
    if(m==WM_DRAWITEM && l){DRAWITEMSTRUCT *d=(void*)l;
        if(d->CtlType==ODT_COMBOBOX && d->hwndItem==observed){
            if(raise_item){raise_item=FALSE;RaiseException(COLOR_TEST_EXCEPTION,0,0,NULL);}
            ++draws;if(retire){retire=FALSE;removed=remove_style(observed);}
            if(destroy_item){destroy_item=FALSE;destroyed=DestroyWindow(observed);return TRUE;}
            if(nest_item && !nested_done){
                nested_done=TRUE;HDC dc=CreateCompatibleDC(d->hDC);HBITMAP bmp=CreateCompatibleBitmap(d->hDC,220,64);
                HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
                if(old){SendMessageW(observed,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);SelectObject(dc,old);}
                if(bmp)DeleteObject(bmp);
                if(dc)DeleteDC(dc);
            }
            int saved=SaveDC(d->hDC);if(saved){SetDCBrushColor(d->hDC,RGB(40,180,90));FillRect(d->hDC,&d->rcItem,(HBRUSH)GetStockObject(DC_BRUSH));RestoreDC(d->hDC,saved);}
            return TRUE;
        }
    }
    if(m==WM_COMMAND && (HWND)l==observed)++commands;
    return DefWindowProcW(w,m,p,l);
}
static NTW16S_UNWIND_FUNCTION BOOL test_unwind(HWND w,HDC dc)
{
    exception_seen=FALSE;raise_item=TRUE;
#ifndef _WIN64
    color_catcher c;c.record.handler=color_catch;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Ls39_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Ls39_catch_start,.Ls39_catch_end,color_filter,.Ls39_catch_end\n.text\n":::"memory");
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
    __asm__ __volatile__("nop\n.Ls39_catch_end: nop\n":::"memory");
#endif
    raise_item=FALSE;return exception_seen;
}
static BOOL capture(HWND w,DWORD *pixels,int width,int height,const RECT *clip,RECT *returned_clip)
{
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO bi={0};void *bits=NULL;
    bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bmp=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,&bits,NULL,0);HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
    if(old){memset(bits,0x5a,(size_t)width*height*4);if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
        SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
        if(returned_clip)GetClipBox(dc,returned_clip);
        memcpy(pixels,bits,(size_t)width*height*4);SelectObject(dc,old);}
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    return old!=NULL;
}
static BOOL print_resources(HWND w)
{
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);HBITMAP bmp=CreateCompatibleBitmap(screen,220,64);
    HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;DWORD before=0,after=0;
    if(old){for(unsigned i=0;i<1024;++i){
        SelectClipRgn(dc,NULL);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        if(i==959){GdiFlush();before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);}
    }GdiFlush();after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);SelectObject(dc,old);}
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    printf("COLOR_GDI repeated_print_before=%lu after=%lu\n",before,after);
    return before && after<=before+2;
}
static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
    reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
    unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg && create && unreg);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove_style);
    HWND startup=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(startup);CHECK(DestroyWindow(startup));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclModuleColorParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Module Color",WS_OVERLAPPEDWINDOW,20,20,400,240,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    for(unsigned mode=0;mode<2;++mode){
        CHECK(reg(mode==0));observed=create(root,FALSE);CHECK(observed && IsWindowVisible(observed));
        CHECK(SendMessageW(observed,CB_ADDSTRING,0,(LPARAM)L"Green")!=CB_ERR);
        CHECK(SendMessageW(observed,CB_ADDSTRING,0,(LPARAM)L"Blue")!=CB_ERR);CHECK(SendMessageW(observed,CB_SETCURSEL,0,0)==0);
        CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(!mode)CHECK(!attached(observed));
        else {
            CHECK(attached(observed));CHECK(SendMessageW(observed,CB_GETCOUNT,0,0)==2);
            COMBOBOXINFO info={.cbSize=sizeof(info)};RECT client;CHECK(GetComboBoxInfo(observed,&info) && GetClientRect(observed,&client));
            printf("COLOR_GEOMETRY dpi=%u size=%ld,%ld button=%ld,%ld,%ld,%ld item=%ld,%ld,%ld,%ld\n",GetDpiForWindow(observed),client.right,client.bottom,info.rcButton.left,info.rcButton.top,info.rcButton.right,info.rcButton.bottom,info.rcItem.left,info.rcItem.top,info.rcItem.right,info.rcItem.bottom);
            CHECK(client.right>0 && client.right<=220 && client.bottom>0 && client.bottom<=64);
            int width=client.right,height=client.bottom;DWORD styled[220*64],native[220*64],clipped[220*64];RECT styled_clip,native_clip;
            draws=commands=0;CHECK(capture(observed,styled,width,height,NULL,&styled_clip));CHECK(draws);
            nest_item=TRUE;nested_done=FALSE;CHECK(capture(observed,clipped,width,height,NULL,NULL));nest_item=FALSE;CHECK(nested_done);
            CHECK(print_resources(observed));
            HDC screen=GetDC(observed),memory=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,width,height);
            HGDIOBJ prior=memory && bitmap?SelectObject(memory,bitmap):NULL;CHECK(prior);
            DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(test_unwind(observed,memory));
            GdiFlush();DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(gdi_after==gdi_before);
            CHECK(attached(observed));SelectObject(memory,prior);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(memory));CHECK(ReleaseDC(observed,screen));
            CHECK(capture(observed,clipped,width,height,&info.rcButton,NULL));unsigned inside=0,outside=0;
            for(int y=0;y<height;++y)for(int x=0;x<width;++x)if((clipped[y*width+x]&0xffffff)!=0x5a5a5a){
                if(x>=info.rcButton.left && x<info.rcButton.right && y>=info.rcButton.top && y<info.rcButton.bottom)++inside;else ++outside;
            }
            CHECK(inside && !outside);CHECK(SendMessageW(observed,CB_SHOWDROPDOWN,TRUE,0));CHECK(SendMessageW(observed,CB_GETDROPPEDSTATE,0,0));
            CHECK(SendMessageW(observed,CB_SHOWDROPDOWN,FALSE,0));CHECK(SendMessageW(observed,CB_GETCURSEL,0,0)==0);
            CHECK(SendMessageW(observed,CB_SETCURSEL,1,0)==1);CHECK(SendMessageW(observed,CB_GETCURSEL,0,0)==1);
            CHECK(SendMessageW(observed,CB_SETCURSEL,0,0)==0);commands=0;
            SendMessageW(observed,WM_KEYDOWN,VK_DOWN,0);SendMessageW(observed,WM_KEYUP,VK_DOWN,0);
            CHECK(SendMessageW(observed,CB_GETCURSEL,0,0)==1);unsigned styled_commands=commands;
            CHECK(SendMessageW(observed,CB_SETCURSEL,0,0)==0);CHECK(remove_style(observed));
            commands=0;SendMessageW(observed,WM_KEYDOWN,VK_DOWN,0);SendMessageW(observed,WM_KEYUP,VK_DOWN,0);
            CHECK(SendMessageW(observed,CB_GETCURSEL,0,0)==1 && commands==styled_commands);
            CHECK(SendMessageW(observed,CB_SETCURSEL,0,0)==0);CHECK(capture(observed,native,width,height,NULL,&native_clip));
            CHECK(EqualRect(&styled_clip,&native_clip));
            unsigned changed=0,beyond=0;
            for(int y=0;y<height;++y)for(int x=0;x<width;++x)if((styled[y*width+x]&0xffffff)!=(native[y*width+x]&0xffffff)){
                ++changed;if(x<info.rcButton.left || x>=info.rcButton.right || y<info.rcButton.top || y>=info.rcButton.bottom)++beyond;
            }
            printf("COLOR_PIXELS changed=%u outside_button=%u clip_outside=%u\n",changed,beyond,outside);CHECK(changed && !beyond);
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);SendMessageW(observed,CB_ADDSTRING,0,(LPARAM)L"Green");SendMessageW(observed,CB_SETCURSEL,0,0);
            CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            retire=TRUE;removed=FALSE;RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);CHECK(removed && !attached(observed));
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);SendMessageW(observed,CB_ADDSTRING,0,(LPARAM)L"Green");SendMessageW(observed,CB_SETCURSEL,0,0);
            CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            destroy_item=TRUE;destroyed=FALSE;RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);CHECK(destroyed && !IsWindow(observed));observed=NULL;
        }
        if(observed)CHECK(DestroyWindow(observed));
        observed=NULL;CHECK(unreg());
    }
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    printf("VCL_COLOR_MODULE_OK bits=%u fake=1 selection=1 keyboard=1 popup=1 ownerdraw=1 nested=1 pixels=1 clip=1 dc_state=1 print_resources=1 unwind=1 retire=1 destroy=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
