#include <windows.h>
#include <stdio.h>

static DWORD wait_result;
static unsigned closed;
static BOOL WINAPI fake_desktop(HDESK desktop){(void)desktop;return TRUE;}
static BOOL WINAPI fake_set(HANDLE event){(void)event;return TRUE;}
static HANDLE WINAPI fake_event(LPSECURITY_ATTRIBUTES attributes,BOOL manual,BOOL initial,LPCWSTR name)
{(void)attributes;(void)manual;(void)initial;(void)name;return (HANDLE)(UINT_PTR)1;}
static HANDLE WINAPI fake_thread(LPSECURITY_ATTRIBUTES attributes,SIZE_T stack,LPTHREAD_START_ROUTINE entry,LPVOID arg,DWORD flags,LPDWORD id)
{(void)attributes;(void)stack;(void)entry;(void)arg;(void)flags;(void)id;return (HANDLE)(UINT_PTR)2;}
static DWORD WINAPI fake_wait(DWORD count,const HANDLE *handles,BOOL all,DWORD timeout)
{(void)count;(void)handles;(void)all;(void)timeout;SetLastError(ERROR_ACCESS_DENIED);return wait_result;}
static BOOL WINAPI fake_close(HANDLE handle)
{(void)handle;++closed;return TRUE;}
#define SetThreadDesktop fake_desktop
#define SetEvent fake_set
#define CreateEventW fake_event
#define CreateThread fake_thread
#define WaitForMultipleObjects fake_wait
#define CloseHandle fake_close
#include "../../../src/lib-win32/process-global/dispatch.c"

static BOOL next(void *context){(void)context;return FALSE;}
int main(void)
{
    const DWORD results[]={WAIT_FAILED,WAIT_TIMEOUT,WAIT_ABANDONED_0};
    const DWORD expected[]={ERROR_ACCESS_DENIED,ERROR_TIMEOUT,ERROR_INVALID_STATE};
    for(unsigned i=0;i<ARRAYSIZE(results);++i){
        win32_process_global_dispatch_pool pool={0};wait_result=results[i];closed=0;
        DWORD error=win32_process_global_dispatch_pool_start(&pool,1,(HDESK)(UINT_PTR)3,
            (HANDLE)(UINT_PTR)4,(HANDLE)(UINT_PTR)5,(HANDLE)(UINT_PTR)6,next,NULL,10);
        if(error!=expected[i] || pool.count!=1 || !pool.ready[0] || !pool.threads[0])return (int)i+1;
        win32_process_global_dispatch_pool_release(&pool);
        if(closed!=2 || pool.count)return (int)i+4;
    }
    win32_process_global_dispatch dispatch={0};
    dispatch.desktop=(HDESK)(UINT_PTR)3;dispatch.ready=(HANDLE)(UINT_PTR)4;dispatch.next=next;
    wait_result=WAIT_FAILED;if(win32_process_global_dispatch_run(&dispatch)!=ERROR_ACCESS_DENIED)return 7;
    wait_result=WAIT_ABANDONED_0;if(win32_process_global_dispatch_run(&dispatch)!=ERROR_INVALID_STATE)return 8;
    wait_result=WAIT_OBJECT_0;if(win32_process_global_dispatch_run(&dispatch)!=ERROR_SUCCESS)return 9;
    puts("PROCESS_GLOBAL_DISPATCH_ERRORS_OK failed=1 timeout=1 unexpected=1 ownership=1 runtime_exit=1");return 0;
}
