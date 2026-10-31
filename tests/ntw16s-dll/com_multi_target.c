/* Native COM multi-selection probe; owns all created files and windows. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static DWORD tid;static volatile LONG seen,selected,event_veto_seen;static HWND dialog;static BOOL cancel,single_mode,modern_mode,veto_cancel,saw_modern;
typedef struct multi_sink {IFileDialogEvents iface;LONG refs,calls,counts,paths,selection_calls,selection_valid;} multi_sink;
static WCHAR event_first[MAX_PATH],event_second[MAX_PATH],selection_first[MAX_PATH],selection_second[MAX_PATH];
static HRESULT STDMETHODCALLTYPE event_query(IFileDialogEvents *self,REFIID iid,void **out)
{if(!out)return E_POINTER;*out=NULL;if(!IsEqualIID(iid,&IID_IUnknown)&&!IsEqualIID(iid,&IID_IFileDialogEvents))return E_NOINTERFACE;*out=self;InterlockedIncrement(&((multi_sink *)self)->refs);return S_OK;}
static ULONG STDMETHODCALLTYPE event_add(IFileDialogEvents *self){return (ULONG)InterlockedIncrement(&((multi_sink *)self)->refs);}
static ULONG STDMETHODCALLTYPE event_release(IFileDialogEvents *self){return (ULONG)InterlockedDecrement(&((multi_sink *)self)->refs);}
static HRESULT STDMETHODCALLTYPE event_ok(IFileDialogEvents *self,IFileDialog *dialog)
{multi_sink *sink=(multi_sink *)self;IFileOpenDialog *open=NULL;IShellItemArray *items=NULL;DWORD count=0;
 HRESULT hr=IFileDialog_QueryInterface(dialog,&IID_IFileOpenDialog,(void **)&open);
 if(SUCCEEDED(hr))hr=IFileOpenDialog_GetResults(open,&items);
 if(SUCCEEDED(hr))hr=IShellItemArray_GetCount(items,&count);
 if(SUCCEEDED(hr)&&count==(single_mode?1u:2u)){BOOL first=FALSE,second=FALSE;++sink->counts;
    for(DWORD n=0;n<count;++n){IShellItem *item=NULL;PWSTR path=NULL;
        if(SUCCEEDED(IShellItemArray_GetItemAt(items,n,&item)) && SUCCEEDED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path))){
            if(!_wcsicmp(path,event_first))first=TRUE;
            if(!_wcsicmp(path,event_second))second=TRUE;}
        if(path)CoTaskMemFree(path);
        if(item)IShellItem_Release(item);}
    if(first&&(single_mode?!second:second))++sink->paths;}
 if(items)IShellItemArray_Release(items);
 if(open)IFileOpenDialog_Release(open);
 printf("MULTI_EVENT_OK hr=%08lx count=%lu\n",(unsigned long)hr,count);
 if(++sink->calls==1){InterlockedExchange(&event_veto_seen,1);return S_FALSE;}return S_OK;}
static HRESULT STDMETHODCALLTYPE event_changing(IFileDialogEvents *s,IFileDialog *d,IShellItem *i){(void)s;(void)d;(void)i;return S_OK;}
static HRESULT STDMETHODCALLTYPE event_simple(IFileDialogEvents *s,IFileDialog *d){(void)s;(void)d;return S_OK;}
static HRESULT STDMETHODCALLTYPE event_selection(IFileDialogEvents *s,IFileDialog *d)
{multi_sink *sink=(multi_sink *)s;IShellItem *item=NULL;PWSTR path=NULL;HRESULT hr=IFileDialog_GetCurrentSelection(d,&item);
 if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
 ++sink->selection_calls;if(SUCCEEDED(hr)&&path&&(!_wcsicmp(path,selection_first)||!_wcsicmp(path,selection_second)))++sink->selection_valid;
 printf("MULTI_SELECTION hr=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");
 if(path)CoTaskMemFree(path);
 if(item)IShellItem_Release(item);
 return S_OK;}
static HRESULT STDMETHODCALLTYPE event_share(IFileDialogEvents *s,IFileDialog *d,IShellItem *i,FDE_SHAREVIOLATION_RESPONSE *r){(void)s;(void)d;(void)i;*r=FDESVR_DEFAULT;return S_OK;}
static HRESULT STDMETHODCALLTYPE event_overwrite(IFileDialogEvents *s,IFileDialog *d,IShellItem *i,FDE_OVERWRITE_RESPONSE *r){(void)s;(void)d;(void)i;*r=FDEOR_DEFAULT;return S_OK;}
static IFileDialogEventsVtbl event_vtable={event_query,event_add,event_release,event_ok,event_changing,event_simple,event_selection,event_share,event_simple,event_overwrite};
static WCHAR name1[64]=L"first.txt",name2[64]=L"second.txt",short1[64],short2[64];
static const WCHAR title[]=L"Win16S COM multi select";
static BOOL create_link(PCWSTR path,PCWSTR target)
{
    IShellLinkW *link=NULL;IPersistFile *persist=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
    if(SUCCEEDED(hr))hr=IShellLinkW_SetPath(link,target);
    if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
    if(SUCCEEDED(hr))hr=IPersistFile_Save(persist,path,TRUE);
    if(persist)IPersistFile_Release(persist);
    if(link)IShellLinkW_Release(link);
    return SUCCEEDED(hr);
}
static BOOL CALLBACK find(HWND w,LPARAM arg)
{
    (void)arg;WCHAR text[100];GetWindowTextW(w,text,100);
    if(!IsWindowVisible(w)||lstrcmpW(text,title))return TRUE;
    HWND list=GetDlgItem(w,1120),folders=GetDlgItem(w,1121);
    if(!list||!folders){if(modern_mode){saw_modern=TRUE;dialog=w;InterlockedExchange(&selected,single_mode?1:2);InterlockedExchange(&seen,1);return FALSE;}return TRUE;}
    static BOOL reported;LRESULT count=SendMessageW(list,LB_GETCOUNT,0,0);int matched=0;
    if(!reported){WCHAR folder[512]=L"",file[512]=L"";GetDlgItemTextW(w,1088,folder,512);GetDlgItemTextW(w,1152,file,512);
        printf("MULTI_INITIAL count=%lld folder=%ls file=%ls style=%08lx\n",(long long)count,folder,file,(unsigned long)GetWindowLongPtrW(list,GWL_STYLE));reported=TRUE;}
    for(LRESULT i=0;i<count;++i){WCHAR name[512]=L"";if(SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0)>=511)continue;
        if(SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)name)==LB_ERR)continue;
        if(!_wcsicmp(name,name1)||!_wcsicmp(name,short1)||(!single_mode && (!_wcsicmp(name,name2)||!_wcsicmp(name,short2)))){
            if(SendMessageW(list,LB_SETSEL,TRUE,i)!=LB_ERR)++matched;}}
    if(matched==(single_mode?1:2)){SendMessageW(w,WM_COMMAND,MAKEWPARAM(1120,LBN_SELCHANGE),(LPARAM)list);
        printf("MULTI_LIST count=%lld selected=%d\n",(long long)count,matched);dialog=w;InterlockedExchange(&selected,matched);InterlockedExchange(&seen,1);}
    return FALSE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;for(unsigned i=0;i<200;++i){if(!seen)EnumThreadWindows(tid,find,0);
        if(seen && IsWindow(dialog) && i>10 && i%10==0){BOOL cancel_now=cancel||(veto_cancel&&event_veto_seen);
            HWND b=GetDlgItem(dialog,cancel_now?IDCANCEL:IDOK);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(cancel_now?IDCANCEL:IDOK,BN_CLICKED),(LPARAM)b);}
        if(seen&&!IsWindow(dialog))return 0;
        Sleep(50);}
    PostThreadMessageW(tid,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc>7 || (argc==7 && (!SetEnvironmentVariableW(L"TEMP",argv[6]) || !SetEnvironmentVariableW(L"TMP",argv[6]))))return 2;
    cancel=argc>1 && !lstrcmpW(argv[1],L"cancel");
    veto_cancel=argc>1 && !lstrcmpW(argv[1],L"vetocancel");
    BOOL spaces=argc>2 && !lstrcmpW(argv[2],L"spaces");
    BOOL links=argc>2 && (!lstrcmpW(argv[2],L"linkfollow")||!lstrcmpW(argv[2],L"linkkeep"));
    BOOL keep=links&&!lstrcmpW(argv[2],L"linkkeep");
    single_mode=argc>3 && !lstrcmpW(argv[3],L"single");
    BOOL event_mode=argc>4 && !lstrcmpW(argv[4],L"events");
    modern_mode=argc>5 && !lstrcmpW(argv[5],L"modern");
    if(spaces){lstrcpyW(name1,L"alpha long.txt");lstrcpyW(name2,L"beta long.txt");}
    if(links){lstrcpyW(name1,L"alpha.lnk");lstrcpyW(name2,L"beta.lnk");}
    WCHAR temp[MAX_PATH],dir[MAX_PATH],first[MAX_PATH],second[MAX_PATH],actual1[MAX_PATH],actual2[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,dir)||!DeleteFileW(dir)||!CreateDirectoryW(dir,NULL))return 2;
    if(swprintf(first,MAX_PATH,L"%ls\\%ls",dir,name1)<0||swprintf(second,MAX_PATH,L"%ls\\%ls",dir,name2)<0)return 3;
    lstrcpynW(event_first,first,MAX_PATH);lstrcpynW(event_second,second,MAX_PATH);
    lstrcpynW(selection_first,first,MAX_PATH);lstrcpynW(selection_second,second,MAX_PATH);
    if(swprintf(actual1,MAX_PATH,L"%ls\\real-one.txt",dir)<0||swprintf(actual2,MAX_PATH,L"%ls\\real-two.txt",dir)<0)return 3;
    if(links&&!keep){lstrcpynW(event_first,actual1,MAX_PATH);lstrcpynW(event_second,actual2,MAX_PATH);}
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 5;
    HANDLE f=CreateFileW(links?actual1:first,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);
    f=CreateFileW(links?actual2:second,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);
    if(links&&(!create_link(first,actual1)||!create_link(second,actual2)))return 4;
    WCHAR short_path[MAX_PATH];DWORD n=GetShortPathNameW(first,short_path,MAX_PATH);WCHAR *leaf=n && n<MAX_PATH?wcsrchr(short_path,L'\\'):NULL;
    lstrcpynW(short1,leaf?leaf+1:name1,ARRAYSIZE(short1));n=GetShortPathNameW(second,short_path,MAX_PATH);leaf=n && n<MAX_PATH?wcsrchr(short_path,L'\\'):NULL;
    lstrcpynW(short2,leaf?leaf+1:name2,ARRAYSIZE(short2));
    printf("MULTI_NAMES spaces=%d first=%ls second=%ls\n",spaces,short1,short2);
    IFileOpenDialog *d=NULL;HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileOpenDialog,(void **)&d);
    IShellItem *folder=NULL;FILEOPENDIALOGOPTIONS options=0;
    if(FAILED(hr)||FAILED(IFileOpenDialog_SetTitle(d,title))||FAILED(IFileOpenDialog_GetOptions(d,&options))||
       FAILED(IFileOpenDialog_SetOptions(d,options|FOS_ALLOWMULTISELECT|FOS_FORCEFILESYSTEM|(keep?FOS_NODEREFERENCELINKS:0)))||
       FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&folder))||FAILED(IFileOpenDialog_SetFolder(d,folder)))return 6;
    if(modern_mode){WCHAR input[MAX_PATH*2+8];
        int written=single_mode?swprintf(input,ARRAYSIZE(input),L"\"%ls\"",first):swprintf(input,ARRAYSIZE(input),L"\"%ls\" \"%ls\"",first,second);
        if(written<0 || (size_t)written>=ARRAYSIZE(input) || FAILED(IFileOpenDialog_SetFileName(d,input)))return 6;}
    multi_sink sink={0};sink.iface.lpVtbl=&event_vtable;sink.refs=1;DWORD cookie=0;
    if(event_mode && FAILED(IFileOpenDialog_Advise(d,&sink.iface,&cookie)))return 7;
    IShellItem_Release(folder);tid=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 7;
    hr=IFileOpenDialog_Show(d,NULL);DWORD wait=WaitForSingleObject(worker,12000),worker_exit=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&worker_exit);CloseHandle(worker);
    DWORD count=0;BOOL found1=FALSE,found2=FALSE;IShellItemArray *array=NULL;HRESULT ar=E_FAIL;
    IShellItem *single=NULL;HRESULT single_hr=E_FAIL;
    if(SUCCEEDED(hr)||veto_cancel){single_hr=IFileOpenDialog_GetResult(d,&single);if(single)IShellItem_Release(single);}
    if(SUCCEEDED(hr)||veto_cancel){ar=IFileOpenDialog_GetResults(d,&array);if(SUCCEEDED(ar))ar=IShellItemArray_GetCount(array,&count);
        for(DWORD i=0;SUCCEEDED(ar)&&i<count;++i){IShellItem *item=NULL;PWSTR name=NULL;
            ar=IShellItemArray_GetItemAt(array,i,&item);if(SUCCEEDED(ar))ar=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&name);
            if(name){if(!_wcsicmp(name,links&&!keep?actual1:first))found1=TRUE;if(!_wcsicmp(name,links&&!keep?actual2:second))found2=TRUE;CoTaskMemFree(name);}if(item)IShellItem_Release(item);}}
    if(array)IShellItemArray_Release(array);
    if(event_mode)IFileOpenDialog_Unadvise(d,cookie);
    IFileOpenDialog_Release(d);CoUninitialize();BOOL cwd_reset=SetCurrentDirectoryW(temp);
    BOOL removed1=DeleteFileW(first),removed2=DeleteFileW(second),real1=TRUE,real2=TRUE;
    if(links){real1=DeleteFileW(actual1);real2=DeleteFileW(actual2);}
    BOOL removed_dir=RemoveDirectoryW(dir),cleanup=cwd_reset&&removed1&&removed2&&real1&&real2&&removed_dir;
    printf("MULTI_CLEANUP cwd=%d first=%d second=%d targets=%d,%d dir=%d\n",cwd_reset,removed1,removed2,real1,real2,removed_dir);
    if(event_mode)printf("MULTI_EVENTS calls=%ld counts=%ld paths=%ld refs=%ld selections=%ld valid=%ld\n",sink.calls,sink.counts,sink.paths,sink.refs,sink.selection_calls,sink.selection_valid);
    printf("COM_MULTI bits=%u seen=%ld selected=%ld cancel=%d hr=%08lx array=%08lx count=%lu paths=%d%d wait=%lu worker=%lu single=%08lx\n",(unsigned)(sizeof(void*)*8),seen,selected,cancel,(unsigned long)hr,(unsigned long)ar,count,found1,found2,wait,worker_exit,(unsigned long)single_hr);
    printf("COM_MULTI_LAYOUT modern=%d\n",saw_modern);
    BOOL sink_ok=!event_mode||(sink.refs==1&&sink.selection_calls>0&&sink.selection_calls==sink.selection_valid&&
        (cancel?sink.calls==0:veto_cancel?(sink.calls==1&&sink.counts==1&&sink.paths==1):(sink.calls==2&&sink.counts==2&&sink.paths==2)));
    BOOL result_ok=(cancel||veto_cancel)?(hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)&&(!veto_cancel||(FAILED(ar)&&FAILED(single_hr)))):
        (SUCCEEDED(hr)&&SUCCEEDED(ar)&&count==(single_mode?1u:2u)&&found1&&(single_mode?!found2:found2)&&(single_mode?SUCCEEDED(single_hr):single_hr==E_UNEXPECTED));
    return cleanup&&seen&&(!modern_mode || saw_modern)&&selected==(single_mode?1:2)&&wait==WAIT_OBJECT_0&&worker_exit==0&&sink_ok&&result_ok?0:1;
}
