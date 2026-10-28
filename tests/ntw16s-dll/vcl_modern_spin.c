#define UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <setjmp.h>
#include "ntw16s-dll/injection/unwind.h"
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SPIN_MODEL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)

static BOOL exception_seen;
#ifndef _WIN64
typedef struct catcher {ntw16s_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch_exception(EXCEPTION_RECORD *e,void *frame,CONTEXT *c,void *dispatcher)
{(void)c;(void)dispatcher;if(!(e->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && e->ExceptionCode==0xe0425342u){exception_seen=TRUE;RtlUnwind(frame,NULL,e,NULL);longjmp(((catcher*)frame)->saved,1);}return ExceptionContinueSearch;}
#else
static LONG __attribute__((used)) spin_filter(EXCEPTION_POINTERS *e,void *frame)
{(void)frame;if(e->ExceptionRecord->ExceptionCode==0xe0425342u){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}return EXCEPTION_CONTINUE_SEARCH;}
#endif
static NTW16S_UNWIND_FUNCTION BOOL test_unwind(HWND w,HDC dc)
{exception_seen=FALSE;
#ifndef _WIN64
 catcher c;c.record.handler=catch_exception;__asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");__asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");if(!setjmp(c.saved))SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);__asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
 __asm__ __volatile__(".Lspin_start:\n.seh_handler __C_specific_handler, @except\n.seh_handlerdata\n.long 1\n.rva .Lspin_start,.Lspin_end,spin_filter,.Lspin_end\n.text\n":::"memory");SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);__asm__ __volatile__("nop\n.Lspin_end: nop\n":::"memory");
#endif
 return exception_seen;}

