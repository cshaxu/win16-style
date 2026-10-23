/* Native patch/abort/trampoline test in this expendable test process only. */
#include "ntw16s-dll/injection/startup_hooks.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STARTUP HOOK FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef DWORD (WINAPI *process_id)(void);
static process_id real_id;
static DWORD WINAPI replacement(void){return 0x42424242u;}
int main(void)
{
    DWORD actual=GetCurrentProcessId();process_id initial;
    /* GCC static dllimport initializers can name the executable import thunk.
     * Resolve through the IAT at runtime so the actual native export is patched. */
    real_id=GetCurrentProcessId;initial=real_id;
    CHECK(!ntw16s_hooks_begin());
    CHECK(ntw16s_hooks_begin()==ERROR_INVALID_STATE);
    CHECK(ntw16s_hooks_attach(NULL,(void *)replacement)==ERROR_INVALID_PARAMETER);
    CHECK(!ntw16s_hooks_attach(&real_id,(void *)replacement));
    CHECK(real_id()!=0x42424242u && GetCurrentProcessId()==actual);
    ntw16s_hooks_abort();CHECK(real_id==initial && real_id()==actual);
    CHECK(!ntw16s_hooks_begin());
    CHECK(!ntw16s_hooks_attach(&real_id,(void *)replacement));
    CHECK(!ntw16s_hooks_commit());
    CHECK(GetCurrentProcessId()==0x42424242u && real_id()==actual);
    CHECK(ntw16s_hooks_begin()==ERROR_INVALID_STATE);
    CHECK(ntw16s_hooks_commit()==ERROR_INVALID_STATE);
    printf("C STARTUP HOOK PASS width=%u native export/abort/trampoline/commit\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
