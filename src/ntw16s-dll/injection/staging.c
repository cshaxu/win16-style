#include "staging.h"
#include <string.h>
#include <stdint.h>
#define STAGE_MAGIC0 UINT64_C(0x534e544953363157)
#define STAGE_MAGIC1 UINT64_C(0x3130474154534550)
#define IMPORT_LIMIT 1024u
#define STAGE_LIMIT (1024u*1024u)
typedef struct stage_header {
    uint64_t magic0,magic1;
    uint32_t version,allocation_bytes,packet_bytes,descriptor_offset;
    uint64_t allocation_base,image_base;
    IMAGE_DATA_DIRECTORY imports,bound,iat;
    DWORD checksum,reserved;
} stage_header;
_Static_assert(sizeof(stage_header)==80,"stage header must remain width independent");
typedef struct image_info {BYTE *base;IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS nt;} image_info;
static SIZE_T align_size(SIZE_T n,SIZE_T alignment){return (n+alignment-1)&~(alignment-1);}
static DWORD read_memory(HANDLE process,const void *address,void *out,SIZE_T bytes)
{
    SIZE_T read=0;
    if(!ReadProcessMemory(process,address,out,bytes,&read))return GetLastError();
    return read==bytes?ERROR_SUCCESS:ERROR_PARTIAL_COPY;
}
static DWORD read_image(HANDLE process,BYTE *base,image_info *image)
{
    DWORD error;ZeroMemory(image,sizeof(*image));image->base=base;
    error=read_memory(process,base,&image->dos,sizeof(image->dos));if(error)return error;
    if(image->dos.e_magic!=IMAGE_DOS_SIGNATURE || image->dos.e_lfanew<64 || image->dos.e_lfanew>1024*1024)
        return ERROR_BAD_EXE_FORMAT;
    error=read_memory(process,base+image->dos.e_lfanew,&image->nt,sizeof(image->nt));if(error)return error;
    if(image->nt.Signature!=IMAGE_NT_SIGNATURE ||
       image->nt.FileHeader.Machine!=(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386) ||
       image->nt.OptionalHeader.Magic!=(sizeof(void *)==8?IMAGE_NT_OPTIONAL_HDR64_MAGIC:IMAGE_NT_OPTIONAL_HDR32_MAGIC) ||
       image->nt.FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER) ||
       image->nt.OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_IAT ||
       image->nt.OptionalHeader.SizeOfImage<(DWORD)image->dos.e_lfanew+sizeof(image->nt))return ERROR_BAD_EXE_FORMAT;
    return ERROR_SUCCESS;
}
static uintptr_t address_limit(void)
{
#ifdef _WIN64
    SYSTEM_INFO info;GetSystemInfo(&info);return (uintptr_t)info.lpMaximumApplicationAddress;
#else
    /* WOW64 large-address-aware targets may map images above 2 GiB. */
    return UINTPTR_MAX-0x10000u;
#endif
}
static DWORD find_image(HANDLE process,image_info *image)
{
    uintptr_t cursor=0,limit=address_limit();MEMORY_BASIC_INFORMATION region;
    for(unsigned visited=0;visited<65536 && cursor<limit;++visited){
        if(!VirtualQueryEx(process,(void *)cursor,&region,sizeof(region)))return GetLastError();
        if(region.State==MEM_COMMIT && region.Type==MEM_IMAGE && region.BaseAddress==region.AllocationBase){
            DWORD error=read_image(process,region.AllocationBase,image);
            if(!error && !(image->nt.FileHeader.Characteristics&IMAGE_FILE_DLL))return ERROR_SUCCESS;
            /* A query-only handle can enumerate regions but cannot read their
             * PE headers. Preserve its access error rather than reporting an
             * unsupported executable after exhausting the address space. */
            if(error && error!=ERROR_BAD_EXE_FORMAT)return error;
        }
        uintptr_t next=(uintptr_t)region.BaseAddress+region.RegionSize;
        if(next<=cursor)break;
        cursor=next;
    }
    return ERROR_BAD_EXE_FORMAT;
}
static DWORD current_stage(HANDLE process,const image_info *image,stage_header *stage,BOOL *found)
{
    DWORD rva=image->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    *found=FALSE;ZeroMemory(stage,sizeof(*stage));
    if(rva<sizeof(*stage))return ERROR_SUCCESS;
    if(read_memory(process,image->base+rva-sizeof(*stage),stage,sizeof(*stage)))return ERROR_SUCCESS;
    if(stage->magic0!=STAGE_MAGIC0 || stage->magic1!=STAGE_MAGIC1)return ERROR_SUCCESS;
    *found=TRUE;
    if(stage->version!=1 || stage->reserved || stage->image_base!=(uint64_t)(uintptr_t)image->base ||
       stage->allocation_base>(uint64_t)UINTPTR_MAX || stage->allocation_bytes>STAGE_LIMIT ||
       stage->descriptor_offset<sizeof(*stage) || stage->descriptor_offset>stage->allocation_bytes ||
       stage->packet_bytes<sizeof(ntw16s_packet) || stage->packet_bytes>65536 ||
       stage->packet_bytes>stage->descriptor_offset-sizeof(*stage) ||
       stage->allocation_base+(uint64_t)stage->descriptor_offset!=(uint64_t)(uintptr_t)image->base+rva ||
       stage->allocation_base+stage->allocation_bytes<stage->allocation_base)return ERROR_INVALID_DATA;
    return ERROR_SUCCESS;
}
static BYTE *allocate_near(HANDLE process,const image_info *image,SIZE_T bytes)
{
    uintptr_t base=(uintptr_t)image->base,limit=address_limit(),cursor;
    MEMORY_BASIC_INFORMATION region;SYSTEM_INFO info;GetSystemInfo(&info);
    if(image->nt.OptionalHeader.SizeOfImage>limit || base>limit-image->nt.OptionalHeader.SizeOfImage){
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;
    }
    cursor=align_size(base+image->nt.OptionalHeader.SizeOfImage,info.dwAllocationGranularity);
    if(limit-base>UINT32_MAX)limit=base+UINT32_MAX;
    for(unsigned visited=0;visited<65536 && cursor<limit && bytes<=limit-cursor;++visited){
        if(!VirtualQueryEx(process,(void *)cursor,&region,sizeof(region)))return NULL;
        uintptr_t end=(uintptr_t)region.BaseAddress+region.RegionSize;
        if(end<=cursor)break;
        if(region.State==MEM_FREE && bytes<=end-cursor){
            BYTE *allocation=VirtualAllocEx(process,(void *)cursor,bytes,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
            if(allocation)return allocation;
        }
        if(end>limit)break;
        cursor=align_size(end,info.dwAllocationGranularity);
    }
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;
}
static DWORD write_header(HANDLE process,const image_info *image,const IMAGE_NT_HEADERS *nt)
{
    DWORD protect,unused,error=ERROR_SUCCESS;SIZE_T written=0;
    BYTE *address=image->base+image->dos.e_lfanew;
    if(!VirtualProtectEx(process,address,sizeof(*nt),PAGE_READWRITE,&protect))return GetLastError();
    if(!WriteProcessMemory(process,address,nt,sizeof(*nt),&written))error=GetLastError();
    else if(written!=sizeof(*nt))error=ERROR_PARTIAL_COPY;
    if(!VirtualProtectEx(process,address,sizeof(*nt),protect,&unused) && !error)error=GetLastError();
    return error;
}
DWORD ntw16s_stage_process(HANDLE child,const char *dll,const void *packet,DWORD bytes)
{
    image_info image;stage_header previous,*stage;BOOL staged;DWORD error;unsigned old_count=0;
    IMAGE_IMPORT_DESCRIPTOR *old,*imports;BYTE *scratch=NULL,*local=NULL,*remote=NULL;
    IMAGE_NT_HEADERS next;SIZE_T header_offset,descriptor_offset,thunk_offset,name_offset,total,written=0;
    uintptr_t *thunks;SIZE_T dll_bytes;
    if(!child || !dll || !packet || bytes<sizeof(ntw16s_packet) || bytes>65536)return ERROR_INVALID_PARAMETER;
    dll_bytes=strnlen(dll,4096)+1;if(dll_bytes>4096)return ERROR_FILENAME_EXCED_RANGE;
    error=find_image(child,&image);if(error)return error;
    error=current_stage(child,&image,&previous,&staged);if(error)return error;
    scratch=HeapAlloc(GetProcessHeap(),0,65536);if(!scratch)return ERROR_NOT_ENOUGH_MEMORY;
    old=(IMAGE_IMPORT_DESCRIPTOR *)scratch;next=image.nt;
    if(staged){
        if(previous.packet_bytes==bytes && !read_memory(child,(void *)(uintptr_t)previous.allocation_base,scratch,bytes) &&
           !memcmp(scratch,packet,bytes))goto done;
        next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]=previous.imports;
        next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]=previous.bound;
        next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]=previous.iat;
        next.OptionalHeader.CheckSum=previous.checksum;
    }
    DWORD import_rva=next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if(import_rva){
        for(;old_count<IMPORT_LIMIT;++old_count){
            SIZE_T at=(SIZE_T)import_rva+old_count*sizeof(*old);
            if(at>next.OptionalHeader.SizeOfImage || sizeof(*old)>next.OptionalHeader.SizeOfImage-at){error=ERROR_BAD_EXE_FORMAT;goto done;}
            error=read_memory(child,image.base+at,&old[old_count],sizeof(*old));if(error)goto done;
            if(!old[old_count].Name && !old[old_count].FirstThunk && !old[old_count].OriginalFirstThunk)break;
        }
        if(old_count==IMPORT_LIMIT){error=ERROR_BAD_EXE_FORMAT;goto done;}
    }
    header_offset=align_size(bytes,16);descriptor_offset=header_offset+sizeof(*stage);
    thunk_offset=align_size(descriptor_offset+(old_count+2)*sizeof(*imports),sizeof(uintptr_t));
    name_offset=thunk_offset+4*sizeof(uintptr_t);total=name_offset+dll_bytes;
    if(total>STAGE_LIMIT){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    local=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,total);if(!local){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    remote=allocate_near(child,&image,total);if(!remote){error=GetLastError();goto done;}
    memcpy(local,packet,bytes);stage=(stage_header *)(local+header_offset);
    stage->magic0=STAGE_MAGIC0;stage->magic1=STAGE_MAGIC1;stage->version=1;
    stage->allocation_bytes=(DWORD)total;stage->packet_bytes=bytes;stage->descriptor_offset=(DWORD)descriptor_offset;
    stage->allocation_base=(uint64_t)(uintptr_t)remote;stage->image_base=(uint64_t)(uintptr_t)image.base;
    stage->imports=next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    stage->bound=next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];
    stage->iat=next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];stage->checksum=next.OptionalHeader.CheckSum;
    DWORD rva=(DWORD)((uintptr_t)remote-(uintptr_t)image.base);
    imports=(IMAGE_IMPORT_DESCRIPTOR *)(local+descriptor_offset);
    memcpy(imports+1,old,old_count*sizeof(*old));
    imports->OriginalFirstThunk=rva+(DWORD)thunk_offset;imports->FirstThunk=imports->OriginalFirstThunk+2*sizeof(uintptr_t);
    imports->Name=rva+(DWORD)name_offset;
    thunks=(uintptr_t *)(local+thunk_offset);thunks[0]=thunks[2]=((uintptr_t)1<<(sizeof(uintptr_t)*8-1))|1;
    memcpy(local+name_offset,dll,dll_bytes);
    if(!WriteProcessMemory(child,remote,local,total,&written)){error=GetLastError();goto done;}
    if(written!=total){error=ERROR_PARTIAL_COPY;goto done;}
    next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress=rva+(DWORD)descriptor_offset;
    next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size=(old_count+2)*sizeof(*imports);
    next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress=0;
    next.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size=0;next.OptionalHeader.CheckSum=0;
    error=write_header(child,&image,&next);
    if(error){
        /* Creator still owns this suspended target and must roll it back if
         * any write/protection step failed. Never release a possibly live table. */
        remote=NULL;goto done;
    }
    remote=NULL;
