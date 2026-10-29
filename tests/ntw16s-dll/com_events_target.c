/* Real COM event observer. The driver owns the target and its dialog. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
typedef struct listener {IFileDialogEvents iface;LONG refs;} listener;
static volatile LONG called,correct,identity,seen,old_layout,kept,ole_window,ole_identity;static DWORD owner;static HWND window;
static WCHAR expected[MAX_PATH];static IUnknown *expected_identity;static BOOL self_unadvise;static DWORD event_cookie;static HRESULT self_result=E_FAIL;
static HRESULT STDMETHODCALLTYPE qi(IFileDialogEvents *p,REFIID id,void **out)
{if(!out)return E_POINTER;*out=NULL;if(!IsEqualIID(id,&IID_IUnknown)&&!IsEqualIID(id,&IID_IFileDialogEvents))return E_NOINTERFACE;*out=p;InterlockedIncrement(&((listener *)p)->refs);return S_OK;}
static ULONG STDMETHODCALLTYPE add(IFileDialogEvents *p){return (ULONG)InterlockedIncrement(&((listener *)p)->refs);}
static ULONG STDMETHODCALLTYPE release(IFileDialogEvents *p){return (ULONG)InterlockedDecrement(&((listener *)p)->refs);}
static HRESULT STDMETHODCALLTYPE ok(IFileDialogEvents *p,IFileDialog *dialog)
{
    (void)p;IUnknown *id=NULL;IShellItem *item=NULL;PWSTR path=NULL;
    if(SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IUnknown,(void **)&id)) && id==expected_identity)InterlockedIncrement(&identity);
    if(SUCCEEDED(IFileDialog_GetResult(dialog,&item)) && SUCCEEDED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path)) && !lstrcmpiW(path,expected))InterlockedIncrement(&correct);
    IOleWindow *ole=NULL;IUnknown *ole_id=NULL;HWND native_window=NULL;
    if(SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IOleWindow,(void **)&ole))){
        if(SUCCEEDED(IOleWindow_GetWindow(ole,&native_window)) && native_window && IsWindow(native_window))InterlockedIncrement(&ole_window);
        if(SUCCEEDED(IOleWindow_QueryInterface(ole,&IID_IUnknown,(void **)&ole_id)) && ole_id==expected_identity)InterlockedIncrement(&ole_identity);
        if(ole_id)IUnknown_Release(ole_id);
        IOleWindow_Release(ole);
    }

    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    if(id)IUnknown_Release(id);
    LONG count=InterlockedIncrement(&called);
    if(self_unadvise){self_result=IFileDialog_Unadvise(dialog,event_cookie);return S_OK;}
    return count==1?S_FALSE:S_OK;
}
static HRESULT STDMETHODCALLTYPE folder_changing(IFileDialogEvents *p,IFileDialog *d,IShellItem *i){(void)p;(void)d;PWSTR path=NULL;HRESULT hr=i?IShellItem_GetDisplayName(i,SIGDN_FILESYSPATH,&path):E_INVALIDARG;printf("EVENT_FOLDER_CHANGING item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");if(path)CoTaskMemFree(path);return S_OK;}
static HRESULT STDMETHODCALLTYPE folder_changed(IFileDialogEvents *p,IFileDialog *d){(void)p;IShellItem *item=NULL;PWSTR path=NULL;HRESULT hr=IFileDialog_GetFolder(d,&item);if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);printf("EVENT_FOLDER_CHANGED item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");if(path)CoTaskMemFree(path);if(item)IShellItem_Release(item);return S_OK;}
static HRESULT STDMETHODCALLTYPE selection_changed(IFileDialogEvents *p,IFileDialog *d){(void)p;IShellItem *item=NULL;PWSTR path=NULL;HRESULT hr=IFileDialog_GetCurrentSelection(d,&item);if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);printf("EVENT_SELECTION item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");if(path)CoTaskMemFree(path);if(item)IShellItem_Release(item);return S_OK;}
static HRESULT STDMETHODCALLTYPE type_changed(IFileDialogEvents *p,IFileDialog *d){(void)p;UINT type=0;HRESULT hr=IFileDialog_GetFileTypeIndex(d,&type);printf("EVENT_TYPE hr=%08lx index=%u\n",(unsigned long)hr,type);return S_OK;}
static HRESULT STDMETHODCALLTYPE share(IFileDialogEvents *p,IFileDialog *d,IShellItem *i,FDE_SHAREVIOLATION_RESPONSE *r){(void)p;(void)d;(void)i;*r=FDESVR_DEFAULT;return S_OK;}
static HRESULT STDMETHODCALLTYPE overwrite(IFileDialogEvents *p,IFileDialog *d,IShellItem *i,FDE_OVERWRITE_RESPONSE *r){(void)p;(void)d;(void)i;*r=FDEOR_DEFAULT;return S_OK;}
static IFileDialogEventsVtbl table={qi,add,release,ok,folder_changing,folder_changed,selection_changed,share,type_changed,overwrite};
static BOOL CALLBACK scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR title[100];GetWindowTextW(w,title,ARRAYSIZE(title));
    if(IsWindowVisible(w)&&!lstrcmpW(title,L"Win16S COM event")){window=w;old_layout=GetDlgItem(w,1121)!=NULL;InterlockedExchange(&seen,1);return FALSE;}
    return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;BOOL first=FALSE,second=FALSE;
    for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(owner,scan,0);
        if(seen && IsWindow(window) && i>10 && !first){HWND b=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)b);first=TRUE;}
        if(first && !second && called>=1 && IsWindow(window)){InterlockedExchange(&kept,1);HWND b=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)b);second=TRUE;}
        if(first&&!IsWindow(window))return 0;
        Sleep(50);}
    PostThreadMessageW(owner,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    BOOL removed=argc>1&&!lstrcmpW(argv[1],L"remove");
    self_unadvise=argc>1&&!lstrcmpW(argv[1],L"self");
    BOOL require_old=removed || (argc>1&&!lstrcmpW(argv[1],L"old")),release_only=argc>1&&!lstrcmpW(argv[1],L"release");WCHAR temp[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,expected))return 2;
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 3;
    IFileDialog *dialog=NULL;HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,L"Win16S COM event"))||FAILED(IFileDialog_SetFileName(dialog,expected))||
       FAILED(IFileDialog_QueryInterface(dialog,&IID_IUnknown,(void **)&expected_identity)))return 4;
    listener events={0};events.iface.lpVtbl=&table;events.refs=1;DWORD cookie=0;
    if(FAILED(IFileDialog_Advise(dialog,&events.iface,&cookie)))return 5;
    event_cookie=cookie;
    if(removed && FAILED(IFileDialog_Unadvise(dialog,cookie)))return 5;
    owner=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 6;
    hr=IFileDialog_Show(dialog,NULL);DWORD wait=WaitForSingleObject(worker,10000);CloseHandle(worker);
    HRESULT unadvise=(release_only||removed)?S_OK:self_unadvise?self_result:IFileDialog_Unadvise(dialog,cookie);IUnknown_Release(expected_identity);IFileDialog_Release(dialog);CoUninitialize();DeleteFileW(expected);
    printf("COM_EVENTS bits=%u seen=%ld old=%ld calls=%ld correct=%ld identity=%ld kept=%ld hr=%08lx unadvise=%08lx refs=%ld wait=%lu ole=%ld ole_identity=%ld\n",
        (unsigned)(sizeof(void*)*8),seen,old_layout,called,correct,identity,kept,(unsigned long)hr,(unsigned long)unadvise,events.refs,wait,ole_window,ole_identity);
    return seen&&(!require_old||old_layout)&&(removed?(called==0&&correct==0&&identity==0):
        (self_unadvise?(called==1&&correct==1&&identity==1&&ole_window==1&&ole_identity==1):(called==2&&correct==2&&identity==2&&ole_window==2&&ole_identity==2&&kept)))&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&events.refs==1&&wait==WAIT_OBJECT_0?0:1;
}
