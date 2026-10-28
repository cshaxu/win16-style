#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <richedit.h>
#include <commctrl.h>
#include <imm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "ntw16s-dll/injection/unwind.h"
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define CHECK(x) do {if(!(x)){fprintf(stderr,"BOX_MODEL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}} while(0)
typedef BOOL (WINAPI *regfn)(void);typedef HWND (WINAPI *parentfn)(HWND);typedef HWND (WINAPI *createfn)(HWND,BOOL);typedef BOOL (__cdecl *windowfn)(HWND);
static LRESULT WINAPI foreign_forward(HWND w,UINT msg,WPARAM wp,LPARAM lp){return DefWindowProcW(w,msg,wp,lp);}
static BOOL exception_seen;
#ifndef _WIN64
typedef struct catcher {ntw16s_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch_exception(EXCEPTION_RECORD *e,void *frame,CONTEXT *c,void *dispatcher)
{(void)c;(void)dispatcher;if(!(e->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && e->ExceptionCode==0xe0425342u){exception_seen=TRUE;RtlUnwind(frame,NULL,e,NULL);longjmp(((catcher*)frame)->saved,1);}return ExceptionContinueSearch;}
#else
static LONG __attribute__((used)) rich_filter(EXCEPTION_POINTERS *e,void *frame)
{(void)frame;if(e->ExceptionRecord->ExceptionCode==0xe0425342u){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}return EXCEPTION_CONTINUE_SEARCH;}
#endif
static NTW16S_UNWIND_FUNCTION BOOL test_unwind(HWND w,HDC dc)
{(void)dc;exception_seen=FALSE;
#ifndef _WIN64
 catcher c;c.record.handler=catch_exception;__asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");__asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");if(!setjmp(c.saved))SendMessageW(w,WM_NCPAINT,1,0);__asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
 __asm__ __volatile__(".Lrich_start:\n.seh_handler __C_specific_handler, @except\n.seh_handlerdata\n.long 1\n.rva .Lrich_start,.Lrich_end,rich_filter,.Lrich_end\n.text\n":::"memory");SendMessageW(w,WM_NCPAINT,1,0);__asm__ __volatile__("nop\n.Lrich_end: nop\n":::"memory");
#endif
 return exception_seen;}

static int compare_chrome(HWND w,windowfn remove)
{
 RECT outer,client;POINT origin={0};CHECK(GetWindowRect(w,&outer)&&GetClientRect(w,&client)&&ClientToScreen(w,&origin));int width=outer.right-outer.left,height=outer.bottom-outer.top;CHECK(width>0&&height>0&&width<1024&&height<1024);HDC window=GetWindowDC(w);CHECK(window);HDC dc=CreateCompatibleDC(window);CHECK(dc);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;DWORD *bits=NULL;HBITMAP bitmap=CreateDIBSection(window,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(bitmap&&bits);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);DWORD *before=malloc((size_t)width*height*sizeof(DWORD));CHECK(before);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*sizeof(DWORD));CHECK(remove(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();unsigned edge=0,content=0;int x0=origin.x-outer.left,y0=origin.y-outer.top;for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(before[y*width+x]!=bits[y*width+x]){if(x>=x0&&x<x0+client.right&&y>=y0&&y<y0+client.bottom)++content;else ++edge;}printf("BOX_PIXELS edge=%u client=%u\n",edge,content);CHECK(edge>0&&content==0);free(before);SelectObject(dc,old);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(dc));CHECK(ReleaseDC(w,window));return 0;
}
static BOOL redraw(HWND w){return RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);}
static int worker(int argc,WCHAR **argv){ImmDisableIME(0);CHECK(argc==2);HMODULE f=LoadLibraryW(argv[1]),dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(f&&dll);regfn reg=(void*)GetProcAddress(f,"FixtureRegister"),unreg=(void*)GetProcAddress(f,"FixtureUnregister");parentfn parent=(void*)GetProcAddress(f,"FixtureParent");createfn create=(void*)GetProcAddress(f,"FixtureCreate");windowfn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");void(WINAPI *raise_paint)(void)=(void*)GetProcAddress(f,"FixtureRaise");void(WINAPI *set_forward)(void*)=(void*)GetProcAddress(f,"FixtureForward");void(WINAPI *set_fake)(BOOL)=(void*)GetProcAddress(f,"FixtureFake");void(WINAPI *change_parent)(HWND)=(void*)GetProcAddress(f,"FixtureChangeParent");CHECK(reg&&unreg&&parent&&create&&attached&&remove&&raise_paint&&set_forward&&set_fake&&change_parent&&reg());HWND root=CreateWindowW(L"STATIC",L"box model",WS_OVERLAPPEDWINDOW,0,0,430,180,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);HWND p=parent(root);CHECK(p);HWND wrong=create(root,FALSE);CHECK(wrong&&redraw(wrong)&&!attached(wrong)&&DestroyWindow(wrong));HWND fake=create(p,TRUE);CHECK(fake&&redraw(fake)&&!attached(fake));raise_paint();CHECK(test_unwind(fake,NULL));CHECK(!attached(fake));set_fake(FALSE);CHECK(redraw(fake)&&attached(fake));CHECK(DestroyWindow(fake));set_forward((void*)foreign_forward);HWND foreign=create(p,FALSE);CHECK(foreign&&redraw(foreign)&&!attached(foreign));set_forward(NULL);CHECK(redraw(foreign)&&attached(foreign)&&DestroyWindow(foreign));HWND alt=CreateWindowW(L"STATIC",L"alt",WS_CHILD|WS_VISIBLE,0,0,300,100,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(alt);HWND stale=create(p,TRUE);CHECK(stale&&redraw(stale)&&!attached(stale));set_fake(FALSE);change_parent(alt);CHECK(redraw(stale)&&GetParent(stale)==alt&&!attached(stale));CHECK(DestroyWindow(stale));CHECK(DestroyWindow(alt));HWND w=create(p,FALSE);CHECK(w&&redraw(w)&&attached(w));CHECK(compare_chrome(w,remove)==0);CHECK(DestroyWindow(w));HWND moving=create(p,FALSE);CHECK(moving&&redraw(moving)&&attached(moving));CHECK(SetParent(moving,root));SendMessageW(moving,WM_ENABLE,TRUE,0);CHECK(!attached(moving));SetLastError(0);CHECK(!remove(moving)&&GetLastError()==ERROR_NOT_FOUND);CHECK(DestroyWindow(moving));HWND role=create(p,FALSE);CHECK(role&&redraw(role)&&attached(role));SetWindowLongPtrW(role,GWL_EXSTYLE,GetWindowLongPtrW(role,GWL_EXSTYLE)&~WS_EX_CLIENTEDGE);SetWindowPos(role,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);SendMessageW(role,WM_ENABLE,TRUE,0);CHECK(!attached(role));CHECK(DestroyWindow(role));CHECK(DestroyWindow(p)&&DestroyWindow(root)&&unreg()&&FreeLibrary(f));puts("BOX_MODEL_OK owner=1 fake=1 caller=1 witness=1 unwind=1 stale=1 reparent=1 role=1 pixels=1 cleanup=1");return 0;}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
