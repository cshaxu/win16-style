#include "lib-win32-style/access/startup/startup.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"bootstrap line=%d\n",__LINE__);return 1;}}while(0)
static DWORD read_error,restore_error,begin_error,prepare_error,commit_error;
static unsigned restores,begins,prepares,aborts,commits;
static BOOL packet;
static DWORD flags;
DWORD style_startup_read(style_startup_context *context,BOOL *found)
{
    ZeroMemory(context,sizeof(*context));wcscpy(context->module,L"example");
    context->flags=flags;*found=packet;return read_error;
}
DWORD process_hook_stage_restore(void){++restores;return restore_error;}
DWORD process_hook_startup_begin(void){++begins;return begin_error;}
void process_hook_startup_abort(void){++aborts;}
DWORD process_hook_startup_commit(void){++commits;return commit_error;}
static DWORD prepare(void *context){++prepares;return context==&packet?prepare_error:ERROR_INVALID_PARAMETER;}
static DWORD run(PCWSTR module,BOOL session)
{
    static style_startup_context context;BOOL found=FALSE;
    restores=begins=prepares=aborts=commits=0;
    return style_startup_bootstrap(&context,module,session,&found,prepare,&packet);
}
int main(void)
{
    CHECK(!run(L"example",FALSE) && !restores && !begins);
    read_error=ERROR_INVALID_DATA;CHECK(run(L"example",FALSE)==read_error && !restores);read_error=0;
    packet=TRUE;CHECK(run(L"other",TRUE)==ERROR_INVALID_DATA && !restores);
    flags=STYLE_STARTUP_POLICY_GLOBAL_SESSION;
    CHECK(run(L"example",FALSE)==ERROR_INVALID_DATA && !restores);
    CHECK(!run(L"EXAMPLE",TRUE) && restores==1 && begins==1 && prepares==1 && commits==1 && !aborts);
    flags=0;restore_error=ERROR_ACCESS_DENIED;
    CHECK(run(L"example",FALSE)==restore_error && restores==1 && !begins);restore_error=0;
    begin_error=ERROR_BUSY;CHECK(run(L"example",FALSE)==begin_error && !prepares && !aborts);begin_error=0;
    prepare_error=ERROR_NOT_ENOUGH_MEMORY;
    CHECK(run(L"example",FALSE)==prepare_error && prepares==1 && aborts==1 && !commits);prepare_error=0;
    commit_error=ERROR_INVALID_FUNCTION;
    CHECK(run(L"example",FALSE)==commit_error && commits==1 && !aborts);
    puts("startup bootstrap pass: passive identity session restore transaction rollback");return 0;
}
