#include <windows.h>
#include <stdio.h>
#include "lib-win32/process-hook/startup_hooks.h"
static DWORD begin_error,prepare_error,commit_error;
static unsigned begin_count,prepare_count,commit_count,abort_count;
static void *expected_context;
static DWORD fake_begin(void){++begin_count;return begin_error;}
static DWORD fake_commit(void){++commit_count;return commit_error;}
static void fake_abort(void){++abort_count;}
#define process_hook_startup_begin fake_begin
#define process_hook_startup_commit fake_commit
#define process_hook_startup_abort fake_abort
#include "lib-win32/style/startup.h"
static DWORD prepare(void *context)
{++prepare_count;return context==expected_context?prepare_error:ERROR_INVALID_PARAMETER;}
static void reset(void)
{begin_error=prepare_error=commit_error=0;begin_count=prepare_count=commit_count=abort_count=0;}
int main(void)
{
    int context=0;expected_context=&context;
    reset();if(win32_style_startup(prepare,&context) || begin_count!=1 || prepare_count!=1 || commit_count!=1 || abort_count)return 1;
    reset();begin_error=ERROR_BUSY;if(win32_style_startup(prepare,&context)!=ERROR_BUSY || begin_count!=1 || prepare_count || commit_count || abort_count)return 2;
    reset();prepare_error=ERROR_ACCESS_DENIED;if(win32_style_startup(prepare,&context)!=ERROR_ACCESS_DENIED || begin_count!=1 || prepare_count!=1 || commit_count || abort_count!=1)return 3;
    reset();commit_error=ERROR_INVALID_STATE;if(win32_style_startup(prepare,&context)!=ERROR_INVALID_STATE || begin_count!=1 || prepare_count!=1 || commit_count!=1 || abort_count)return 4;
    puts("STYLE STARTUP PASS success=1 begin_failure=1 prepare_rollback=1 commit_failure=1 context=1");return 0;
}
