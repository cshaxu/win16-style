#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/import_scan.h"

#define CHECK(x) do{if(!(x)){printf("IMPORT_PATCH FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct race {HANDLE start;HMODULE image;ntw16s_import_change change;DWORD error;unsigned changed;} race;
static unsigned fault_mode;
static void **fault_slot;
static void *fault_foreign;
static int (*page_code)(void);
static unsigned executable_checks;
static BOOL fail_restore_once;
BOOL ntw16s_import_patch_test_restore_fail(void)
{BOOL fail=fail_restore_once;fail_restore_once=FALSE;return fail;}
void ntw16s_import_patch_test_protected(void *slot)
{
    MEMORY_BASIC_INFORMATION info;
    if(VirtualQuery(slot,&info,sizeof(info)) && (info.Protect&0xff)==PAGE_EXECUTE_READWRITE && page_code && page_code()==42)
        ++executable_checks;
}
BOOL ntw16s_import_patch_test_fail(size_t index)
{
    if(index!=1 || !fault_mode)return FALSE;
    if(fault_mode==2){DWORD old,ignored;
        if(!VirtualProtect(fault_slot,sizeof(void *),PAGE_EXECUTE_READWRITE,&old))return FALSE;
        InterlockedExchangePointer((void *volatile *)fault_slot,fault_foreign);
        if(!VirtualProtect(fault_slot,sizeof(void *),old,&ignored))return FALSE;
    }
    return TRUE;
}
static DWORD WINAPI compete(void *data)
{
    race *r=data;if(WaitForSingleObject(r->start,2000)!=WAIT_OBJECT_0)return 1;
    r->error=ntw16s_import_apply(r->image,&r->change,1,&r->changed);return 0;
}
static BOOL protect(void *p,DWORD mode)
{DWORD old;return VirtualProtect(p,4096,mode,&old);}
static DWORD protection(void *p)
{MEMORY_BASIC_INFORMATION info;return VirtualQuery(p,&info,sizeof(info))?info.Protect:0;}

int wmain(void)
{
    void **base=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);CHECK(base);
    const BYTE machine_code[]={0xb8,42,0,0,0,0xc3};
    CopyMemory((BYTE *)base+0x100,machine_code,sizeof(machine_code));
    CHECK(FlushInstructionCache(GetCurrentProcess(),(BYTE *)base+0x100,sizeof(machine_code)));
    page_code=(int (*)(void))(void *)((BYTE *)base+0x100);
    void *a=(void *)(UINT_PTR)0x11111110,*b=(void *)(UINT_PTR)0x22222220;
    void *x=(void *)(UINT_PTR)0x33333330,*y=(void *)(UINT_PTR)0x44444440,*foreign=(void *)(UINT_PTR)0x55555550;
    base[0]=a;base[1]=b;CHECK(protect(base,PAGE_EXECUTE_READ));
    ntw16s_import_change changes[]={{&base[0],a,x},{&base[1],b,y}};unsigned changed=99;
    CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_SUCCESS && changed==2);
    CHECK(base[0]==x && base[1]==y && protection(base)==PAGE_EXECUTE_READ);
    CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_SUCCESS && changed==0);
    CHECK(protect(base,PAGE_READWRITE));base[0]=a;base[1]=foreign;CHECK(protect(base,PAGE_EXECUTE_READ));
    CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_BUSY && changed==0);
    CHECK(base[0]==a && base[1]==foreign && protection(base)==PAGE_EXECUTE_READ);
    CHECK(protect(base,PAGE_READWRITE));base[0]=a;base[1]=b;CHECK(protect(base,PAGE_EXECUTE_READ));
    fault_mode=1;CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_WRITE_FAULT && changed==0);
    CHECK(base[0]==a && base[1]==b && protection(base)==PAGE_EXECUTE_READ);
    fault_mode=2;fault_slot=&base[0];fault_foreign=foreign;
    CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_WRITE_FAULT && changed==0);
    CHECK(base[0]==foreign && base[1]==b && protection(base)==PAGE_EXECUTE_READ);fault_mode=0;
    CHECK(protect(base,PAGE_READWRITE));base[0]=a;base[1]=b;CHECK(protect(base,PAGE_EXECUTE_READ));
    fail_restore_once=TRUE;CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_ACCESS_DENIED && changed==0);
    CHECK(base[0]==a && base[1]==b && protection(base)==PAGE_EXECUTE_READ);
    changes[1].slot=&base[0];CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_INVALID_PARAMETER && changed==0);
    changes[1].slot=(void **)((BYTE *)base+1);CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_INVALID_ADDRESS);
    changes[1].slot=&base[1];CHECK(protect(base,PAGE_EXECUTE_READ|PAGE_GUARD));
    CHECK(ntw16s_import_apply((HMODULE)base,changes,2,&changed)==ERROR_INVALID_ADDRESS);
    CHECK(protect(base,PAGE_READWRITE));base[0]=a;base[1]=b;CHECK(protect(base,PAGE_EXECUTE_READ));
    HANDLE start=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(start);
    race r[2]={{start,(HMODULE)base,{&base[0],a,x},0,0},{start,(HMODULE)base,{&base[0],a,y},0,0}};
    HANDLE threads[2]={CreateThread(NULL,0,compete,&r[0],0,NULL),CreateThread(NULL,0,compete,&r[1],0,NULL)};
    CHECK(threads[0] && threads[1] && SetEvent(start));
    CHECK(WaitForMultipleObjects(2,threads,TRUE,2000)==WAIT_OBJECT_0);
    CHECK((r[0].error==ERROR_SUCCESS && r[1].error==ERROR_BUSY) || (r[1].error==ERROR_SUCCESS && r[0].error==ERROR_BUSY));
    CHECK(r[0].changed+r[1].changed==1 && (base[0]==x || base[0]==y) && protection(base)==PAGE_EXECUTE_READ && executable_checks>0);
    CloseHandle(threads[0]);CloseHandle(threads[1]);CloseHandle(start);
    CHECK(VirtualFree(base,0,MEM_RELEASE));
    printf("IMPORT_PATCH PASS bits=%u guarded=1 foreign=1 rollback=3 concurrent=2 protection=1 execution=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
