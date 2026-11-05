#include "transaction.h"
DWORD win32_process_global_transaction_advance(DWORD *phase,DWORD *control_error,BOOL apply,void *context,const win32_process_global_transaction_ops *ops)
{
    DWORD error;
    if(!phase || !control_error || !ops || !ops->load || !ops->validate || !ops->start || !ops->ready || !ops->apply)return ERROR_INVALID_PARAMETER;
    if(*phase==WIN32_PROCESS_GLOBAL_LOADING){
        error=ops->load(context);if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;if(error)return error;
        error=ops->validate(context);if(error)return error;
        error=ops->start(context);if(error)return error;
        *phase=WIN32_PROCESS_GLOBAL_STARTING;
    }
    if(*phase==WIN32_PROCESS_GLOBAL_STARTING){
        error=ops->ready(context);if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;if(error)return error;
        *phase=WIN32_PROCESS_GLOBAL_ACTIVE;
        if(!apply)return ERROR_SUCCESS;
        *control_error=ops->apply(context);return *control_error;
    }
    return *phase==WIN32_PROCESS_GLOBAL_ACTIVE?*control_error:ERROR_INVALID_STATE;
}
