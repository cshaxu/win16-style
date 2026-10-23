/* Adapted from the fixed source context.cpp: standalone pointer-free policy. */
#include "injection.h"
#include "staging.h"
#include <wchar.h>
#include <string.h>
const GUID ntw16s_payload_guid={0xa170a252,0x7d0c,0x4b4a,{0xa2,0x86,0x61,0xd3,0x7a,0xea,0x39,0xc8}};
static BOOL dos_absolute(PCWSTR p,size_t n)
{return n>=3 && ((p[0]>=L'A' && p[0]<=L'Z') || (p[0]>=L'a' && p[0]<=L'z')) && p[1]==L':' && p[2]==L'\\';}
static BOOL unc_absolute(PCWSTR p)
{PCWSTR slash=wcschr(p,L'\\');return slash && slash>p && slash[1] && slash[1]!=L'\\';}
static BOOL absolute(PCWSTR p,size_t n)
{
    if(dos_absolute(p,n))return TRUE;
    if(n<5 || p[0]!=L'\\' || p[1]!=L'\\')return FALSE;
    if(p[2]==L'?' && p[3]==L'\\')return dos_absolute(p+4,n-4) ||
        (n>=8 && !_wcsnicmp(p+4,L"UNC\\",4) && unc_absolute(p+8));
    return p[2]!=L'.' && p[2]!=L'?' && unc_absolute(p+2);
}
DWORD ntw16s_packet_parse(const void *data,DWORD bytes,DWORD machine,ntw16s_context *context)
{
    ntw16s_packet h;const WCHAR *directory;
    if(!data || !context)return ERROR_INVALID_PARAMETER;
    ZeroMemory(context,sizeof(*context));
    if(bytes<sizeof(h) || bytes>65536)return ERROR_INVALID_DATA;
    memcpy(&h,data,sizeof(h));
    if(h.bytes!=bytes || h.version!=1 || h.machine!=machine ||
        (machine!=IMAGE_FILE_MACHINE_I386 && machine!=IMAGE_FILE_MACHINE_AMD64) ||
        !(h.flags&NTW16S_POLICY_STYLE) || (h.flags&~3u) || h.reserved0 || h.reserved1 ||
        h.directory_offset!=sizeof(h) || h.directory_bytes!=bytes-sizeof(h) ||
        h.directory_bytes<8 || h.directory_bytes%2)return ERROR_INVALID_DATA;
    /* Do not assume the external payload pointer's alignment. */
    memcpy(context->directory,(const BYTE *)data+sizeof(h),h.directory_bytes);
    directory=context->directory;
    size_t count=h.directory_bytes/sizeof(WCHAR);
    if(directory[count-1] || wcsnlen_s(directory,count)!=count-1 || !absolute(directory,count-1)){
        ZeroMemory(context,sizeof(*context));return ERROR_INVALID_DATA;
    }
    context->flags=h.flags;return ERROR_SUCCESS;
}
DWORD ntw16s_context_read(ntw16s_context *context,BOOL *found)
{
    return ntw16s_stage_read(context,found);
}
DWORD ntw16s_context_paths(ntw16s_context *context,HMODULE module)
{
    if(!context)return ERROR_INVALID_PARAMETER;
    ZeroMemory(context,sizeof(*context));
    DWORD count=GetModuleFileNameW(module,context->directory,NTW16S_PATH_CAP);
    if(!count || count>=NTW16S_PATH_CAP)return count?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    WCHAR *slash=wcsrchr(context->directory,L'\\');if(!slash)return ERROR_INVALID_NAME;
    slash[1]=0;context->flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE;return ERROR_SUCCESS;
}
