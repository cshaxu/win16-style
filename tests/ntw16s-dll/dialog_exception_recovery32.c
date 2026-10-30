#include <windows.h>
#include <stdio.h>
#include <setjmp.h>
#include <commdlg.h>
#include "ntw16s-dll/injection/unwind.h"
static BOOL caught;
static jmp_buf jump32;
static EXCEPTION_DISPOSITION NTAPI filter32(EXCEPTION_RECORD *e,void *frame,CONTEXT *context,void *dispatcher){(void)context;(void)dispatcher;if(e->ExceptionCode==0xe0475342u && !(e->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND))){caught=TRUE;RtlUnwind(frame,NULL,e,NULL);longjmp(jump32,1);}return ExceptionContinueSearch;}
static INT_PTR CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){(void)w;(void)p;(void)l;if(m==WM_INITDIALOG)RaiseException(0xe0475342u,0,0,NULL);return FALSE;}
static NTW16S_UNWIND_FUNCTION INT_PTR guarded(DLGTEMPLATE *t){volatile INT_PTR result=0;ntw16s_unwind_record record;record.handler=filter32;__asm__ __volatile__("movl %%fs:0,%0":"=r"(record.previous)::"memory");if(setjmp(jump32)==0){__asm__ __volatile__("movl %0,%%fs:0"::"r"(&record):"memory");result=DialogBoxIndirectParamW(GetModuleHandleW(NULL),t,NULL,proc,0);}__asm__ __volatile__("movl %0,%%fs:0"::"r"(record.previous):"memory");return result;}
static LONG shown,attached;typedef BOOL (__cdecl *install_fn)(HWND,const void *);
static VOID CALLBACK stop(HWND w,UINT m,UINT_PTR id,DWORD time){(void)m;(void)time;KillTimer(w,id);++shown;HMODULE dll=GetModuleHandleW(L"ntw16s32.dll");install_fn f=dll?(install_fn)(void*)GetProcAddress(dll,"win16_style_control_install"):NULL;HWND list=GetDlgItem(w,1121);if(f&&list){SetLastError(0);if(!f(list,NULL)&&GetLastError()==ERROR_ALREADY_EXISTS)attached=1;}PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),(LPARAM)GetDlgItem(w,IDCANCEL));}
static UINT_PTR CALLBACK old_hook(HWND w,UINT m,WPARAM p,LPARAM l){(void)p;(void)l;if(m==WM_INITDIALOG)SetTimer(w,77,80,stop);return 0;}
int wmain(void){struct {DLGTEMPLATE t;WORD menu,cls,title;} data={0};data.t.style=WS_POPUP|WS_CAPTION|WS_SYSMENU;data.t.cx=100;data.t.cy=80;INT_PTR result=guarded(&data.t);WCHAR path[MAX_PATH]=L"";OPENFILENAMEW f={0};f.lStructSize=sizeof(f);f.lpstrFile=path;f.nMaxFile=MAX_PATH;f.lpstrFilter=L"All\0*.*\0\0";f.lpfnHook=old_hook;f.Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR;BOOL second=GetOpenFileNameW(&f);DWORD error=CommDlgExtendedError();printf("EXCEPTION_RECOVERY caught=%d result=%lld shown=%ld attached=%ld second=%d error=%lu\n",caught,(long long)result,shown,attached,second,error);return caught&&shown==1&&attached==1&&!second&&!error?0:1;}
