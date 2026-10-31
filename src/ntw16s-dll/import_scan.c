#include "import_scan.h"
#include <stdint.h>
#include <string.h>

#define IMPORT_QUERY_LIMIT 8u
#define IMPORT_DESCRIPTOR_LIMIT 1024u
#define IMPORT_THUNK_LIMIT 65536u
#define IMPORT_NAME_LIMIT 128u

static BOOL span(size_t offset,size_t bytes,size_t limit)
{return offset<=limit && bytes<=limit-offset;}

static BOOL read_at(const BYTE *base,size_t limit,size_t offset,void *out,size_t bytes)
{
    SIZE_T got=0;size_t checked=0;
    if(!span(offset,bytes,limit) || (uintptr_t)base>UINTPTR_MAX-offset)return FALSE;
    uintptr_t address=(uintptr_t)base+offset;
    if(bytes && address>UINTPTR_MAX-(bytes-1))return FALSE;
    while(checked<bytes){
        MEMORY_BASIC_INFORMATION region;uintptr_t cursor=address+checked;
        if(!VirtualQuery((const void *)cursor,&region,sizeof(region)) || region.AllocationBase!=base ||
           region.State!=MEM_COMMIT || (region.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return FALSE;
        uintptr_t start=(uintptr_t)region.BaseAddress;
        if(cursor<start || cursor-start>=region.RegionSize)return FALSE;
        size_t available=region.RegionSize-(size_t)(cursor-start);
        checked+=min(available,bytes-checked);
    }
    return ReadProcessMemory(GetCurrentProcess(),base+offset,out,bytes,&got) && got==bytes;
}

static BOOL read_name(const BYTE *base,size_t limit,size_t offset,char out[IMPORT_NAME_LIMIT])
{
    for(size_t i=0;i<IMPORT_NAME_LIMIT;++i){
        if(offset>SIZE_MAX-i || !read_at(base,limit,offset+i,&out[i],1))return FALSE;
        if(!out[i])return TRUE;
    }
    return FALSE;
}

DWORD ntw16s_import_slots(HMODULE image,const ntw16s_import_query *queries,
                          size_t count,void ***slots)
{
    BYTE *base=(BYTE *)image;IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS nt;void **found[IMPORT_QUERY_LIMIT]={0};
    IMAGE_DATA_DIRECTORY imports;BOOL terminated=FALSE;DWORD error=ERROR_BAD_EXE_FORMAT;
    if(!image || !queries || !slots || !count || count>IMPORT_QUERY_LIMIT)return ERROR_INVALID_PARAMETER;
    for(size_t i=0;i<count;++i){slots[i]=NULL;if(!queries[i].library || !queries[i].symbol)return ERROR_INVALID_PARAMETER;}
    if(!read_at(base,1024u*1024u,0,&dos,sizeof(dos)) || dos.e_magic!=IMAGE_DOS_SIGNATURE ||
       dos.e_lfanew<64 || dos.e_lfanew>1024*1024-(LONG)sizeof(nt) ||
       !read_at(base,1024u*1024u,(size_t)dos.e_lfanew,&nt,sizeof(nt)) || nt.Signature!=IMAGE_NT_SIGNATURE ||
       nt.FileHeader.Machine!=(sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386) ||
       nt.OptionalHeader.Magic!=(sizeof(void*)==8?IMAGE_NT_OPTIONAL_HDR64_MAGIC:IMAGE_NT_OPTIONAL_HDR32_MAGIC) ||
       nt.FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER) ||
       nt.OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_IMPORT ||
       nt.OptionalHeader.SizeOfImage<(DWORD)dos.e_lfanew+sizeof(nt))return error;
    size_t size=nt.OptionalHeader.SizeOfImage;
    imports=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if(!imports.VirtualAddress || !imports.Size)return ERROR_PROC_NOT_FOUND;
    if(!span(imports.VirtualAddress,imports.Size,size))return error;
    size_t max_descriptors=imports.Size/sizeof(IMAGE_IMPORT_DESCRIPTOR);
    if(max_descriptors>IMPORT_DESCRIPTOR_LIMIT)max_descriptors=IMPORT_DESCRIPTOR_LIMIT;
    for(size_t i=0;i<max_descriptors;++i){
        IMAGE_IMPORT_DESCRIPTOR d,zero={0};char library[IMPORT_NAME_LIMIT];
        size_t off=(size_t)imports.VirtualAddress+i*sizeof(d);
        if(!read_at(base,size,off,&d,sizeof(d)))return error;
        if(!memcmp(&d,&zero,sizeof(d))){terminated=TRUE;break;}
        if(!d.Name || !d.OriginalFirstThunk || !d.FirstThunk || !read_name(base,size,d.Name,library))return error;
        BOOL wanted=FALSE;for(size_t q=0;q<count;++q)if(!lstrcmpiA(library,queries[q].library))wanted=TRUE;
        if(!wanted)continue;
        BOOL thunk_end=FALSE;
        for(size_t j=0;j<IMPORT_THUNK_LIMIT;++j){
            IMAGE_THUNK_DATA name;void *slot_value;size_t delta=j*sizeof(name),nameoff=(size_t)d.OriginalFirstThunk+delta,slotoff=(size_t)d.FirstThunk+delta;
            if(nameoff<(size_t)d.OriginalFirstThunk || slotoff<(size_t)d.FirstThunk ||
               !read_at(base,size,nameoff,&name,sizeof(name)) || !read_at(base,size,slotoff,&slot_value,sizeof(slot_value)))return error;
            if(!name.u1.AddressOfData){thunk_end=TRUE;break;}
            if(IMAGE_SNAP_BY_ORDINAL(name.u1.Ordinal))continue;
            if((uint64_t)name.u1.AddressOfData>SIZE_MAX-2)return error;
            char symbol[IMPORT_NAME_LIMIT];
            if(!read_name(base,size,(size_t)name.u1.AddressOfData+2,symbol))return error;
            for(size_t q=0;q<count;++q)if(!lstrcmpiA(library,queries[q].library) && !strcmp(symbol,queries[q].symbol)){
                if(found[q] || ((uintptr_t)(base+slotoff)%sizeof(void *)))return error;
                found[q]=(void **)(base+slotoff);
            }
        }
        if(!thunk_end)return error;
    }
    if(!terminated)return error;
    for(size_t i=0;i<count;++i)if(!found[i])return ERROR_PROC_NOT_FOUND;
    for(size_t i=0;i<count;++i)slots[i]=found[i];
    return ERROR_SUCCESS;
}
