#define _WIN32_WINNT 0x0a00
#define COBJMACROS
#include "print_directory.h"
#include <objbase.h>
#include <oleauto.h>
#include <activeds.h>
#include <dsclient.h>
#include <wchar.h>
#include <initguid.h>
#include <cmnquery.h>
#include <dsquery.h>

static PCWSTR directory_string(const DSOBJECTNAMES *names,SIZE_T bytes,DWORD offset)
{
    if(offset<sizeof(*names) || offset%sizeof(WCHAR) || offset>=bytes)return NULL;
    PCWSTR value=(const WCHAR *)((const BYTE *)names+offset);
    return wmemchr(value,L'\0',(bytes-offset)/sizeof(WCHAR))?value:NULL;
}

static BOOL queue_name(PCWSTR value)
{
    if(value[0]!=L'\\' || value[1]!=L'\\')return FALSE;
    PCWSTR host=value+2,share=wcschr(host,L'\\');
    if(!share || share==host || !share[1] || (share==host+1 && (*host==L'.' || *host==L'?')))return FALSE;
    if(wcspbrk(host,L"/:?*\r\n") || wcschr(share+1,L'\\'))return FALSE;
    return TRUE;
}

static HRESULT resolve_queue(PCWSTR path,PWSTR *queue)
{
    if(_wcsnicmp(path,L"LDAP://",7))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    IADs *object=NULL;
    HRESULT result=ADsOpenObject(path,NULL,NULL,ADS_SECURE_AUTHENTICATION|ADS_READONLY_SERVER,
        &IID_IADs,(void **)&object);
    if(FAILED(result))return result;
    VARIANT value;
    VariantInit(&value);
    BSTR property=SysAllocString(L"uNCName");
    result=property?IADs_Get(object,property,&value):E_OUTOFMEMORY;
    SysFreeString(property);
    if(SUCCEEDED(result)){
        BSTR name=V_VT(&value)==VT_BSTR?V_BSTR(&value):NULL;
        if(!name || wcslen(name)!=SysStringLen(name) || !queue_name(name))result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        else{
            SIZE_T bytes=(wcslen(name)+1)*sizeof(WCHAR);
            *queue=CoTaskMemAlloc(bytes);
            if(*queue)CopyMemory(*queue,name,bytes);
            else result=E_OUTOFMEMORY;
        }
    }
    VariantClear(&value);
    IADs_Release(object);
    return result;
}

static HRESULT selected_queue(IDataObject *data,PWSTR *queue)
{
    CLIPFORMAT format=(CLIPFORMAT)RegisterClipboardFormatW(L"DsObjectNames");
    if(!format)return HRESULT_FROM_WIN32(GetLastError());
    FORMATETC request={format,NULL,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};
    STGMEDIUM medium={0};
    HRESULT result=IDataObject_GetData(data,&request,&medium);
    if(FAILED(result))return result;
    result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if(medium.tymed==TYMED_HGLOBAL){
        SIZE_T size=GlobalSize(medium.hGlobal);
        const DSOBJECTNAMES *names=GlobalLock(medium.hGlobal);
        if(names && size>=sizeof(*names) && names->cItems==1){
            PCWSTR cls=directory_string(names,size,names->aObjects[0].offsetClass);
            PCWSTR path=directory_string(names,size,names->aObjects[0].offsetName);
            if(cls && path && !lstrcmpiW(cls,L"printQueue"))result=resolve_queue(path,queue);
        }
        if(names)GlobalUnlock(medium.hGlobal);
    }
    ReleaseStgMedium(&medium);
    return result;
}

HRESULT win2k_print_directory_select(HWND owner,PWSTR *queue)
{
    *queue=NULL;
    HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(apartment))return apartment;
    ICommonQuery *query=NULL;
    HRESULT result=CoCreateInstance(&CLSID_CommonQuery,NULL,CLSCTX_INPROC_SERVER,&IID_ICommonQuery,(void **)&query);
    if(SUCCEEDED(result)){
        DSQUERYINITPARAMS params={.cbStruct=sizeof(params),.dwFlags=DSQPF_NOSAVE};
        OPENQUERYWINDOW window={.cbStruct=sizeof(window),
            .dwFlags=OQWF_DEFAULTFORM|OQWF_REMOVEFORMS|OQWF_SINGLESELECT|OQWF_OKCANCEL,
            .clsidHandler=CLSID_DsQuery,.pHandlerParameters=&params,.clsidDefaultForm=CLSID_DsFindPrinter};
        IDataObject *data=NULL;
        result=query->lpVtbl->OpenQueryWindow(query,owner,&window,&data);
        if(result==S_OK)result=data?selected_queue(data,queue):E_UNEXPECTED;
        if(data)IDataObject_Release(data);
        query->lpVtbl->Release(query);
    }
    CoUninitialize();
    return result;
}
