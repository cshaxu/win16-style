/* Resolve selected Shell items while preserving shortcut policy. */
#define COBJMACROS
#include "lib-style/common/classic_shell_item.h"
#include <shlobj.h>
#include <wchar.h>
#include <stdio.h>

HRESULT style_classic_link_target_id(PCWSTR path,PIDLIST_ABSOLUTE *target)
{
    *target=NULL;const WCHAR *extension=wcsrchr(path,L'.');
    if(!extension || lstrcmpiW(extension,L".lnk"))return S_FALSE;
    IShellLinkW *link=NULL;IPersistFile *persist=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
    if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
    if(SUCCEEDED(hr))hr=IPersistFile_Load(persist,path,STGM_READ);
    if(SUCCEEDED(hr))hr=IShellLinkW_GetIDList(link,target);
    if(persist)IPersistFile_Release(persist);
    if(link)IShellLinkW_Release(link);
    if(FAILED(hr) && *target){CoTaskMemFree(*target);*target=NULL;}
    if(SUCCEEDED(hr) && !*target)return E_FAIL;
    return hr;
}
HRESULT style_classic_open_item(PCWSTR path,BOOL keep_link,IShellItem **item)
{
    PIDLIST_ABSOLUTE target=NULL;
    if(!keep_link && style_classic_link_target_id(path,&target)==S_OK){
        HRESULT hr=SHCreateItemFromIDList(target,&IID_IShellItem,(void **)item);
        CoTaskMemFree(target);return hr;
    }
    return SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)item);
}

/* Shell needs metadata to represent a save target which does not exist yet. */
typedef struct bind_data {IFileSystemBindData iface;LONG refs;WIN32_FIND_DATAW data;} bind_data;
static HRESULT STDMETHODCALLTYPE bind_query(IFileSystemBindData *self,REFIID id,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(!IsEqualIID(id,&IID_IUnknown) && !IsEqualIID(id,&IID_IFileSystemBindData))return E_NOINTERFACE;
    *out=self;InterlockedIncrement(&((bind_data *)self)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE bind_add(IFileSystemBindData *self)
{return (ULONG)InterlockedIncrement(&((bind_data *)self)->refs);}
static ULONG STDMETHODCALLTYPE bind_release(IFileSystemBindData *self)
{LONG refs=InterlockedDecrement(&((bind_data *)self)->refs);if(!refs)HeapFree(GetProcessHeap(),0,self);return (ULONG)refs;}
static HRESULT STDMETHODCALLTYPE bind_set(IFileSystemBindData *self,const WIN32_FIND_DATAW *data)
{if(!data)return E_POINTER;((bind_data *)self)->data=*data;return S_OK;}
static HRESULT STDMETHODCALLTYPE bind_get(IFileSystemBindData *self,WIN32_FIND_DATAW *data)
{if(!data)return E_POINTER;*data=((bind_data *)self)->data;return S_OK;}
static IFileSystemBindDataVtbl bind_vtable={bind_query,bind_add,bind_release,bind_set,bind_get};
HRESULT style_classic_save_item(PCWSTR path,IShellItem **item)
{
    bind_data *data=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*data));if(!data)return E_OUTOFMEMORY;
    data->iface.lpVtbl=&bind_vtable;data->refs=1;data->data.dwFileAttributes=FILE_ATTRIBUTE_NORMAL;
    IBindCtx *ctx=NULL;HRESULT hr=CreateBindCtx(0,&ctx);
    if(SUCCEEDED(hr))hr=IBindCtx_RegisterObjectParam(ctx,L"File System Bind Data",(IUnknown *)&data->iface);
    if(SUCCEEDED(hr))hr=SHCreateItemFromParsingName(path,ctx,&IID_IShellItem,(void **)item);
    if(ctx)IBindCtx_Release(ctx);
    IFileSystemBindData_Release(&data->iface);return hr;
}

HRESULT style_classic_multi_result(WCHAR *file,WORD offset,BOOL keep_link,
    IShellItem **result,IShellItemArray **results,BOOL *completed)
{
    size_t length=wcslen(file);
    if(offset==0 || offset>length || file[offset-1]!=L' '){
        HRESULT hr=style_classic_open_item(file,keep_link,result);
        if(SUCCEEDED(hr))*completed=TRUE;
        return hr;
    }
    file[offset-1]=0;WCHAR *token=file+offset;PIDLIST_ABSOLUTE ids[512]={0};UINT count=0;HRESULT hr=S_OK;
    while(*token){
        while(*token==L' ')++token;
        if(!*token)break;
        WCHAR *end=wcschr(token,L' ');if(end)*end=0;
        if(count==ARRAYSIZE(ids)){hr=HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);break;}
        WCHAR path[32768];size_t dirlen=wcslen(file);int written=swprintf(path,ARRAYSIZE(path),L"%ls%ls%ls",file,
            dirlen && file[dirlen-1]==L'\\'?L"":L"\\",token);
        if(written<0 || (size_t)written>=ARRAYSIZE(path)){hr=HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);break;}
        WCHAR normalized[32768];DWORD n=GetLongPathNameW(path,normalized,ARRAYSIZE(normalized));
        PCWSTR selected=n && n<ARRAYSIZE(normalized)?normalized:path;
        hr=keep_link?S_FALSE:style_classic_link_target_id(selected,&ids[count]);
        if(hr!=S_OK)hr=SHParseDisplayName(selected,NULL,&ids[count],0,NULL);
        if(FAILED(hr))break;
        ++count;if(!end)break;token=end+1;
    }
    if(SUCCEEDED(hr) && count)hr=SHCreateShellItemArrayFromIDLists(count,(PCIDLIST_ABSOLUTE_ARRAY)ids,results);
    else if(SUCCEEDED(hr))hr=E_FAIL;
    for(UINT i=0;i<count;++i)CoTaskMemFree(ids[i]);
    if(SUCCEEDED(hr))*completed=TRUE;
    return hr;
}
