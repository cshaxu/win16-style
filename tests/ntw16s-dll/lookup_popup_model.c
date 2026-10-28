#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <stdio.h>
#include <setjmp.h>
#include "ntw16s-dll/injection/unwind.h"
#define NTW16S_TEST_PROCESS_TIMEOUT 15000
#include "isolated_process.h"
#define CHECK(x) do {if(!(x)){fprintf(stderr,"LOOKUP_POP_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (__cdecl *winfn)(HWND);
static HWND made,root;static BOOL do_throw,wrong_owner;
static LRESULT CALLBACK popup_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK lookup_proc(HWND w,UINT m,WPARAM p,LPARAM l){
 if(m==WM_LBUTTONDOWN || m==WM_SYSKEYDOWN || m==WM_KEYDOWN){if(do_throw){do_throw=FALSE;RaiseException(0xe0425342u,0,0,NULL);}made=CreateWindowExW(WS_EX_TOOLWINDOW,L"TPopupDataList",L"",WS_POPUP|WS_BORDER|WS_VSCROLL,10,10,120,130,wrong_owner?NULL:root,NULL,GetModuleHandleW(NULL),NULL);return 0;}
 if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);FillRect(dc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));RECT b={r.right-16,0,r.right,r.bottom};HTHEME t=OpenThemeData(NULL,L"combobox");if(t){DrawThemeBackground(t,dc,1,1,&b,NULL);CloseThemeData(t);}EndPaint(w,&ps);return 0;}
 return DefWindowProcW(w,m,p,l);
}
static BOOL exception_seen;
#ifndef _WIN64
typedef struct catcher {ntw16s_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch_exception(EXCEPTION_RECORD *e,void *frame,CONTEXT *c,void *dispatcher){(void)c;(void)dispatcher;if(!(e->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && e->ExceptionCode==0xe0425342u){exception_seen=TRUE;RtlUnwind(frame,NULL,e,NULL);longjmp(((catcher*)frame)->saved,1);}return ExceptionContinueSearch;}
#else
static LONG __attribute__((used)) pop_filter(EXCEPTION_POINTERS *e,void *frame){(void)frame;if(e->ExceptionRecord->ExceptionCode==0xe0425342u){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}return EXCEPTION_CONTINUE_SEARCH;}
#endif
static NTW16S_UNWIND_FUNCTION BOOL test_exception(HWND w){exception_seen=FALSE;
#ifndef _WIN64
 catcher c;c.record.handler=catch_exception;__asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");__asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");if(!setjmp(c.saved))SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,0);__asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
 __asm__ __volatile__(".Lpop_start:\n.seh_handler __C_specific_handler, @except\n.seh_handlerdata\n.long 1\n.rva .Lpop_start,.Lpop_end,pop_filter,.Lpop_end\n.text\n":::"memory");SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,0);__asm__ __volatile__("nop\n.Lpop_end: nop\n":::"memory");
#endif
 return exception_seen;}
static int worker(int argc,WCHAR **argv){(void)argc;(void)argv;HMODULE d=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(d);winfn attached=(void*)GetProcAddress(d,"Ntw16sWindowAttached");CHECK(attached);HINSTANCE h=GetModuleHandleW(NULL);WNDCLASSW c={0};c.hInstance=h;c.lpfnWndProc=popup_proc;c.lpszClassName=L"TMainForm";CHECK(RegisterClassW(&c));c.lpszClassName=L"TPopupDataList";CHECK(RegisterClassW(&c));c.lpszClassName=L"TDBLookupComboBox";c.lpfnWndProc=lookup_proc;CHECK(RegisterClassW(&c));root=CreateWindowW(L"TMainForm",L"",WS_OVERLAPPEDWINDOW,0,0,250,150,NULL,NULL,h,NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBLookupComboBox",L"",WS_CHILD|WS_VISIBLE,0,0,120,20,root,NULL,h,NULL);CHECK(w);CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(w));HWND outsider=CreateWindowExW(WS_EX_TOOLWINDOW,L"TPopupDataList",L"",WS_POPUP|WS_BORDER|WS_VSCROLL,0,0,120,130,root,NULL,h,NULL);CHECK(outsider&&!attached(outsider));CHECK(DestroyWindow(outsider));made=NULL;SendMessageW(w,WM_SYSKEYDOWN,VK_DOWN,0);CHECK(made&&!attached(made));CHECK(DestroyWindow(made));made=NULL;SendMessageW(w,WM_SYSKEYDOWN,VK_UP,(LPARAM)((ULONG_PTR)1<<29));CHECK(made&&!attached(made));CHECK(DestroyWindow(made));made=NULL;SendMessageW(w,WM_KEYDOWN,VK_DOWN,(LPARAM)((ULONG_PTR)1<<29));CHECK(made&&!attached(made));CHECK(DestroyWindow(made));made=NULL;SendMessageW(w,WM_SYSKEYDOWN,VK_DOWN,(LPARAM)((ULONG_PTR)1<<29));CHECK(made&&attached(made));CHECK(DestroyWindow(made));made=NULL;wrong_owner=TRUE;SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,0);CHECK(made&&!attached(made));CHECK(DestroyWindow(made));wrong_owner=FALSE;made=NULL;SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,0);CHECK(made&&attached(made));HWND other=CreateWindowW(L"TMainForm",L"",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,h,NULL);CHECK(other);CHECK((HWND)SetWindowLongPtrW(made,GWLP_HWNDPARENT,(LONG_PTR)other)==root);SendMessageW(made,WM_NULL,0,0);CHECK(!attached(made));CHECK(DestroyWindow(made)&&DestroyWindow(other));made=NULL;SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,0);CHECK(made&&attached(made));CHECK(DestroyWindow(w));SendMessageW(made,WM_NULL,0,0);CHECK(!attached(made));CHECK(DestroyWindow(made));made=NULL;w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TDBLookupComboBox",L"",WS_CHILD|WS_VISIBLE,0,0,120,20,root,NULL,h,NULL);CHECK(w&&RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)&&attached(w));do_throw=TRUE;CHECK(test_exception(w));outsider=CreateWindowExW(WS_EX_TOOLWINDOW,L"TPopupDataList",L"",WS_POPUP|WS_BORDER|WS_VSCROLL,0,0,120,130,root,NULL,h,NULL);CHECK(outsider&&!attached(outsider));CHECK(DestroyWindow(outsider));CHECK(DestroyWindow(w)&&DestroyWindow(root));CHECK(UnregisterClassW(L"TDBLookupComboBox",h)&&UnregisterClassW(L"TPopupDataList",h)&&UnregisterClassW(L"TMainForm",h));printf("LOOKUP_POP_MODEL_OK width=%u scoped=1 alt_down=1 wrong_key=1 wrong_owner=1 outside=1 owner_change=1 source_destroy=1 exception=1 cleanup=1\n",(unsigned)(8*sizeof(void*)));return 0;}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