done:
    if(remote)VirtualFreeEx(child,remote,0,MEM_RELEASE);
    if(local)HeapFree(GetProcessHeap(),0,local);
    if(scratch)HeapFree(GetProcessHeap(),0,scratch);
    return error;
}
DWORD ntw16s_stage_read(ntw16s_context *context,BOOL *found)
{
    image_info image;stage_header stage;BYTE *packet;DWORD error;
    if(!context || !found)return ERROR_INVALID_PARAMETER;
    *found=FALSE;ZeroMemory(context,sizeof(*context));
    error=read_image(GetCurrentProcess(),(BYTE *)GetModuleHandleW(NULL),&image);if(error)return error;
    error=current_stage(GetCurrentProcess(),&image,&stage,found);if(error || !*found)return error;
    packet=HeapAlloc(GetProcessHeap(),0,stage.packet_bytes);if(!packet)return ERROR_NOT_ENOUGH_MEMORY;
    error=read_memory(GetCurrentProcess(),(void *)(uintptr_t)stage.allocation_base,packet,stage.packet_bytes);
    if(!error)error=ntw16s_packet_parse(packet,stage.packet_bytes,image.nt.FileHeader.Machine,context);
    HeapFree(GetProcessHeap(),0,packet);return error;
}
DWORD ntw16s_stage_restore(void)
{
    image_info image;stage_header stage;BOOL found;DWORD error;
    error=read_image(GetCurrentProcess(),(BYTE *)GetModuleHandleW(NULL),&image);if(error)return error;
    error=current_stage(GetCurrentProcess(),&image,&stage,&found);if(error || !found)return error?error:ERROR_INVALID_DATA;
    image.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]=stage.imports;
    image.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]=stage.bound;
    image.nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]=stage.iat;
    image.nt.OptionalHeader.CheckSum=stage.checksum;
    return write_header(GetCurrentProcess(),&image,&image.nt);
}
