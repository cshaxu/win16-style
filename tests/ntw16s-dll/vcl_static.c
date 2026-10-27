#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <setjmp.h>
#include "isolated_process.h"
#include "ntw16s-dll/injection/unwind.h"

#define CHECK(x) do { if(!(x)){fprintf(stderr,"VCL_STATIC_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)
typedef BOOL (__cdecl *attached_fn)(HWND);
static WNDPROC native_static;
static unsigned color_queries;
static HWND observed_child;
static HFONT expected_font;
static HFONT requested_font;
static BOOL wrong_font;
static BOOL change_font;
static HWND nested_target;
static BOOL trigger_nested,nested_ok;
static BOOL raise_in_color,exception_seen;
#define STATIC_TEST_EXCEPTION 0xe0425331u
#ifndef _WIN64
typedef struct static_catcher {ntw16s_unwind_record record;jmp_buf saved;} static_catcher;
static EXCEPTION_DISPOSITION NTAPI static_catch(EXCEPTION_RECORD *exception,void *frame,CONTEXT *context,void *dispatcher)
{
    (void)context;(void)dispatcher;
    if(!(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && exception->ExceptionCode==STATIC_TEST_EXCEPTION){
        exception_seen=TRUE;RtlUnwind(frame,NULL,exception,NULL);longjmp(((static_catcher *)frame)->saved,1);
    }
    return ExceptionContinueSearch;
}
#else
static LONG __attribute__((used)) static_filter(EXCEPTION_POINTERS *exception,void *frame)
{
    (void)frame;
    if(exception->ExceptionRecord->ExceptionCode==STATIC_TEST_EXCEPTION){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif
static LRESULT CALLBACK parent_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_CTLCOLORSTATIC && (HWND)lp==observed_child){
        ++color_queries;
        if(raise_in_color){raise_in_color=FALSE;RaiseException(STATIC_TEST_EXCEPTION,0,0,NULL);}
        if((HFONT)SendMessageW(observed_child,WM_GETFONT,0,0)!=expected_font)wrong_font=TRUE;
        if(change_font){change_font=FALSE;expected_font=requested_font;SendMessageW(observed_child,WM_SETFONT,(WPARAM)requested_font,FALSE);}
        if(trigger_nested){trigger_nested=FALSE;nested_ok=RedrawWindow(nested_target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);}
    }
    return DefWindowProcW(w,message,wp,lp);
}
static LRESULT CALLBACK framework_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{return CallWindowProcW(native_static,w,message,wp,lp);}
static LRESULT CALLBACK initializer_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{if(message==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)framework_proc);return framework_proc(w,message,wp,lp);}
static LRESULT CALLBACK fake_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{if(message==WM_GETDLGCODE)return DLGC_STATIC;return DefWindowProcW(w,message,wp,lp);}
static BOOL register_wrapper(const WCHAR *name,WNDPROC proc)
{
    WNDCLASSEXW native={.cbSize=sizeof(native)},cls={.cbSize=sizeof(cls)};
    if(!GetClassInfoExW(NULL,L"STATIC",&native))return FALSE;
    native_static=native.lpfnWndProc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=name;cls.lpfnWndProc=proc;
    cls.style=CS_DBLCLKS|CS_HREDRAW;cls.cbWndExtra=native.cbWndExtra;cls.hCursor=LoadCursorW(NULL,IDC_ARROW);
    return RegisterClassExW(&cls)!=0;
}
static BOOL snapshot(HWND w,DWORD *pixels,unsigned width,unsigned height,BOOL print)
{
    HDC source=GetDC(w),memory=source?CreateCompatibleDC(source):NULL;void *bits=NULL;BITMAPINFO info={0};
    info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=(LONG)width;
    info.bmiHeader.biHeight=-(LONG)height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=memory?CreateDIBSection(memory,&info,DIB_RGB_COLORS,&bits,NULL,0):NULL;BOOL ok=FALSE;
    if(bitmap && bits){HGDIOBJ old=SelectObject(memory,bitmap);RECT r={0,0,(LONG)width,(LONG)height};
        if(!old || old==HGDI_ERROR)fprintf(stderr,"VCL_STATIC_SELECT_FAIL bitmap=%p memory=%p error=%lu\n",bitmap,memory,GetLastError());
        FillRect(memory,&r,GetStockObject(WHITE_BRUSH));
        if(print){DWORD_PTR result=0;ok=SendMessageTimeoutW(w,WM_PRINTCLIENT,(WPARAM)memory,PRF_CLIENT,SMTO_ABORTIFHUNG,3000,&result)!=0;}
        else {ok=BitBlt(memory,0,0,(int)width,(int)height,source,0,0,SRCCOPY);if(!ok){
            DWORD error=GetLastError();HWND parent=GetParent(w);HDC pdc=GetDC(parent),sdc=GetDC(NULL);POINT origin={0,0};
            MapWindowPoints(w,parent,&origin,1);BOOL parent_ok=pdc && BitBlt(memory,0,0,(int)width,(int)height,pdc,origin.x,origin.y,SRCCOPY);DWORD parent_error=GetLastError();
            BOOL screen_ok=sdc && BitBlt(memory,0,0,(int)width,(int)height,sdc,0,0,SRCCOPY);DWORD screen_error=GetLastError();
            HDC other=CreateCompatibleDC(source);HBITMAP other_bitmap=other?CreateCompatibleBitmap(source,2,2):NULL;HGDIOBJ other_old=other_bitmap?SelectObject(other,other_bitmap):NULL;
            BOOL memory_ok=other_old && BitBlt(memory,0,0,2,2,other,0,0,SRCCOPY);DWORD memory_error=GetLastError();
            fprintf(stderr,"VCL_STATIC_BITBLT_FAIL dc=%p memory=%p error=%lu source_type=%lu memory_type=%lu parent=%u:%lu screen=%u:%lu memory=%u:%lu caps=%d,%d,%d bitmap=%p\n",source,memory,error,GetObjectType(source),GetObjectType(memory),parent_ok,parent_error,screen_ok,screen_error,memory_ok,memory_error,GetDeviceCaps(source,HORZRES),GetDeviceCaps(source,VERTRES),GetDeviceCaps(source,TECHNOLOGY),bitmap);
            if(other_old)SelectObject(other,other_old);
            if(other_bitmap)DeleteObject(other_bitmap);
            if(other)DeleteDC(other);
            if(pdc)ReleaseDC(parent,pdc);
            if(sdc)ReleaseDC(NULL,sdc);
        }}
        if(ok){GdiFlush();CopyMemory(pixels,bits,width*height*sizeof(*pixels));}
        SelectObject(memory,old);
    }
    if(!source || !memory || !bitmap)fprintf(stderr,"VCL_STATIC_CAPTURE_SETUP_FAIL source=%p memory=%p bitmap=%p error=%lu\n",source,memory,bitmap,GetLastError());
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(source)ReleaseDC(w,source);
    return ok;
}
static NTW16S_UNWIND_FUNCTION BOOL test_unwind(HWND w)
{
    exception_seen=FALSE;raise_in_color=TRUE;
#ifndef _WIN64
    static_catcher c;c.record.handler=static_catch;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Ls31_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Ls31_catch_start,.Ls31_catch_end,static_filter,.Ls31_catch_end\n.text\n":::"memory");
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
    __asm__ __volatile__("nop\n.Ls31_catch_end: nop\n":::"memory");
#endif
    raise_in_color=FALSE;return exception_seen;
}
static int worker(int argc,WCHAR **argv)
{
    BOOL bare=argc>1 && !wcscmp(argv[1],L"bare"),exception_case=argc>1 && !wcscmp(argv[1],L"exception");HINSTANCE self=GetModuleHandleW(NULL);
    WNDCLASSW parent={.hInstance=self,.lpfnWndProc=parent_proc,.lpszClassName=L"S31StaticParent",.hbrBackground=(HBRUSH)(COLOR_WINDOW+1)};
    CHECK(RegisterClassW(&parent));CHECK(register_wrapper(L"TStaticText",initializer_proc));
    HWND root=CreateWindowW(parent.lpszClassName,L"static test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,30,30,400,180,NULL,NULL,self,NULL);CHECK(root);
    ShowWindow(root,SW_SHOW);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    HWND text=CreateWindowW(L"TStaticText",L"Native &text",WS_CHILD|WS_VISIBLE|SS_LEFT,20,20,180,30,root,NULL,self,NULL);CHECK(text);
    HWND reference=CreateWindowW(L"TStaticText",L"Native &text",WS_CHILD|WS_VISIBLE|SS_LEFT,20,60,180,30,root,NULL,self,NULL);CHECK(reference);
    printf("VCL_STATIC_VISIBILITY root=%u child=%u iconic=%u\n",IsWindowVisible(root),IsWindowVisible(text),IsIconic(root));
    WCHAR initial_real[64]={0};RealGetWindowClassW(text,initial_real,64);printf("VCL_STATIC_INITIAL real=%ls\n",initial_real);
    HFONT font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Segoe UI");CHECK(font);
    SendMessageW(text,WM_SETFONT,(WPARAM)font,FALSE);SendMessageW(reference,WM_SETFONT,(WPARAM)font,FALSE);observed_child=text;expected_font=font;
    HMODULE module=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    attached_fn attached=module?(attached_fn)(void*)GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    attached_fn remove=module?(attached_fn)(void*)GetProcAddress(module,"Ntw16sWindowRemove"):NULL;
    WCHAR real[64]={0},registered[64]={0};RealGetWindowClassW(text,real,64);GetClassNameW(text,registered,64);
    printf("VCL_STATIC_MODEL class=%ls real=%ls attached=%u source=%p current=%p\n",registered,real,attached?attached(text):0,font,(void*)SendMessageW(text,WM_GETFONT,0,0));
    if(!bare)CHECK(attached && attached(root) && remove && remove(reference));
    CHECK(RedrawWindow(text,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    if(!bare)CHECK(attached(text));
    CHECK(RedrawWindow(reference,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    DWORD screen[180*30],printed[180*30],native_screen[180*30],native_print[180*30];
    CHECK(snapshot(text,screen,180,30,FALSE));CHECK(snapshot(text,printed,180,30,TRUE));
    CHECK(snapshot(reference,native_screen,180,30,FALSE));CHECK(snapshot(reference,native_print,180,30,TRUE));
    printf("VCL_STATIC_COLOR queries=%u wrong=%u\n",color_queries,wrong_font);
    CHECK(color_queries && !wrong_font && (HFONT)SendMessageW(text,WM_GETFONT,0,0)==font);
    unsigned differing=0,print_diff=0;for(unsigned i=0;i<ARRAYSIZE(screen);++i){
        if((screen[i]&0xffffff)!=(native_screen[i]&0xffffff))++differing;
        if((printed[i]&0xffffff)!=(native_print[i]&0xffffff))++print_diff;
    }
    printf("VCL_STATIC_PIXELS bare=%u screen_diff=%u print_diff=%u\n",bare,differing,print_diff);
    CHECK(print_diff==0);if(bare)CHECK(differing==0);else CHECK(differing>20);
    CHECK(!wrong_font && (HFONT)SendMessageW(text,WM_GETFONT,0,0)==font);
    EnableWindow(reference,FALSE);CHECK(RedrawWindow(reference,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    CHECK(snapshot(reference,native_screen,180,30,FALSE));nested_target=reference;trigger_nested=TRUE;
    CHECK(RedrawWindow(text,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));CHECK(!trigger_nested && nested_ok);
    CHECK(snapshot(reference,screen,180,30,FALSE));
    for(unsigned i=0;i<ARRAYSIZE(screen);++i)CHECK((screen[i]&0xffffff)==(native_screen[i]&0xffffff));
    requested_font=CreateFontW(-20,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(requested_font);
    change_font=TRUE;CHECK(RedrawWindow(text,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    CHECK(!change_font && !wrong_font && (HFONT)SendMessageW(text,WM_GETFONT,0,0)==requested_font);
    if(exception_case){CHECK(test_unwind(text));CHECK(RedrawWindow(text,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
        CHECK(attached(text) && !wrong_font && (HFONT)SendMessageW(text,WM_GETFONT,0,0)==requested_font);}
    CHECK(DestroyWindow(root));observed_child=NULL;CHECK(DeleteObject(font));CHECK(DeleteObject(requested_font));
    CHECK(UnregisterClassW(L"TStaticText",self));
    CHECK(register_wrapper(L"TStaticText",fake_proc));
    root=CreateWindowW(parent.lpszClassName,L"fake",WS_OVERLAPPEDWINDOW|WS_VISIBLE,30,30,400,180,NULL,NULL,self,NULL);CHECK(root);
    HWND fake=CreateWindowW(L"TStaticText",L"fake",WS_CHILD|WS_VISIBLE,20,20,180,30,root,NULL,self,NULL);CHECK(fake);
    CHECK(RedrawWindow(fake,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));
    if(!bare)CHECK(!attached(fake));
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(L"TStaticText",self));CHECK(UnregisterClassW(parent.lpszClassName,self));
    if(bare)printf("VCL_STATIC_BARE_OK bits=%u font_preserved=1\n",(unsigned)(sizeof(void*)*8));
    else printf("VCL_STATIC_OK bits=%u font_preserved=1 fake_rejected=1 nested_native=1\n",(unsigned)(sizeof(void*)*8));
    return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
