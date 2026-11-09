#define COBJMACROS
#include "diagnostics_subscribe.h"
#include <objbase.h>

typedef struct subscription {
    IStream *service,*callback;
    win16_xaml_subscribe_done done;
    void *context;
} subscription;

static void release_marshaled(IStream *stream)
{
    if (!stream) return;
    LARGE_INTEGER zero={0};
    if (SUCCEEDED(IStream_Seek(stream,zero,STREAM_SEEK_SET,NULL))) (void)CoReleaseMarshalData(stream);
    IStream_Release(stream);
}

static DWORD WINAPI subscribe_worker(void *parameter)
{
    subscription *s=parameter;
    HRESULT initialized=CoInitializeEx(NULL,COINIT_MULTITHREADED),hr=initialized;
    IVisualTreeService *service=NULL;
    IVisualTreeServiceCallback *callback=NULL;
    if (SUCCEEDED(hr)) {
        hr=CoGetInterfaceAndReleaseStream(s->service,&IID_IVisualTreeService,(void **)&service);
        s->service=NULL;
    }
    if (SUCCEEDED(hr)) {
        hr=CoGetInterfaceAndReleaseStream(s->callback,&IID_IVisualTreeServiceCallback,(void **)&callback);
        s->callback=NULL;
    }
    if (SUCCEEDED(hr)) hr=IVisualTreeService_AdviseVisualTreeChange(service,callback);
    s->done(s->context,hr);
    if (callback) IVisualTreeServiceCallback_Release(callback);
    if (service) IVisualTreeService_Release(service);
    release_marshaled(s->callback);
    release_marshaled(s->service);
    HeapFree(GetProcessHeap(),0,s);
    if (SUCCEEDED(initialized)) CoUninitialize();
    return (DWORD)hr;
}

HRESULT win16_xaml_subscribe_async(IVisualTreeService *service,IVisualTreeServiceCallback *callback,
                                   win16_xaml_subscribe_done done,void *context,HANDLE *worker)
{
    if (!service || !callback || !done || !worker) return E_INVALIDARG;
    *worker=NULL;
    subscription *s=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s));
    if (!s) return E_OUTOFMEMORY;
    s->done=done; s->context=context;
    HRESULT hr=CoMarshalInterThreadInterfaceInStream(&IID_IVisualTreeService,(IUnknown *)service,&s->service);
    if (SUCCEEDED(hr)) hr=CoMarshalInterThreadInterfaceInStream(&IID_IVisualTreeServiceCallback,(IUnknown *)callback,&s->callback);
    if (SUCCEEDED(hr)) {
        HANDLE thread=CreateThread(NULL,0,subscribe_worker,s,0,NULL);
        if (thread) { *worker=thread; return S_OK; }
        DWORD error=GetLastError(); hr=HRESULT_FROM_WIN32(error ? error : ERROR_NOT_ENOUGH_MEMORY);
    }
    release_marshaled(s->callback);
    release_marshaled(s->service);
    HeapFree(GetProcessHeap(),0,s);
    return hr;
}
