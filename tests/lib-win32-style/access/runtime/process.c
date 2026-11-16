#include "lib-win32-style/access/runtime/process.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"runtime process line=%d\n",__LINE__);return 1;}}while(0)
static style_runtime_process state,nested;
static style_startup_context process,nested_process;
static BOOL registered,sticky,nest;
static DWORD prepare_error,begin_error,commit_error;
static unsigned preparations,discards,commits;
static process_hook_late_batch *observed;
static DWORD actual_flags;static BOOL actual_reuse;
BOOL process_hook_propagation_registered(void){return registered;}
DWORD process_hook_late_batch_begin(process_hook_late_batch *batch,process_hook_late_patch *patches,
    process_hook_late_patch **items,DWORD capacity)
{
    if(capacity!=88)return ERROR_INVALID_PARAMETER;
    batch->patches=patches;batch->items=items;batch->capacity=capacity;batch->count=0;
    return begin_error;
}
DWORD process_hook_late_batch_prepare(process_hook_late_batch *batch,void *slot,void *replacement)
{
    (void)slot;(void)replacement;observed=batch;
    if(batch->count==batch->capacity)return ERROR_INSUFFICIENT_BUFFER;
    ++batch->count;return 0;
}
DWORD process_hook_late_batch_commit(process_hook_late_batch *batch,DWORD error,DWORD attempts,DWORD delay)
{
    ++commits;if(attempts!=100 || delay!=1)return ERROR_INVALID_PARAMETER;
    if(!error)error=commit_error;
    if(error){batch->count=0;if(sticky)batch->sticky_error=error;}
    else batch->installed=TRUE;
    return error;
}
static DWORD prepare(DWORD (*attach)(void *,void *),BOOL reuse,DWORD flags,void *context);
static void discard(void *context){(void)context;++discards;}
static const style_runtime_process_ops operations={L"example",prepare,discard,NULL};
static DWORD prepare(DWORD (*attach)(void *,void *),BOOL reuse,DWORD flags,void *context)
{
    (void)context;++preparations;actual_flags=flags;actual_reuse=reuse;
    DWORD error=attach(NULL,NULL);if(error)return error;
    if(nest){
        nest=FALSE;error=style_runtime_process_init(&nested,&nested_process,&operations,L"C:\\fixture",0,2,TRUE);
        if(error || observed!=&nested.batch)return ERROR_INVALID_DATA;
    }
    error=attach(NULL,NULL);return error?error:prepare_error;
}
static void reset(void)
{
    ZeroMemory(&state,sizeof(state));ZeroMemory(&process,sizeof(process));
    registered=sticky=nest=FALSE;prepare_error=begin_error=commit_error=0;
    preparations=discards=commits=0;observed=NULL;
}
static DWORD initialize(PCWSTR directory,DWORD cookie,DWORD flags,BOOL strict)
{return style_runtime_process_init(&state,&process,&operations,directory,cookie,flags,strict);}
int main(void)
{
    CHECK(initialize(L"C:\\fixture",0,1,TRUE)==ERROR_INVALID_PARAMETER);
    CHECK(initialize(NULL,0,2,TRUE)==ERROR_INVALID_PARAMETER);
    CHECK(initialize(L"relative",0,2,TRUE)==ERROR_INVALID_NAME);
    CHECK(!preparations && !process.flags);
    CHECK(!initialize(L"C:\\fixture",0,2,TRUE) && actual_flags==2 && !actual_reuse &&
        state.batch.installed && state.batch.capacity==88 && !wcscmp(process.module,L"example"));
    CHECK(!initialize(L"C:\\fixture",0,2,TRUE) && preparations==1 && commits==1);
    CHECK(initialize(L"C:\\fixture",123,3,TRUE)==ERROR_ALREADY_EXISTS && !process.session_cookie);
    CHECK(!initialize(L"C:\\fixture",123,3,FALSE) && process.session_cookie==123 && process.flags==6 && preparations==1);
    reset();registered=TRUE;
    CHECK(initialize(L"C:\\fixture",0,3,FALSE)==ERROR_ALREADY_EXISTS && !preparations);
    CHECK(!initialize(L"C:\\fixture",123,3,FALSE) && actual_reuse && actual_flags==3);
    reset();prepare_error=ERROR_ACCESS_DENIED;
    CHECK(initialize(L"C:\\fixture",0,2,TRUE)==prepare_error && !process.flags && discards==1);
    prepare_error=0;CHECK(!initialize(L"C:\\fixture",0,2,TRUE) && preparations==2);
    reset();registered=TRUE;prepare_error=ERROR_CANCELLED;
    CHECK(initialize(L"C:\\fixture",123,3,FALSE)==prepare_error && !discards && !process.flags);
    reset();begin_error=ERROR_BUSY;
    CHECK(initialize(L"C:\\fixture",0,2,TRUE)==begin_error && !preparations && !process.flags && discards==1);
    reset();sticky=TRUE;commit_error=ERROR_PARTIAL_COPY;
    CHECK(initialize(L"C:\\fixture",0,2,TRUE)==commit_error && process.flags==2 && !discards);
    CHECK(initialize(L"C:\\fixture",0,2,TRUE)==commit_error && preparations==1);
    reset();nest=TRUE;
    CHECK(!initialize(L"C:\\fixture",0,2,TRUE) && observed==&state.batch &&
        nested.batch.count==2 && state.batch.count==2);
    puts("RUNTIME_PROCESS_OK flags=1 adoption=1 repeat=1 rollback=1 retry=1 reuse=1 sticky=1 batch_binding=1 capacity=88");return 0;
}
