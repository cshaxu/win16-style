#include "import_scan.h"
#include <stdint.h>

#define IMPORT_PATCH_LIMIT 8u
static SRWLOCK import_patch_lock=SRWLOCK_INIT;
#ifdef NTW16S_IMPORT_PATCH_TESTING
extern BOOL ntw16s_import_patch_test_fail(size_t);
extern void ntw16s_import_patch_test_protected(void *);
extern BOOL ntw16s_import_patch_test_restore_fail(void);
#endif
static BOOL read_slot(void **slot,void **value)
{
    SIZE_T bytes=0;
    return ReadProcessMemory(GetCurrentProcess(),slot,value,sizeof(*value),&bytes) && bytes==sizeof(*value);
}

static BOOL slot_region(HMODULE image,void **slot,DWORD *writable)
{
    MEMORY_BASIC_INFORMATION region;DWORD type;
    if(!slot || (uintptr_t)slot%sizeof(void *) ||
       !VirtualQuery(slot,&region,sizeof(region)) || region.AllocationBase!=image ||
       region.State!=MEM_COMMIT || (region.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return FALSE;
    uintptr_t start=(uintptr_t)region.BaseAddress,where=(uintptr_t)slot;
    if(where<start || where-start>region.RegionSize || sizeof(void *)>region.RegionSize-(where-start))return FALSE;
    type=region.Protect&0xff;
    if(type==PAGE_READONLY || type==PAGE_READWRITE || type==PAGE_WRITECOPY)*writable=PAGE_READWRITE;
    else if(type==PAGE_EXECUTE_READ || type==PAGE_EXECUTE_READWRITE || type==PAGE_EXECUTE_WRITECOPY)*writable=PAGE_EXECUTE_READWRITE;
    else return FALSE;
    return TRUE;
}

/* Preserve executable access during the short write window: some VB runtime
 * import slots share a page with executable code. */
static DWORD exchange(HMODULE image,void **slot,void *from,void *to,BOOL *exchanged,DWORD *original)
{
    DWORD writable,old,ignored,error=ERROR_SUCCESS;void *previous;
    *exchanged=FALSE;*original=0;
    if(!slot_region(image,slot,&writable))return ERROR_INVALID_ADDRESS;
    if(!VirtualProtect(slot,sizeof(void *),writable,&old))return GetLastError();
    *original=old;
#ifdef NTW16S_IMPORT_PATCH_TESTING
    ntw16s_import_patch_test_protected(slot);
#endif
    previous=InterlockedCompareExchangePointer((void *volatile *)slot,to,from);
    *exchanged=previous==from;
#ifdef NTW16S_IMPORT_PATCH_TESTING
    if(ntw16s_import_patch_test_restore_fail())error=ERROR_ACCESS_DENIED;
    else
#endif
    if(!VirtualProtect(slot,sizeof(void *),old,&ignored))error=GetLastError();
    if(!error && !*exchanged)error=ERROR_BUSY;
    return error;
}

DWORD ntw16s_import_apply(HMODULE image,const ntw16s_import_change *changes,
                          size_t count,unsigned *changed)
{
    BOOL installed[IMPORT_PATCH_LIMIT]={0};DWORD protection[IMPORT_PATCH_LIMIT]={0},error=ERROR_SUCCESS;unsigned total=0;
    if(changed)*changed=0;
    if(!image || !changes || !count || count>IMPORT_PATCH_LIMIT || !changed)return ERROR_INVALID_PARAMETER;
    if(!TryAcquireSRWLockExclusive(&import_patch_lock))return ERROR_BUSY;
    for(size_t i=0;i<count;++i){
        DWORD writable;void *current;
        if(!changes[i].expected || !changes[i].replacement || changes[i].expected==changes[i].replacement ||
           !slot_region(image,changes[i].slot,&writable)){error=ERROR_INVALID_ADDRESS;goto done;}
        for(size_t j=0;j<i;++j)if(changes[j].slot==changes[i].slot){error=ERROR_INVALID_PARAMETER;goto done;}
        if(!read_slot(changes[i].slot,&current)){error=ERROR_INVALID_ADDRESS;goto done;}
        if(current!=changes[i].expected && current!=changes[i].replacement){error=ERROR_BUSY;goto done;}
    }
    for(size_t i=0;i<count;++i){
        void *current;
#ifdef NTW16S_IMPORT_PATCH_TESTING
        if(ntw16s_import_patch_test_fail(i)){error=ERROR_WRITE_FAULT;goto rollback;}
#endif
        if(!read_slot(changes[i].slot,&current)){error=ERROR_INVALID_ADDRESS;goto rollback;}
        if(current==changes[i].replacement)continue;
        error=exchange(image,changes[i].slot,changes[i].expected,changes[i].replacement,&installed[i],&protection[i]);
        if(installed[i])++total;
        if(error)goto rollback;
    }
    *changed=total;goto done;
rollback:
    for(size_t i=count;i>0;--i)if(protection[i-1]){
        if(installed[i-1]){
            BOOL restored=FALSE;DWORD previous=0;
            DWORD result=exchange(image,changes[i-1].slot,changes[i-1].replacement,changes[i-1].expected,&restored,&previous);
            if(result && result!=ERROR_BUSY)error=result;
        }
        DWORD ignored;
        if(!VirtualProtect(changes[i-1].slot,sizeof(void *),protection[i-1],&ignored))error=GetLastError();
    }
done:
    ReleaseSRWLockExclusive(&import_patch_lock);return error;
}
