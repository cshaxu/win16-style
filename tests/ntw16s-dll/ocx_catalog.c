#define COBJMACROS
#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <stdio.h>

typedef HRESULT (STDAPICALLTYPE *dll_get_class_object_fn)(REFCLSID,REFIID,LPVOID *);

static void print_hr(const WCHAR *name,UINT index,REFGUID guid,const WCHAR *kind,HRESULT factory,HRESULT create,HRESULT ole,HRESULT misc)
{
    WCHAR text[64];StringFromGUID2(guid,text,ARRAYSIZE(text));
    wprintf(L"OCX_CATALOG name=%ls index=%u guid=%ls kind=%ls factory=%08lx create=%08lx ole=%08lx misc=%08lx\n",
        name?name:L"",index,text,kind,(DWORD)factory,(DWORD)create,(DWORD)ole,(DWORD)misc);
}

int wmain(int argc,WCHAR **argv)
{
    ITypeLib *library=NULL;HMODULE module=NULL;dll_get_class_object_fn get_class=NULL;HRESULT hr;
    union {FARPROC raw;dll_get_class_object_fn typed;} procedure={0};
    UINT count,visible=0,classes=0;
    if(argc!=2){fwprintf(stderr,L"usage: core-ocx-catalog <explicit-ocx-path>\n");return 64;}
    hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(hr)){fwprintf(stderr,L"OCX_CATALOG_FAIL coinit=%08lx\n",(DWORD)hr);return 1;}
    hr=LoadTypeLibEx(argv[1],REGKIND_NONE,&library);
    if(FAILED(hr)){fwprintf(stderr,L"OCX_CATALOG_FAIL typelib=%08lx\n",(DWORD)hr);CoUninitialize();return 1;}
    module=LoadLibraryW(argv[1]);procedure.raw=module?GetProcAddress(module,"DllGetClassObject"):NULL;get_class=procedure.typed;
    if(!module || !get_class){fwprintf(stderr,L"OCX_CATALOG_FAIL module=%lu export=%u\n",GetLastError(),get_class!=NULL);if(module)FreeLibrary(module);ITypeLib_Release(library);CoUninitialize();return 1;}
    count=ITypeLib_GetTypeInfoCount(library);
    for(UINT i=0;i<count;++i){
        ITypeInfo *info=NULL;TYPEATTR *attributes=NULL;TYPEKIND type;BSTR name=NULL;
        IClassFactory *factory=NULL;IUnknown *unknown=NULL;IOleObject *object=NULL;
        HRESULT factory_hr=E_NOINTERFACE,create_hr=E_NOINTERFACE,ole_hr=E_NOINTERFACE,misc_hr=E_NOINTERFACE;
        if(FAILED(ITypeLib_GetTypeInfoType(library,i,&type)) || type!=TKIND_COCLASS ||
            FAILED(ITypeLib_GetTypeInfo(library,i,&info)) || FAILED(ITypeInfo_GetTypeAttr(info,&attributes))){if(info)ITypeInfo_Release(info);continue;}
        ++classes;ITypeLib_GetDocumentation(library,i,&name,NULL,NULL,NULL);
        factory_hr=get_class(&attributes->guid,&IID_IClassFactory,(void **)&factory);
        if(SUCCEEDED(factory_hr)){
            create_hr=IClassFactory_CreateInstance(factory,NULL,&IID_IUnknown,(void **)&unknown);
            if(SUCCEEDED(create_hr)){
                ole_hr=IUnknown_QueryInterface(unknown,&IID_IOleObject,(void **)&object);
                if(SUCCEEDED(ole_hr)){
                    DWORD flags=0;misc_hr=IOleObject_GetMiscStatus(object,DVASPECT_CONTENT,&flags);
                    if(SUCCEEDED(misc_hr) && !(flags&OLEMISC_INVISIBLEATRUNTIME))++visible;
                }
            }
        }
        print_hr(name,i,&attributes->guid,L"coclass",factory_hr,create_hr,ole_hr,misc_hr);
        if(object)IOleObject_Release(object);
        if(unknown)IUnknown_Release(unknown);
        if(factory)IClassFactory_Release(factory);
        if(name)SysFreeString(name);
        ITypeInfo_ReleaseTypeAttr(info,attributes);
        ITypeInfo_Release(info);
    }
    wprintf(L"OCX_CATALOG_RESULT file=%ls type_infos=%u coclasses=%u visible_ole=%u\n",argv[1],count,classes,visible);
    FreeLibrary(module);ITypeLib_Release(library);CoUninitialize();return 0;
}
