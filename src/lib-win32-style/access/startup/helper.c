#include "lib-win32/process-image/image.h"
/* Product policy for the generic opposite-width helper. */
#include "install.h"
#include <wchar.h>
static DWORD apply(HANDLE child,const void *packet,DWORD bytes,PCWSTR dll,void *context)
{
    typedef struct helper_work {style_startup_context context;WCHAR expected[STYLE_STARTUP_PATH_CAP];} helper_work;
    WORD machine;DWORD error;helper_work *work;PCWSTR dependency=context;
    error=win32_process_image_machine(child,&machine);if(error)return error;
    if(machine!=(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))return ERROR_BAD_EXE_FORMAT;
    work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    error=style_startup_packet_parse(packet,bytes,machine,&work->context);
    if(!error){
        error=style_startup_library(&work->context,machine,work->expected,STYLE_STARTUP_PATH_CAP);
        if(!error && _wcsicmp(work->expected,dll))error=ERROR_INVALID_NAME;
        else if(!error)error=style_startup_install(child,&work->context,dependency,CreateProcessW);
    }
    HeapFree(GetProcessHeap(),0,work);return error;
}
DWORD style_startup_run_helper(PCWSTR dependency)
{return process_hook_run_helper(apply,(void *)dependency);}
