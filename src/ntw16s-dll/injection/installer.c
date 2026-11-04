/* Adapted from fixed installer.cpp; no service identity or transferred handles. */
#include "injection.h"
#include "staging.h"
#include "lib-win32/process-image/image.h"
#include <wchar.h>
#include <string.h>
DWORD ntw16s_process_machine(HANDLE child,WORD *machine)
{return win32_process_image_machine(child,machine);}
DWORD ntw16s_validate_image_ex(PCWSTR path,WORD machine,BOOL dll,WORD *subsystem)
{return win32_process_image_validate_ex(path,machine,dll,subsystem);}
DWORD ntw16s_validate_image(PCWSTR path,WORD machine,BOOL dll)
{return win32_process_image_validate(path,machine,dll);}
static DWORD import_path(PCWSTR path,char *out,DWORD capacity)
{
    typedef struct path_work {WCHAR short_path[NTW16S_PATH_CAP],roundtrip[NTW16S_PATH_CAP];} path_work;
    path_work *work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));DWORD error=ERROR_NO_UNICODE_TRANSLATION;
    if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    PCWSTR candidates[2]={path,work->short_path};
    DWORD short_count=GetShortPathNameW(path,work->short_path,NTW16S_PATH_CAP);
    for(unsigned i=0;i<(short_count && short_count<NTW16S_PATH_CAP?2u:1u);++i){
        BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        int count=WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,candidates[i],-1,out,(int)capacity,NULL,utf8?NULL:&replaced);
        if(count && !replaced && MultiByteToWideChar(CP_ACP,0,out,-1,work->roundtrip,NTW16S_PATH_CAP) && !wcscmp(work->roundtrip,candidates[i])){error=ERROR_SUCCESS;break;}
    }
    HeapFree(GetProcessHeap(),0,work);return error;
}
DWORD ntw16s_install(HANDLE child,const ntw16s_context *context,ntw16s_create_w create_helper)
{
    typedef struct install_work {BYTE data[65536];ntw16s_context parsed;WCHAR path[NTW16S_PATH_CAP];char dll[4096];} install_work;
    ntw16s_packet header={0};install_work *work;WORD machine=0;DWORD error;
    if(!child || !context || !create_helper)return ERROR_INVALID_PARAMETER;
    size_t length=wcsnlen_s(context->directory,NTW16S_PATH_CAP);
    if(!length || length>=NTW16S_PATH_CAP)return ERROR_INVALID_DATA;
    error=ntw16s_process_machine(child,&machine);if(error)return error;
    work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    header.bytes=(DWORD)(sizeof(header)+(length+1)*sizeof(WCHAR));header.version=1;
    header.machine=machine;header.flags=__atomic_load_n(&context->flags,__ATOMIC_ACQUIRE);header.directory_offset=sizeof(header);
    header.reserved0=(header.flags&NTW16S_POLICY_GLOBAL_SESSION)?
        __atomic_load_n(&context->session_cookie,__ATOMIC_ACQUIRE):0;
    header.directory_bytes=header.bytes-sizeof(header);
    memcpy(work->data,&header,sizeof(header));memcpy(work->data+sizeof(header),context->directory,header.directory_bytes);
    error=ntw16s_packet_parse(work->data,header.bytes,machine,&work->parsed);if(error)goto done;
    if(swprintf_s(work->path,NTW16S_PATH_CAP,L"%ls%lsntw16s%u.dll",context->directory,
        context->directory[length-1]==L'\\'?L"":L"\\",machine==IMAGE_FILE_MACHINE_I386?32:64)<0){error=ERROR_FILENAME_EXCED_RANGE;goto done;}
    error=ntw16s_validate_image(work->path,machine,TRUE);if(error)goto done;
    error=import_path(work->path,work->dll,sizeof(work->dll));if(error)goto done;
    if(machine==(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))
        error=ntw16s_stage_process(child,work->dll,work->data,header.bytes);
    else error=ntw16s_install_helper(child,work->path,work->data,header.bytes,create_helper);
done:
    HeapFree(GetProcessHeap(),0,work);return error;
}
