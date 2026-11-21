/* Adapted from fixed installer.cpp; no service identity or transferred handles. */
#include "install.h"
#include "lib-win32/process-hook/staging.h"
#include "lib-win32/process-image/image.h"
#include <wchar.h>
#include <string.h>
static DWORD import_path(PCWSTR path,char *out,DWORD capacity)
{
    typedef struct path_work {WCHAR short_path[STYLE_STARTUP_PATH_CAP],roundtrip[STYLE_STARTUP_PATH_CAP];} path_work;
    path_work *work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));DWORD error=ERROR_NO_UNICODE_TRANSLATION;
    if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    PCWSTR candidates[2]={path,work->short_path};
    DWORD short_count=GetShortPathNameW(path,work->short_path,STYLE_STARTUP_PATH_CAP);
    for(unsigned i=0;i<(short_count && short_count<STYLE_STARTUP_PATH_CAP?2u:1u);++i){
        BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        int count=WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,candidates[i],-1,out,(int)capacity,NULL,utf8?NULL:&replaced);
        if(count && !replaced && MultiByteToWideChar(CP_ACP,0,out,-1,work->roundtrip,STYLE_STARTUP_PATH_CAP) && !wcscmp(work->roundtrip,candidates[i])){error=ERROR_SUCCESS;break;}
    }
    HeapFree(GetProcessHeap(),0,work);return error;
}
DWORD style_startup_install(HANDLE child,const style_startup_context *context,PCWSTR dependency,process_hook_create_w create_helper)
{
    typedef struct install_work {BYTE data[65536];style_startup_context parsed;WCHAR path[STYLE_STARTUP_PATH_CAP],shared[STYLE_STARTUP_PATH_CAP];char dll[4096],dependency[4096];} install_work;
    style_startup_packet header={0};install_work *work;WORD machine=0;DWORD error;
    const DWORD header_bytes=sizeof(header)+sizeof(style_startup_selection);
    if(!child || !context || !dependency || !create_helper)return ERROR_INVALID_PARAMETER;
    size_t length=wcsnlen_s(context->directory,STYLE_STARTUP_PATH_CAP);
    if(!length || length>=STYLE_STARTUP_PATH_CAP)return ERROR_INVALID_DATA;
    size_t module_length=wcsnlen_s(context->module,STYLE_STARTUP_MODULE_CAP);
    if(!module_length || module_length>=STYLE_STARTUP_MODULE_CAP ||
       header_bytes+(length+module_length+2)*sizeof(WCHAR)>sizeof(work->data))return ERROR_INVALID_DATA;
    error=win32_process_image_machine(child,&machine);if(error)return error;
    work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    header.bytes=(DWORD)(header_bytes+(length+module_length+2)*sizeof(WCHAR));
    DWORD flags=__atomic_load_n(&context->flags,__ATOMIC_ACQUIRE);
    style_startup_selection selection=style_startup_get_selection(flags);
    header.machine=machine;header.flags=flags&(STYLE_STARTUP_POLICY_MASK&~STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE);
    header.directory_offset=header_bytes;
    header.version=STYLE_STARTUP_PACKET_VERSION;
    header.reserved0=(header.flags&STYLE_STARTUP_POLICY_GLOBAL_SESSION)?
        __atomic_load_n(&context->session_cookie,__ATOMIC_ACQUIRE):0;
    header.directory_bytes=(DWORD)((length+1)*sizeof(WCHAR));header.reserved1=(DWORD)((module_length+1)*sizeof(WCHAR));
    memcpy(work->data,&header,sizeof(header));
    memcpy(work->data+sizeof(header),&selection,sizeof(selection));
    memcpy(work->data+header_bytes,context->directory,header.directory_bytes);
    memcpy(work->data+header_bytes+header.directory_bytes,context->module,header.reserved1);
    error=style_startup_packet_parse(work->data,header.bytes,machine,&work->parsed);if(error)goto done;
    error=style_startup_library(context,machine,work->path,STYLE_STARTUP_PATH_CAP);if(error)goto done;
    error=win32_process_image_validate(work->path,machine,TRUE);if(error)goto done;
    error=import_path(work->path,work->dll,sizeof(work->dll));if(error)goto done;
    BOOL unified=!_wcsicmp(context->module,dependency);
    if(!unified){
        error=style_startup_module_path(context,machine,dependency,work->shared,STYLE_STARTUP_PATH_CAP);if(error)goto done;
        error=win32_process_image_validate(work->shared,machine,TRUE);if(error)goto done;
        error=import_path(work->shared,work->dependency,sizeof(work->dependency));if(error)goto done;
    }
    if(machine!=(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))
        error=process_hook_install_helper(child,work->path,work->data,header.bytes,create_helper);
    else if(unified)error=process_hook_stage_process(child,work->dll,work->data,header.bytes);
    else error=process_hook_stage_pair(child,work->dependency,work->dll,work->data,header.bytes);
done:
    HeapFree(GetProcessHeap(),0,work);return error;
}
