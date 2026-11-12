#define COBJMACROS
#include "lib-style/common/dialog_primary.h"

static HRESULT query(void *owner, REFIID id, void **out)
{ (void)id; *out=owner; return S_OK; }
static ULONG ref(void *owner) { return owner ? 2 : 0; }
static HRESULT dispatch(void *owner, style_dialog_v2_operation op, ULONG_PTR a, ULONG_PTR b)
{ return owner && op==STYLE_DIALOG_V2_SHOW && a==42 && b==0 ? S_FALSE : E_FAIL; }
int main(void)
{
    int owner=1; BOOL unsupported=FALSE; WCHAR *label=NULL;
    const style_dialog_identity_services identity={query,ref,ref};
    style_dialog_v2 v2; style_dialog_v2_init(&v2,&owner,NULL,&unsupported,&label,&identity,dispatch);
    if (IFileDialog2_Show(&v2.iface,(HWND)(ULONG_PTR)42)!=S_FALSE) return 1;
    if (IFileDialog2_AddRef(&v2.iface)!=2 || IFileDialog2_Release(&v2.iface)!=2) return 2;
    style_dialog_primary primary; style_dialog_primary_init(&primary,&owner,NULL,&unsupported,FALSE,&identity,dispatch);
    IFileOpenDialog *open=(IFileOpenDialog *)&primary;
    if (IFileOpenDialog_Show(open,(HWND)(ULONG_PTR)42)!=S_FALSE) return 3;
    return unsupported ? 4 : 0;
}
