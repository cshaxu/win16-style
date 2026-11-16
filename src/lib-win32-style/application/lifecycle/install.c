#include "install.h"

BOOL style_instance_install(style_instance *instance,HWND window,SUBCLASSPROC procedure,
    const void *owner,const style_instance_install_ops *ops)
{
    DWORD error;style_instance_retain(instance);
    if(ops && ops->cancelled && *ops->cancelled){error=ERROR_CANCELLED;goto unpublished;}
    if(!style_instance_attach(instance,window,procedure,owner)){error=GetLastError();goto unpublished;}
    if(ops && ops->setup && !ops->setup(window,ops->context)){
        error=GetLastError();
        if(!instance->detached && style_instance_lookup(window,procedure)==instance){
            if(ops->rollback)ops->rollback(window,ops->context);
        }
        goto failed;
    }
    if(instance->detached){
        error=ops && ops->cancelled && *ops->cancelled?ERROR_CANCELLED:ERROR_INVALID_WINDOW_HANDLE;
        goto failed;
    }
    style_instance_release(instance);return TRUE;
unpublished:
    instance->detached=TRUE;
failed:
    style_instance_release(instance);SetLastError(error);return FALSE;
}
