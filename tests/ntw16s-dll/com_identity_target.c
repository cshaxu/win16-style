/* Exercise direct creation and identity across common COM interfaces. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shobjidl.h>
#include <ocidl.h>
#include <stdio.h>
int wmain(void)
{
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 2;
    IFileDialogCustomize *custom=NULL;IFileDialog *base=NULL;IUnknown *a=NULL,*b=NULL;
    hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialogCustomize,(void **)&custom);
    if(SUCCEEDED(hr))hr=IFileDialogCustomize_QueryInterface(custom,&IID_IFileDialog,(void **)&base);
    if(SUCCEEDED(hr))hr=IFileDialogCustomize_QueryInterface(custom,&IID_IUnknown,(void **)&a);
    if(SUCCEEDED(hr))hr=IFileDialog_QueryInterface(base,&IID_IUnknown,(void **)&b);
    BOOL identity=SUCCEEDED(hr) && a==b;
    printf("COM_CUSTOM_IDENTITY bits=%u hr=%08lx same=%d\n",(unsigned)(sizeof(void*)*8),(unsigned long)hr,identity);
    if(b)IUnknown_Release(b);
    if(a)IUnknown_Release(a);
    if(base)IFileDialog_Release(base);
    if(custom)IFileDialogCustomize_Release(custom);
    IFileDialog2 *v2=NULL;IFileDialog *v2base=NULL;IUnknown *v2a=NULL,*v2b=NULL;
    HRESULT vhr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog2,(void **)&v2);
    if(SUCCEEDED(vhr))vhr=IFileDialog2_QueryInterface(v2,&IID_IFileDialog,(void **)&v2base);
    if(SUCCEEDED(vhr))vhr=IFileDialog2_QueryInterface(v2,&IID_IUnknown,(void **)&v2a);
    if(SUCCEEDED(vhr))vhr=IFileDialog_QueryInterface(v2base,&IID_IUnknown,(void **)&v2b);
    BOOL v2same=SUCCEEDED(vhr) && v2a==v2b;
    printf("COM_V2_IDENTITY bits=%u hr=%08lx same=%d\n",(unsigned)(sizeof(void*)*8),(unsigned long)vhr,v2same);
    if(v2b)IUnknown_Release(v2b);
    if(v2a)IUnknown_Release(v2a);
    if(v2base)IFileDialog_Release(v2base);
    if(v2)IFileDialog2_Release(v2);
    IClassFactory *factory=NULL;IUnknown *marshal=NULL,*factory2=NULL;
    HRESULT fh=CoGetClassObject(&CLSID_FileOpenDialog,CLSCTX_INPROC_SERVER,NULL,&IID_IClassFactory,(void **)&factory);
    HRESULT mh=FAILED(fh)?fh:IClassFactory_QueryInterface(factory,&IID_IMarshal,(void **)&marshal);
    HRESULT f2h=FAILED(fh)?fh:IClassFactory_QueryInterface(factory,&IID_IClassFactory2,(void **)&factory2);
    printf("COM_FACTORY_EXTRA bits=%u marshal=%08lx factory2=%08lx\n",(unsigned)(sizeof(void*)*8),(unsigned long)mh,(unsigned long)f2h);
    if(marshal)IUnknown_Release(marshal);
    if(factory2)IUnknown_Release(factory2);
    if(factory)IClassFactory_Release(factory);
    IUnknown *other=NULL;HRESULT other_hr=CoCreateInstance(&CLSID_StdGlobalInterfaceTable,NULL,CLSCTX_INPROC_SERVER,&IID_IUnknown,(void **)&other);
    if(other)IUnknown_Release(other);
    printf("COM_UNRELATED bits=%u hr=%08lx\n",(unsigned)(sizeof(void*)*8),(unsigned long)other_hr);
    CoUninitialize();return identity && v2same && SUCCEEDED(other_hr)?0:1;
}
