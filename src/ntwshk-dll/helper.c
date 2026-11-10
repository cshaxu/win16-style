#include "lib-win32/process-image/image.h"
/* Product policy for the generic opposite-width helper. */
#include "staging.h"
#include <wchar.h>
static DWORD apply(HANDLE child,const void *packet,DWORD bytes,PCWSTR dll,void *unused)
{
    typedef struct helper_work {ntwshk_context context;WCHAR expected[NTWSHK_PATH_CAP];} helper_work;
    WORD machine;DWORD error;helper_work *work;(void)unused;
    error=win32_process_image_machine(child,&machine);if(error)return error;
    if(machine!=(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))return ERROR_BAD_EXE_FORMAT;
    work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    error=ntwshk_packet_parse(packet,bytes,machine,&work->context);
    if(!error){
        error=ntwshk_context_library(&work->context,machine,work->expected,NTWSHK_PATH_CAP);
        if(!error && _wcsicmp(work->expected,dll))error=ERROR_INVALID_NAME;
        else if(!error)error=ntwshk_install(child,&work->context,CreateProcessW);
    }
    HeapFree(GetProcessHeap(),0,work);return error;
}
void ntwshk_run_helper(void)
{ExitProcess(process_hook_run_helper(apply,NULL));}
