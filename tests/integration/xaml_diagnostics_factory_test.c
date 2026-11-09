#define COBJMACROS
#define INITGUID
#include "lib-win32/win16-control-xaml/diagnostics_factory.h"
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr,"line %d: %s\n",__LINE__,#expr); return 1; } } while (0)
int main(void)
{
    static const CLSID supported={0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
    static const CLSID other={0x7132a745,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
    const win16_xaml_site_handlers handlers={0}; IClassFactory *factory=NULL;
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_MULTITHREADED)));
    CHECK(win16_xaml_get_class_object(&other,&supported,&handlers,NULL,&IID_IClassFactory,(void **)&factory)==CLASS_E_CLASSNOTAVAILABLE && !factory);
    CHECK(win16_xaml_get_class_object(&supported,&supported,&handlers,NULL,&IID_IClassFactory,(void **)&factory)==S_OK && factory);
    CHECK(IClassFactory_LockServer(factory,TRUE)==E_NOTIMPL);
    IObjectWithSite *site=NULL;
    CHECK(IClassFactory_CreateInstance(factory,(IUnknown *)factory,&IID_IObjectWithSite,(void **)&site)==CLASS_E_NOAGGREGATION && !site);
    CHECK(IClassFactory_CreateInstance(factory,NULL,&IID_IObjectWithSite,(void **)&site)==S_OK && site);
    CHECK(IObjectWithSite_Release(site)==0);
    CHECK(IClassFactory_Release(factory)==0);
    CoUninitialize(); return 0;
}
