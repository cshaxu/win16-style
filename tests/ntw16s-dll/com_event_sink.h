#ifndef TEST_COM_EVENT_SINK_H
#define TEST_COM_EVENT_SINK_H

typedef struct {IFileDialogEvents iface;LONG refs;} sink;
static HRESULT STDMETHODCALLTYPE qi(IFileDialogEvents *s,REFIID id,void **out){if(!out)return E_POINTER;*out=NULL;if(!IsEqualIID(id,&IID_IUnknown)&&!IsEqualIID(id,&IID_IFileDialogEvents))return E_NOINTERFACE;*out=s;InterlockedIncrement(&((sink *)s)->refs);return S_OK;}
static ULONG STDMETHODCALLTYPE add(IFileDialogEvents *s){return (ULONG)InterlockedIncrement(&((sink *)s)->refs);}
static ULONG STDMETHODCALLTYPE rel(IFileDialogEvents *s){return (ULONG)InterlockedDecrement(&((sink *)s)->refs);}
static HRESULT STDMETHODCALLTYPE ok(IFileDialogEvents *s,IFileDialog *d){(void)s;(void)d;return S_OK;}
static HRESULT STDMETHODCALLTYPE simple(IFileDialogEvents *s,IFileDialog *d){(void)s;(void)d;return S_OK;}
static HRESULT STDMETHODCALLTYPE share(IFileDialogEvents *s,IFileDialog *d,IShellItem *i,FDE_SHAREVIOLATION_RESPONSE *r){(void)s;(void)d;(void)i;*r=FDESVR_DEFAULT;return S_OK;}
static HRESULT STDMETHODCALLTYPE overwrite(IFileDialogEvents *s,IFileDialog *d,IShellItem *i,FDE_OVERWRITE_RESPONSE *r){(void)s;(void)d;(void)i;*r=FDEOR_DEFAULT;return S_OK;}

#endif