static int worker(int argc,WCHAR **argv)
{
 CHECK(argc==2);HMODULE module=LoadLibraryW(argv[1]);CHECK(module);
 BOOL (WINAPI *reg)(int)=(void*)GetProcAddress(module,"FixtureRegister"), (WINAPI *unreg)(void)=(void*)GetProcAddress(module,"FixtureUnregister");
 HWND (WINAPI *create)(HWND,BOOL)=(void*)GetProcAddress(module,"FixtureCreate"), (WINAPI *button)(HWND)=(void*)GetProcAddress(module,"FixtureAddButton");
 void (WINAPI *mode)(int)=(void*)GetProcAddress(module,"SpinMode");COLORREF (WINAPI *pixel)(void)=(void*)GetProcAddress(module,"SpinLastPixel");CHECK(reg&&unreg&&create&&button&&mode&&pixel);
 HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
 BOOL (__cdecl *attached)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),(__cdecl *remove)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove);
 HWND root=CreateWindowW(L"STATIC",L"model",WS_OVERLAPPEDWINDOW,20,20,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(reg(0));
 HWND parent=create(root,FALSE);CHECK(parent);CHECK(RedrawWindow(parent,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(parent));
 HWND child=button(parent);CHECK(child);mode(1);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(child));
 HDC dc=GetDC(child);CHECK(dc);COLORREF styled=GetPixel(dc,3,3),glyph=GetPixel(dc,10,6);for(int gy=0;gy<3;gy++)for(int gx=0;gx<3;gx++)CHECK(GetPixel(dc,10+gx,6+gy)==RGB(220-40*gx,40*gy,20+30*gx));ReleaseDC(child,dc);CHECK(glyph==RGB(220,0,20));puts("SPIN_COLOR_GLYPH pixels=9 unchanged=9");
 HDC screen=GetDC(child),clipdc=CreateCompatibleDC(screen);CHECK(screen&&clipdc);HBITMAP clipbmp=CreateCompatibleBitmap(screen,20,24);CHECK(clipbmp);HGDIOBJ clipold=SelectObject(clipdc,clipbmp);CHECK(clipold);RECT all={0,0,20,24};HBRUSH sentinel=CreateSolidBrush(RGB(80,40,120));CHECK(sentinel);FillRect(clipdc,&all,sentinel);DeleteObject(sentinel);mode(8);CHECK(test_unwind(child,clipdc));CHECK(attached(child));CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));puts("SPIN_UNWIND caught=1 later_paint=1");int clip_save=SaveDC(clipdc);CHECK(clip_save);CHECK(IntersectClipRect(clipdc,0,0,10,10)!=ERROR);SendMessageW(child,WM_PRINTCLIENT,(WPARAM)clipdc,PRF_CLIENT);CHECK(RestoreDC(clipdc,clip_save));COLORREF clipped_inside=GetPixel(clipdc,3,3),clipped_outside=GetPixel(clipdc,15,20);printf("SPIN_CLIP inside=%08lx outside=%08lx\n",(DWORD)clipped_inside,(DWORD)clipped_outside);CHECK(clipped_inside==RGB(192,192,192));CHECK(clipped_outside==RGB(80,40,120));SelectObject(clipdc,clipold);CHECK(DeleteObject(clipbmp));CHECK(DeleteDC(clipdc));ReleaseDC(child,screen);
 mode(7);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF swapped=GetPixel(dc,3,3);ReleaseDC(child,dc);printf("SPIN_SWAP sampled=%08lx expected=%08lx\n",(DWORD)swapped,(DWORD)RGB(253,253,253));CHECK(swapped==RGB(253,253,253));mode(9);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF swapped_back=GetPixel(dc,3,3);ReleaseDC(child,dc);printf("SPIN_SWAP_BACK sampled=%08lx expected=%08lx\n",(DWORD)swapped_back,(DWORD)RGB(253,253,253));CHECK(swapped_back==RGB(253,253,253));

 mode(11);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF different_theme=GetPixel(dc,3,3);ReleaseDC(child,dc);printf("SPIN_OTHER_THEME sampled=%08lx expected=%08lx\n",(DWORD)different_theme,(DWORD)RGB(253,253,253));CHECK(different_theme==RGB(253,253,253));
 mode(12);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF failed_theme=GetPixel(dc,3,3);ReleaseDC(child,dc);printf("SPIN_FAILED_THEME sampled=%08lx expected=%08lx\n",(DWORD)failed_theme,(DWORD)RGB(253,253,253));CHECK(failed_theme==RGB(253,253,253));
 mode(13);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF source_outside=GetPixel(dc,15,3);ReleaseDC(child,dc);printf("SPIN_SOURCE_CLIP sampled=%08lx expected=%08lx\n",(DWORD)source_outside,(DWORD)RGB(253,253,253));CHECK(source_outside==RGB(253,253,253));
 mode(14);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF source_hole=GetPixel(dc,15,3);ReleaseDC(child,dc);printf("SPIN_SOURCE_HOLE sampled=%08lx expected=%08lx\n",(DWORD)source_hole,(DWORD)RGB(253,253,253));CHECK(source_hole==RGB(253,253,253));
 mode(2);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));mode(3);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));COLORREF retained=pixel();
 mode(5);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));COLORREF foreign_styled=pixel();
 CHECK(remove(child));mode(5);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));COLORREF foreign_native=pixel();mode(0);CHECK(RedrawWindow(child,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));dc=GetDC(child);CHECK(dc);COLORREF native=GetPixel(dc,3,3),native_retained=GetPixel(dc,2,2);ReleaseDC(child,dc);
 printf("SPIN_MODEL bits=%u styled=%08lx native=%08lx retained=%08lx glyph=%08lx\n",(unsigned)(sizeof(void*)*8),(DWORD)styled,(DWORD)native,(DWORD)retained,(DWORD)glyph);
 printf("SPIN_SAME_OWNER_IMAGE styled=%08lx native=%08lx changed=%d\n",(DWORD)foreign_styled,(DWORD)foreign_native,foreign_styled!=foreign_native);
 CHECK(styled!=native);CHECK(retained==native_retained);CHECK(foreign_styled==foreign_native);CHECK(DestroyWindow(child));CHECK(DestroyWindow(parent));CHECK(DestroyWindow(root));CHECK(unreg());CHECK(FreeLibrary(module));puts("SPIN_MODEL_PASS nested=1 retained=1 glyph=1 detach=1 cleanup=1");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
