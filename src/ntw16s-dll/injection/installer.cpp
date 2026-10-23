/* Adapted from fixed installer.cpp; no service identity or transferred handles. */
#include "injection.h"
#include "detours/detours.h"
#include <wchar.h>
#include <string.h>
DWORD ntw16s_process_machine(HANDLE child,WORD *machine)
{
    USHORT process=0,native=0;
    if(!child || !machine)return ERROR_INVALID_PARAMETER;
    if(!IsWow64Process2(child,&process,&native))return GetLastError();
    if(native!=IMAGE_FILE_MACHINE_AMD64)return ERROR_NOT_SUPPORTED;
    if(process==IMAGE_FILE_MACHINE_UNKNOWN)*machine=IMAGE_FILE_MACHINE_AMD64;
    else if(process==IMAGE_FILE_MACHINE_I386)*machine=IMAGE_FILE_MACHINE_I386;
    else return ERROR_NOT_SUPPORTED;
    return ERROR_SUCCESS;
}
DWORD ntw16s_validate_image(PCWSTR path,WORD machine,BOOL dll)
{
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return GetLastError();
    IMAGE_DOS_HEADER dos;IMAGE_FILE_HEADER header;DWORD signature,read,error=ERROR_BAD_EXE_FORMAT;
    LARGE_INTEGER size,offset;WORD magic,subsystem;BYTE optional[sizeof(IMAGE_OPTIONAL_HEADER64)];
    if(!GetFileSizeEx(file,&size)){error=GetLastError();goto done;}
    if(!ReadFile(file,&dos,sizeof(dos),&read,NULL) || read!=sizeof(dos) || dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<(LONG)sizeof(dos))goto done;
    offset.QuadPart=dos.e_lfanew;
    if(offset.QuadPart>size.QuadPart-24 || !SetFilePointerEx(file,offset,NULL,FILE_BEGIN))goto done;
    if(!ReadFile(file,&signature,4,&read,NULL) || read!=4 || signature!=IMAGE_NT_SIGNATURE ||
       !ReadFile(file,&header,sizeof(header),&read,NULL) || read!=sizeof(header) || header.Machine!=machine ||
       !(header.Characteristics&IMAGE_FILE_EXECUTABLE_IMAGE) || !!(header.Characteristics&IMAGE_FILE_DLL)!=!!dll ||
       header.SizeOfOptionalHeader<70 || header.SizeOfOptionalHeader>sizeof(optional) ||
       size.QuadPart-offset.QuadPart-24<header.SizeOfOptionalHeader ||
       !ReadFile(file,optional,header.SizeOfOptionalHeader,&read,NULL) || read!=header.SizeOfOptionalHeader)goto done;
    memcpy(&magic,optional,2);memcpy(&subsystem,optional+68,2);
    if(magic!=(machine==IMAGE_FILE_MACHINE_I386?IMAGE_NT_OPTIONAL_HDR32_MAGIC:IMAGE_NT_OPTIONAL_HDR64_MAGIC) ||
       (subsystem!=IMAGE_SUBSYSTEM_WINDOWS_GUI && subsystem!=IMAGE_SUBSYSTEM_WINDOWS_CUI))goto done;
    error=ERROR_SUCCESS;
done:
    CloseHandle(file);return error;
}
static DWORD import_path(PCWSTR path,char *out,DWORD capacity)
{
    BOOL replaced=FALSE;WCHAR short_path[NTW16S_PATH_CAP],roundtrip[NTW16S_PATH_CAP];
    PCWSTR candidates[2]={path,short_path};
    DWORD short_count=GetShortPathNameW(path,short_path,NTW16S_PATH_CAP);
    for(unsigned i=0;i<(short_count && short_count<NTW16S_PATH_CAP?2u:1u);++i){
        replaced=FALSE;
        BOOL utf8=GetACP()==CP_UTF8;
        int count=WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,candidates[i],-1,out,(int)capacity,NULL,utf8?NULL:&replaced);
        if(count && !replaced && MultiByteToWideChar(CP_ACP,0,out,-1,roundtrip,NTW16S_PATH_CAP) && !wcscmp(roundtrip,candidates[i]))return ERROR_SUCCESS;
    }
    return ERROR_NO_UNICODE_TRANSLATION;
}
DWORD ntw16s_install(HANDLE child,const ntw16s_context *context,ntw16s_create_w create_helper)
{
    BYTE data[65536];ntw16s_packet header={0};ntw16s_context parsed;
    WORD machine=0;DWORD error;WCHAR path[NTW16S_PATH_CAP];char dll[4096];
    if(!child || !context || !create_helper)return ERROR_INVALID_PARAMETER;
    size_t length=wcsnlen_s(context->directory,NTW16S_PATH_CAP);
    if(!length || length>=NTW16S_PATH_CAP)return ERROR_INVALID_DATA;
    error=ntw16s_process_machine(child,&machine);if(error)return error;
    header.bytes=(DWORD)(sizeof(header)+(length+1)*sizeof(WCHAR));header.version=1;
    header.machine=machine;header.flags=context->flags;header.directory_offset=sizeof(header);
    header.directory_bytes=header.bytes-sizeof(header);
    memcpy(data,&header,sizeof(header));memcpy(data+sizeof(header),context->directory,header.directory_bytes);
    error=ntw16s_packet_parse(data,header.bytes,machine,&parsed);if(error)return error;
    if(swprintf_s(path,NTW16S_PATH_CAP,L"%ls%lsntw16s%u.dll",context->directory,
        context->directory[length-1]==L'\\'?L"":L"\\",machine==IMAGE_FILE_MACHINE_I386?32:64)<0)return ERROR_FILENAME_EXCED_RANGE;
    error=ntw16s_validate_image(path,machine,TRUE);if(error)return error;
    error=import_path(path,dll,sizeof(dll));if(error)return error;
    if(!DetourCopyPayloadToProcess(child,ntw16s_payload_guid,data,header.bytes))return GetLastError();
    LPCSTR imports[1]={dll};
    BOOL installed=machine==(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386)?
        DetourUpdateProcessWithDll(child,imports,1):
        DetourProcessViaHelperDllsW(GetProcessId(child),1,imports,create_helper);
    error=GetLastError();return installed?ERROR_SUCCESS:(error?error:ERROR_DLL_INIT_FAILED);
}
