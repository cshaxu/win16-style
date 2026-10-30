#define COBJMACROS
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <wchar.h>
static WCHAR root[MAX_PATH],hidden[MAX_PATH],visible[MAX_PATH];static DWORD ui;static volatile LONG init,unknown,filter_ok,visits,denied,enum_calls,cleared,refreshed,requested,visible_events;static HWND dialog,tree,app_window;static LONG refs=1;static int visible_count,blocked_count;static BOOL error_mode,clear_mode,reenter_mode,final_selected;static IFolderFilterSite *clear_site;
static HRESULT STDMETHODCALLTYPE qi(IFolderFilter *p,REFIID iid,void **v){if(!v)return E_POINTER;*v=NULL;if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IFolderFilter)){*v=p;InterlockedIncrement(&refs);return S_OK;}return E_NOINTERFACE;}
static ULONG STDMETHODCALLTYPE add(IFolderFilter*p){(void)p;return InterlockedIncrement(&refs);}static ULONG STDMETHODCALLTYPE rel(IFolderFilter*p){(void)p;return InterlockedDecrement(&refs);}
static HRESULT STDMETHODCALLTYPE show(IFolderFilter*p,IShellFolder*f,PCIDLIST_ABSOLUTE parent,PCUITEMID_CHILD item){(void)p;(void)parent;STRRET r={0};WCHAR name[MAX_PATH]=L"";InterlockedIncrement(&visits);if(SUCCEEDED(IShellFolder_GetDisplayNameOf(f,item,SHGDN_NORMAL,&r))&&SUCCEEDED(StrRetToBufW(&r,item,name,MAX_PATH))&&!lstrcmpiW(name,L"blocked")){InterlockedIncrement(&denied);if(reenter_mode && app_window && InterlockedCompareExchange(&requested,1,0)==0)SendMessageW(app_window,BFFM_SETSELECTIONW,TRUE,(LPARAM)visible);return error_mode?E_FAIL:S_FALSE;}return S_OK;}
static HRESULT STDMETHODCALLTYPE flags(IFolderFilter*p,IShellFolder*f,PCIDLIST_ABSOLUTE parent,HWND*w,DWORD*flags){(void)p;(void)f;(void)parent;(void)w;(void)flags;InterlockedIncrement(&enum_calls);return S_OK;}
static IFolderFilterVtbl table={qi,add,rel,show,flags};static IFolderFilter filter={&table};
static void CALLBACK clear_timer(HWND w,UINT m,UINT_PTR id,DWORD t){(void)m;(void)t;KillTimer(w,id);if(clear_site && SUCCEEDED(IFolderFilterSite_SetFilter(clear_site,NULL)))InterlockedExchange(&cleared,1);}
static int CALLBACK cb(HWND w,UINT m,LPARAM p,LPARAM d){if(d!=0x456789)return 0;if(m==BFFM_INITIALIZED)InterlockedIncrement(&init);if(m==BFFM_SELCHANGED && p){WCHAR path[MAX_PATH]=L"";if(SHGetPathFromIDListW((PCIDLIST_ABSOLUTE)p,path) && !lstrcmpiW(path,visible))InterlockedIncrement(&visible_events);}if(m==BFFM_IUNKNOWN && p){app_window=w;InterlockedIncrement(&unknown);IFolderFilterSite *site=NULL;HRESULT hr=IUnknown_QueryInterface((IUnknown*)p,&IID_IFolderFilterSite,(void**)&site);if(SUCCEEDED(hr)&&site){hr=IFolderFilterSite_SetFilter(site,(IUnknown*)&filter);if(SUCCEEDED(hr)){InterlockedIncrement(&filter_ok);if(clear_mode){IFolderFilterSite_AddRef(site);clear_site=site;SetTimer(w,0x7331,300,clear_timer);}}IFolderFilterSite_Release(site);}}return 0;}
static BOOL CALLBACK child(HWND w,LPARAM v){(void)v;WCHAR c[60];if(GetClassNameW(w,c,60)&&!lstrcmpiW(c,L"SysTreeView32"))tree=w;return TRUE;}
static BOOL CALLBACK scan(HWND w,LPARAM v){(void)v;WCHAR c[60];if(IsWindowVisible(w)&&GetClassNameW(w,c,60)&&!lstrcmpW(c,L"#32770")){dialog=w;EnumChildWindows(w,child,0);}return TRUE;}
static void walk(HWND t,HTREEITEM item,int *yes,int *no){for(HTREEITEM i=item;i;i=TreeView_GetNextSibling(t,i)){WCHAR s[160]=L"";TVITEMW a={0};a.mask=TVIF_TEXT;a.hItem=i;a.pszText=s;a.cchTextMax=160;if(TreeView_GetItem(t,&a)){if(!lstrcmpiW(s,L"blocked"))++*no;if(!lstrcmpiW(s,L"visible"))++*yes;}HTREEITEM child=TreeView_GetChild(t,i);if(child)walk(t,child,yes,no);}}
static DWORD WINAPI observe(void *v){(void)v;for(int i=0;i<120;i++){if(!dialog){EnumThreadWindows(ui,scan,0);if(dialog && !tree)dialog=NULL;}if(dialog&&tree){HTREEITEM top=TreeView_GetRoot(tree);if(top){if(clear_mode && cleared && !refreshed){TreeView_Expand(tree,top,TVE_COLLAPSE|TVE_COLLAPSERESET);InterlockedExchange(&refreshed,1);}TreeView_Expand(tree,top,TVE_EXPAND);}if(i>30){int yes=0,no=0;walk(tree,TreeView_GetRoot(tree),&yes,&no);visible_count=yes;blocked_count=no;if(reenter_mode){WCHAR label[100]=L"";TVITEMW item={0};item.mask=TVIF_TEXT;item.hItem=TreeView_GetSelection(tree);item.pszText=label;item.cchTextMax=100;final_selected=item.hItem && TreeView_GetItem(tree,&item) && !lstrcmpiW(label,L"visible");}printf("TREE visible=%d blocked=%d visits=%ld denied=%ld\n",yes,no,visits,denied);PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);return 0;}}Sleep(50);}if(dialog)PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);return 1;}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);
    if(argc!=2 && argc!=3)return 2;
    BOOL reduced=argc==3 && !lstrcmpW(argv[2],L"reduced");error_mode=argc==3 && !lstrcmpW(argv[2],L"error");clear_mode=argc==3 && !lstrcmpW(argv[2],L"clear");reenter_mode=argc==3 && !lstrcmpW(argv[2],L"reenter");if(argc==3 && !reduced && !error_mode && !clear_mode && !reenter_mode)return 2;
    WCHAR parent[MAX_PATH];DWORD n=GetFullPathNameW(argv[1],MAX_PATH,parent,NULL);
    DWORD attributes=GetFileAttributesW(parent);
    if(!n || n>=MAX_PATH || attributes==INVALID_FILE_ATTRIBUTES || !(attributes&FILE_ATTRIBUTE_DIRECTORY))return 2;
    int a=swprintf(root,MAX_PATH,L"%ls\\Win16S-filter-%lu-%lu",parent,GetCurrentProcessId(),GetTickCount());
    int b=swprintf(hidden,MAX_PATH,L"%ls\\blocked",root),c=swprintf(visible,MAX_PATH,L"%ls\\visible",root);
    if(a<0 || b<0 || c<0 || a>=MAX_PATH || b>=MAX_PATH || c>=MAX_PATH)return 2;
    if(!CreateDirectoryW(root,NULL))return 3;
    if(!CreateDirectoryW(hidden,NULL)){RemoveDirectoryW(root);return 3;}
    if(!CreateDirectoryW(visible,NULL)){RemoveDirectoryW(hidden);RemoveDirectoryW(root);return 3;}
    HRESULT init_hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(init_hr)){RemoveDirectoryW(hidden);RemoveDirectoryW(visible);RemoveDirectoryW(root);return 4;}
    PIDLIST_ABSOLUTE r=NULL;
    if(FAILED(SHParseDisplayName(root,NULL,&r,0,NULL))){CoUninitialize();RemoveDirectoryW(hidden);RemoveDirectoryW(visible);RemoveDirectoryW(root);return 5;}
    ui=GetCurrentThreadId();HANDLE thread=CreateThread(NULL,0,observe,NULL,0,NULL);
    if(!thread){CoTaskMemFree(r);CoUninitialize();RemoveDirectoryW(hidden);RemoveDirectoryW(visible);RemoveDirectoryW(root);return 6;}
    BROWSEINFOW info={0};info.pidlRoot=r;info.ulFlags=BIF_RETURNONLYFSDIRS|(reduced?(BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON):BIF_USENEWUI);info.lpfn=cb;info.lParam=0x456789;
    PIDLIST_ABSOLUTE result=SHBrowseForFolderW(&info);if(result)CoTaskMemFree(result);if(clear_site){IFolderFilterSite_Release(clear_site);clear_site=NULL;}
    DWORD wait=WaitForSingleObject(thread,8000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);
    CloseHandle(thread);CoTaskMemFree(r);CoUninitialize();
    BOOL a_removed=RemoveDirectoryW(hidden),b_removed=RemoveDirectoryW(visible),c_removed=RemoveDirectoryW(root);
    BOOL cleaned=a_removed && b_removed && c_removed;
    BOOL valid=unknown==1 && filter_ok==1 && enum_calls>0 && denied>0 && visible_count>0 && (clear_mode?(cleared && refreshed && blocked_count>0):blocked_count==0) && (!reenter_mode || (requested && visible_events && final_selected)) && refs==1 && wait==WAIT_OBJECT_0 && code==0 && cleaned;
    printf("FOLDER_FILTER bits=%u reduced=%d error=%d clear=%d reenter=%d requested=%ld visible_events=%ld final_selected=%d cleared=%ld refreshed=%ld unknown=%ld installed=%ld enum_calls=%ld calls=%ld denied=%ld visible=%d blocked=%d refs=%ld worker=%lu cleanup=%d valid=%d\n",(unsigned)(sizeof(void*)*8),reduced,error_mode,clear_mode,reenter_mode,requested,visible_events,final_selected,cleared,refreshed,unknown,filter_ok,enum_calls,visits,denied,visible_count,blocked_count,refs,code,cleaned,valid);
    return valid?0:1;
}
