/* Native COM multi-selection probe; owns all created files and windows. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static DWORD tid;static volatile LONG seen,selected;static HWND dialog;static BOOL cancel,single_mode;
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
    HWND list=GetDlgItem(w,1120),folders=GetDlgItem(w,1121);if(!list||!folders)return TRUE;
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
        if(seen && IsWindow(dialog) && i>10 && i%10==0){HWND b=GetDlgItem(dialog,cancel?IDCANCEL:IDOK);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(cancel?IDCANCEL:IDOK,BN_CLICKED),(LPARAM)b);}
        if(seen&&!IsWindow(dialog))return 0;
        Sleep(50);}
    PostThreadMessageW(tid,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    cancel=argc>1 && !lstrcmpW(argv[1],L"cancel");
    BOOL spaces=argc>2 && !lstrcmpW(argv[2],L"spaces");
    BOOL links=argc>2 && (!lstrcmpW(argv[2],L"linkfollow")||!lstrcmpW(argv[2],L"linkkeep"));
    BOOL keep=links&&!lstrcmpW(argv[2],L"linkkeep");
    single_mode=argc>3 && !lstrcmpW(argv[3],L"single");
    if(spaces){lstrcpyW(name1,L"alpha long.txt");lstrcpyW(name2,L"beta long.txt");}
    if(links){lstrcpyW(name1,L"alpha.lnk");lstrcpyW(name2,L"beta.lnk");}
    WCHAR temp[MAX_PATH],dir[MAX_PATH],first[MAX_PATH],second[MAX_PATH],actual1[MAX_PATH],actual2[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,dir)||!DeleteFileW(dir)||!CreateDirectoryW(dir,NULL))return 2;
    if(swprintf(first,MAX_PATH,L"%ls\\%ls",dir,name1)<0||swprintf(second,MAX_PATH,L"%ls\\%ls",dir,name2)<0)return 3;
    if(swprintf(actual1,MAX_PATH,L"%ls\\real-one.txt",dir)<0||swprintf(actual2,MAX_PATH,L"%ls\\real-two.txt",dir)<0)return 3;
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
    IShellItem_Release(folder);tid=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 7;
    hr=IFileOpenDialog_Show(d,NULL);DWORD wait=WaitForSingleObject(worker,12000);CloseHandle(worker);
    DWORD count=0;BOOL found1=FALSE,found2=FALSE;IShellItemArray *array=NULL;HRESULT ar=E_FAIL;
    IShellItem *single=NULL;HRESULT single_hr=E_FAIL;
    if(SUCCEEDED(hr)){single_hr=IFileOpenDialog_GetResult(d,&single);if(single)IShellItem_Release(single);}
    if(SUCCEEDED(hr)){ar=IFileOpenDialog_GetResults(d,&array);if(SUCCEEDED(ar))ar=IShellItemArray_GetCount(array,&count);
        for(DWORD i=0;SUCCEEDED(ar)&&i<count;++i){IShellItem *item=NULL;PWSTR name=NULL;
            ar=IShellItemArray_GetItemAt(array,i,&item);if(SUCCEEDED(ar))ar=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&name);
            if(name){if(!_wcsicmp(name,links&&!keep?actual1:first))found1=TRUE;if(!_wcsicmp(name,links&&!keep?actual2:second))found2=TRUE;CoTaskMemFree(name);}if(item)IShellItem_Release(item);}}
    if(array)IShellItemArray_Release(array);
    IFileOpenDialog_Release(d);CoUninitialize();DeleteFileW(first);DeleteFileW(second);if(links){DeleteFileW(actual1);DeleteFileW(actual2);}RemoveDirectoryW(dir);
    printf("COM_MULTI bits=%u seen=%ld selected=%ld cancel=%d hr=%08lx array=%08lx count=%lu paths=%d%d wait=%lu single=%08lx\n",(unsigned)(sizeof(void*)*8),seen,selected,cancel,(unsigned long)hr,(unsigned long)ar,count,found1,found2,wait,(unsigned long)single_hr);
    return seen&&selected==(single_mode?1:2)&&wait==WAIT_OBJECT_0&&(cancel?hr==HRESULT_FROM_WIN32(ERROR_CANCELLED):(SUCCEEDED(hr)&&SUCCEEDED(ar)&&count==(single_mode?1u:2u)&&found1&&(single_mode?!found2:found2)&&(single_mode?SUCCEEDED(single_hr):FAILED(single_hr))))?0:1;
}
