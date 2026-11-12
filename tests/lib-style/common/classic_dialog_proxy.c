#define COBJMACROS
#include "lib-style/common/classic_dialog_proxy_dispatch.h"

static HRESULT dispatch(void *owner,style_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{(void)owner;(void)op;(void)a;(void)b;return E_NOTIMPL;}
static HRESULT show(style_classic_dialog_proxy *proxy,HWND owner)
{return proxy && !owner?S_OK:E_FAIL;}
int main(void)
{
    HRESULT initialized=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized))return 1;
    IUnknown *native=NULL,*identity=NULL;IOleWindow *ole=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IUnknown,(void **)&native);
    if(FAILED(hr)){CoUninitialize();return 2;}
    style_classic_dialog_proxy *proxy=style_classic_dialog_proxy_new(native,FALSE,dispatch,FALSE);
    if(!proxy){IUnknown_Release(native);CoUninitialize();return 3;}
    const style_classic_dialog_proxy_dispatch_services services={.show=show};
    int result=0;
    if(style_classic_dialog_proxy_dispatch(proxy,STYLE_DIALOG_V2_SHOW,0,0,&services,FALSE)!=S_OK)result=4;
    else if(FAILED(style_classic_dialog_proxy_query(proxy,&IID_IUnknown,(void **)&identity)) || (void *)identity!=(void *)proxy)result=5;
    else if(FAILED(style_classic_dialog_proxy_query(proxy,&IID_IOleWindow,(void **)&ole)))result=6;
    if(ole)IOleWindow_Release(ole);
    if(identity)IUnknown_Release(identity);
    if(style_classic_dialog_proxy_release(proxy)!=0 && !result)result=7;
    IUnknown_Release(native);CoUninitialize();return result;
}
