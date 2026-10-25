/* Native patch/abort/trampoline test in this expendable test process only. */
#include "ntw16s-dll/injection/startup_hooks.h"
#include <stdio.h>
#include <string.h>
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
    /* A native patch transaction must reject slot41 and roll back all40
     * owned trampolines without patching any generated target. */
    BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);CHECK(code);
    process_id slots[41];for(unsigned i=0;i<41;++i){
        BYTE *at=code+i*32;for(unsigned j=0;j<32;++j)at[j]=0x90;at[8]=0xb8;
        DWORD value=0x1010u+i;memcpy(at+9,&value,4);at[13]=0xc3;slots[i]=(process_id)(at+8);
    }
    CHECK(FlushInstructionCache(GetCurrentProcess(),code,4096));CHECK(!ntw16s_hooks_begin());
    for(unsigned i=0;i<40;++i){CHECK(!ntw16s_hooks_attach(&slots[i],(void *)replacement));CHECK(slots[i]()==0x1010u+i);}
    CHECK(ntw16s_hooks_attach(&slots[40],(void *)replacement)==ERROR_INSUFFICIENT_BUFFER && slots[40]()==0x1038u);
    ntw16s_hooks_abort();for(unsigned i=0;i<41;++i)CHECK(slots[i]==(process_id)(code+i*32+8) && slots[i]()==0x1010u+i);
    CHECK(VirtualFree(code,0,MEM_RELEASE));puts("STARTUP_CAPACITY_OK slots=40 overflow41=1 abort_restored=41");
    CHECK(!ntw16s_hooks_begin());
    CHECK(!ntw16s_hooks_attach(&real_id,(void *)replacement));
    CHECK(!ntw16s_hooks_commit());
    CHECK(GetCurrentProcessId()==0x42424242u && real_id()==actual);
    CHECK(ntw16s_hooks_begin()==ERROR_INVALID_STATE);
    CHECK(ntw16s_hooks_commit()==ERROR_INVALID_STATE);
    printf("C STARTUP HOOK PASS width=%u native export/abort/trampoline/commit\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
