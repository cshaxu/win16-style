#include "ntw16s-dll/injection/intercept.h"
#include "lib-win32/process-hook/propagate.h"
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"shell unit line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
ntwshk_context ntw16s_process_context;
static BOOL (WINAPI *hooked)(SHELLEXECUTEINFOW *);
static WCHAR actual_file[32768],actual_args[32768];
static BOOL fake_result;
static DWORD fake_error=ERROR_CANCELLED;
static DWORD actual_mask;static HWND actual_owner;static int actual_show;static WCHAR actual_directory[32768];
static BOOL WINAPI fake_shell(SHELLEXECUTEINFOW *p){wcsncpy(actual_file,p->lpFile?p->lpFile:L"",32767);wcsncpy(actual_args,p->lpParameters?p->lpParameters:L"",32767);wcsncpy(actual_directory,p->lpDirectory?p->lpDirectory:L"",32767);actual_mask=p->fMask;actual_owner=p->hwnd;actual_show=p->nShow;p->hInstApp=(HINSTANCE)(INT_PTR)42;SetLastError(fake_error);return fake_result;}
static DWORD attach(void *slot,void *replacement){memcpy(&hooked,&replacement,sizeof(hooked));BOOL (WINAPI *fn)(SHELLEXECUTEINFOW *)=fake_shell;memcpy(slot,&fn,sizeof(fn));return 0;}
DWORD process_hook_propagation_attach(DWORD (*a)(void *,void *),const process_hook_propagation_ops *o){(void)a;(void)o;return 0;}
void process_hook_propagation_discard(const process_hook_propagation_ops *o){(void)o;}
DWORD process_hook_propagation_attach_as_user_w(DWORD (*a)(void *,void *)){(void)a;return 0;}
DWORD process_hook_startup_attach(void *a,void *b){(void)a;(void)b;return 0;}
BOOL ntw16s_global_policy_active(DWORD c){(void)c;return FALSE;}
BOOL ntw16s_global_policy_style(DWORD c,DWORD style){(void)c;(void)style;return FALSE;}
BOOL ntwshk_system_print_host_w(LPCWSTR a,LPWSTR b){(void)a;(void)b;return FALSE;}
BOOL ntwshk_system_print_host_a(LPCSTR a,LPSTR b){(void)a;(void)b;return FALSE;}
DWORD ntwshk_install(HANDLE h,const ntwshk_context *c,ntwshk_create_w f){(void)h;(void)c;(void)f;return 0;}
int wmain(void){
 WCHAR self[32752],expected[32768];CHECK(GetModuleFileNameW(NULL,self,32752));CHECK(LoadLibraryW(L"shell32.dll"));
 wcscpy(ntw16s_process_context.directory,L"C:\\package\\");ntw16s_process_context.flags=NTWSHK_POLICY_STYLE|NTWSHK_POLICY_PROPAGATE;
 CHECK(!ntw16s_attach_processes_using(attach) && hooked);
 SHELLEXECUTEINFOW p={.cbSize=sizeof(p),.fMask=SEE_MASK_FLAG_NO_UI,.hwnd=(HWND)(INT_PTR)123,.lpVerb=L"runas",.lpFile=self,.lpParameters=L"\"a b\" tail\\",.lpDirectory=L"C:\\work dir",.nShow=SW_SHOWMINIMIZED};
 CHECK(!hooked(&p) && GetLastError()==ERROR_CANCELLED && p.hInstApp==(HINSTANCE)(INT_PTR)42);
 CHECK(!wcscmp(actual_file,L"C:\\package\\run16s.exe"));
 CHECK(actual_mask==p.fMask && actual_owner==p.hwnd && actual_show==p.nShow && !wcscmp(actual_directory,p.lpDirectory));
 CHECK(swprintf(expected,32768,L"--elevated \"%ls\" \"a b\" tail\\",self)>0 && !wcscmp(actual_args,expected));
 fake_result=TRUE;fake_error=ERROR_ALREADY_EXISTS;p.hInstApp=NULL;
 CHECK(hooked(&p) && GetLastError()==ERROR_ALREADY_EXISTS && p.hInstApp==(HINSTANCE)(INT_PTR)42);
 CHECK(!wcscmp(actual_file,L"C:\\package\\run16s.exe") && !wcscmp(actual_args,expected));
 fake_result=FALSE;fake_error=ERROR_CANCELLED;
 p.fMask|=SEE_MASK_NOCLOSEPROCESS;CHECK(!hooked(&p) && !wcscmp(actual_file,self));
 p.fMask=0;CHECK(!hooked(&p) && !wcscmp(actual_file,self));
 p.fMask=SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC;CHECK(!hooked(&p) && !wcscmp(actual_file,self));
 p.fMask=SEE_MASK_FLAG_NO_UI;p.lpFile=L"C:\\unrelated.exe";CHECK(!hooked(&p) && !wcscmp(actual_file,p.lpFile));
 p.lpFile=self;p.lpVerb=L"open";CHECK(!hooked(&p) && !wcscmp(actual_file,self));
 p.lpVerb=L"runas";p.lpParameters=NULL;CHECK(!hooked(&p) && !wcscmp(actual_file,L"C:\\package\\run16s.exe"));
 CHECK(swprintf(expected,32768,L"--elevated \"%ls\"",self)>0 && !wcscmp(actual_args,expected));
 WCHAR *long_tail=calloc(32768,sizeof(WCHAR));CHECK(long_tail);wmemset(long_tail,L'x',32767);p.lpParameters=long_tail;
 CHECK(!hooked(&p) && !wcscmp(actual_file,self));free(long_tail);p.lpParameters=NULL;
 p.lpVerb=L"runas";ntw16s_process_context.flags|=NTWSHK_POLICY_GLOBAL_SESSION;CHECK(!hooked(&p) && !wcscmp(actual_file,self));
 hooked=NULL;CHECK(!ntw16s_attach_processes_using(attach) && !hooked);
 ntw16s_process_context.flags=NTWSHK_POLICY_PROPAGATE;CHECK(!ntw16s_attach_processes_using(attach) && !hooked);
 puts("shell redirect unit pass");return 0;
}
