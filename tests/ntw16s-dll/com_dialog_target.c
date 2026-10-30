/* Real COM call through the launcher; does not explicitly request bridging. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <servprov.h>
#include <stdio.h>
#include <wchar.h>
static DWORD owner;static volatile LONG seen,old,filter_seen,label_seen,cancel_label_seen,hidden_seen;static HWND dialog;static BOOL cancel;
static WCHAR hidden_name[MAX_PATH],hidden_system_name[MAX_PATH],hidden_system_path[MAX_PATH];
static volatile LONG hidden_system_seen;
static const WCHAR title[]=L"Win16S COM bridge integration";
static BOOL CALLBACK find_dialog(HWND w,LPARAM value)
{
    (void)value;WCHAR text[128];GetWindowTextW(w,text,ARRAYSIZE(text));
    if(!IsWindowVisible(w)||lstrcmpW(text,title))return TRUE;
    HWND folders=GetDlgItem(w,1121),drives=GetDlgItem(w,1137);
    InterlockedExchange(&old,folders && drives);
    HWND files=GetDlgItem(w,1120);if(hidden_name[0] && files){LRESULT count=SendMessageW(files,LB_GETCOUNT,0,0);
        printf("HIDDEN_LIST expected=%ls count=%lld\n",hidden_name,(long long)count);
        for(LRESULT i=0;i<count && i<20;++i){WCHAR item[512]=L"";LRESULT len=SendMessageW(files,LB_GETTEXTLEN,(WPARAM)i,0);
            if(len>=0 && len<511){SendMessageW(files,LB_GETTEXT,(WPARAM)i,(LPARAM)item);printf("HIDDEN_ITEM %ls\n",item);
                if(!lstrcmpiW(item,hidden_name))InterlockedExchange(&hidden_seen,1);
                if(!lstrcmpiW(item,hidden_system_name))InterlockedExchange(&hidden_system_seen,1);}}}
    HWND filter=GetDlgItem(w,1136);if(filter && SendMessageW(filter,CB_GETCOUNT,0,0)==2 && SendMessageW(filter,CB_GETCURSEL,0,0)==1)InterlockedExchange(&filter_seen,1);
    WCHAR ok[80]=L"",label[80]=L"";GetDlgItemTextW(w,IDOK,ok,ARRAYSIZE(ok));GetDlgItemTextW(w,1090,label,ARRAYSIZE(label));
    if(!lstrcmpW(ok,L"Choose it") && !lstrcmpW(label,L"Target name:"))InterlockedExchange(&label_seen,1);
    WCHAR cancel_text[80]=L"";GetDlgItemTextW(w,IDCANCEL,cancel_text,ARRAYSIZE(cancel_text));
    if(!lstrcmpW(cancel_text,L"Stop choosing"))InterlockedExchange(&cancel_label_seen,1);
    WCHAR edit[32768]=L"";GetDlgItemTextW(w,1152,edit,ARRAYSIZE(edit));
    printf("COM_DIALOG_EDIT initial=%ls\n",edit);fflush(stdout);
    dialog=w;InterlockedExchange(&seen,1);return FALSE;
}
static DWORD WINAPI observer(void *unused)
{
    (void)unused;
    for(unsigned i=0;i<200;++i){
        if(!seen)EnumThreadWindows(owner,find_dialog,0);
        if(seen && IsWindow(dialog) && i>10 && i%10==0){HWND button=GetDlgItem(dialog,cancel?IDCANCEL:IDOK);
            WCHAR edit[32768]=L"";GetDlgItemTextW(dialog,1152,edit,ARRAYSIZE(edit));printf("COM_DIALOG_EDIT submit=%ls\n",edit);fflush(stdout);
            PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(cancel?IDCANCEL:IDOK,BN_CLICKED),(LPARAM)button);}
        if(seen && !IsWindow(dialog))return 0;
        Sleep(50);
    }
    PostThreadMessageW(owner,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3 && argc!=4)return 2;
    BOOL foreign_query=argc==4 && !lstrcmpW(argv[3],L"foreign");
    BOOL missing_query=argc==4 && !lstrcmpW(argv[3],L"missing");
    BOOL missing_control=argc==4 && !lstrcmpW(argv[3],L"missing-control");
    BOOL filtered=argc==4 && !lstrcmpW(argv[3],L"filter");
    BOOL customized=argc==4 && !lstrcmpW(argv[3],L"custom");
    BOOL use_v2=argc==4 && !lstrcmpW(argv[3],L"v2");
    BOOL use_ex=argc==4 && !lstrcmpW(argv[3],L"ex");
    BOOL factory_mode=argc==4 && !lstrcmpW(argv[3],L"factory");
    BOOL labels=argc==4 && !lstrcmpW(argv[3],L"labels");
    BOOL cancel_label=argc==4 && !lstrcmpW(argv[3],L"cancel-label");
    BOOL hidden=argc==4 && !lstrcmpW(argv[3],L"hidden");
    BOOL folder=argv[1][0]==L'f';BOOL save=argv[1][0]==L's';cancel=argv[2][0]==L'c';
    WCHAR dir[MAX_PATH],path[MAX_PATH];if(!GetTempPathW(MAX_PATH,dir)||!GetTempFileNameW(dir,L"w16",0,path))return 3;
    if((save || folder) && !DeleteFileW(path))return 4;
    if(folder && !CreateDirectoryW(path,NULL))return 4;
    if(hidden){if(save || folder || !SetFileAttributesW(path,FILE_ATTRIBUTE_HIDDEN))return 4;
        WCHAR *leaf=wcsrchr(path,L'\\');lstrcpynW(hidden_name,leaf?leaf+1:path,ARRAYSIZE(hidden_name));
        if(!GetTempFileNameW(dir,L"sys",0,hidden_system_path)||!SetFileAttributesW(hidden_system_path,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))return 4;
        leaf=wcsrchr(hidden_system_path,L'\\');lstrcpynW(hidden_system_name,leaf?leaf+1:hidden_system_path,ARRAYSIZE(hidden_system_name));}
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 5;
    IFileDialog *d=NULL;HRESULT hr;
    if(use_ex){
        const IID missing={0x2468ace0,0x4321,0x8765,{0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0x17,0x28}};
        MULTI_QI qi[4]={{&IID_IFileDialog,NULL,0},{save?&IID_IFileSaveDialog:&IID_IFileOpenDialog,NULL,0},
            {&IID_IOleWindow,NULL,0},{&missing,NULL,0}};
        hr=CoCreateInstanceEx(save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,NULL,ARRAYSIZE(qi),qi);
        if(FAILED(hr) || FAILED(qi[0].hr) || FAILED(qi[1].hr) || FAILED(qi[2].hr) || qi[3].hr!=E_NOINTERFACE || qi[3].pItf)return 14;
        IUnknown *a=NULL,*b=NULL,*c=NULL;
        if(FAILED(IUnknown_QueryInterface(qi[0].pItf,&IID_IUnknown,(void **)&a)) ||
           FAILED(IUnknown_QueryInterface(qi[1].pItf,&IID_IUnknown,(void **)&b)) ||
           FAILED(IUnknown_QueryInterface(qi[2].pItf,&IID_IUnknown,(void **)&c)) || a!=b || a!=c)return 15;
        IUnknown_Release(c);IUnknown_Release(b);IUnknown_Release(a);
        IUnknown_Release(qi[2].pItf);IUnknown_Release(qi[1].pItf);d=(IFileDialog *)qi[0].pItf;
        printf("COM_EX identity=1 partial=%08lx\n",(unsigned long)hr);
    }else if(factory_mode){IClassFactory *factory=NULL;IUnknown *a=NULL,*b=NULL;
        hr=CoGetClassObject(save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,CLSCTX_INPROC_SERVER,NULL,&IID_IClassFactory,(void **)&factory);
        if(FAILED(hr) || FAILED(IClassFactory_QueryInterface(factory,&IID_IUnknown,(void **)&a)) ||
           FAILED(IUnknown_QueryInterface(a,&IID_IClassFactory,(void **)&b)) || (void *)factory!=(void *)b)return 16;
        IUnknown_Release(b);IUnknown_Release(a);
        hr=IClassFactory_CreateInstance(factory,NULL,&IID_IFileDialog,(void **)&d);IClassFactory_Release(factory);
        printf("COM_FACTORY identity=1 hr=%08lx\n",(unsigned long)hr);
    }else hr=CoCreateInstance(save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&d);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(d,title)))return 6;
    if(foreign_query){IServiceProvider *foreign=NULL;HRESULT query=IFileDialog_QueryInterface(d,&IID_IServiceProvider,(void **)&foreign);
        printf("COM_FOREIGN_QUERY hr=%08lx present=%d\n",(unsigned long)query,foreign!=NULL);
        if(FAILED(query) || !foreign)return 19;
        IServiceProvider_Release(foreign);}
    if(missing_query){const IID missing={0x2468ace0,0x4321,0x8765,{0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0x17,0x28}};
        void *unknown=(void *)1;HRESULT query=IFileDialog_QueryInterface(d,&missing,&unknown);
        printf("COM_MISSING_QUERY hr=%08lx null=%d\n",(unsigned long)query,unknown==NULL);
        if(query!=E_NOINTERFACE || unknown)return 18;}

    if(folder){FILEOPENDIALOGOPTIONS options=0;IShellItem *initial=NULL;
        if(FAILED(IFileDialog_GetOptions(d,&options)) || FAILED(IFileDialog_SetOptions(d,options|FOS_PICKFOLDERS|FOS_FORCEFILESYSTEM)) ||
           FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&initial)))return 6;
        HRESULT set=IFileDialog_SetFolder(d,initial);IShellItem_Release(initial);if(FAILED(set))return 6;
    }else if(FAILED(IFileDialog_SetFileName(d,path)))return 6;
    if(hidden){FILEOPENDIALOGOPTIONS options=0;IShellItem *initial=NULL;
        if(FAILED(IFileDialog_GetOptions(d,&options)) || FAILED(IFileDialog_SetOptions(d,options|FOS_FORCESHOWHIDDEN)) ||
           FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&initial)))return 8;
        HRESULT set=IFileDialog_SetFolder(d,initial);IShellItem_Release(initial);if(FAILED(set))return 8;}
    if(labels && (FAILED(IFileDialog_SetOkButtonLabel(d,L"Choose it")) || FAILED(IFileDialog_SetFileNameLabel(d,L"Target name:"))))return 8;
    if(filtered){COMDLG_FILTERSPEC filters[]={{L"First",L"*.*"},{L"Second",L"*.*"}};
        if(FAILED(IFileDialog_SetFileTypes(d,2,filters))||FAILED(IFileDialog_SetFileTypeIndex(d,2))||FAILED(IFileDialog_SetDefaultExtension(d,L"txt")))return 8;}
    IUnknown *identity=NULL,*open_identity=NULL;IFileOpenDialog *open=NULL;
    if(!save){if(FAILED(IFileDialog_QueryInterface(d,&IID_IFileOpenDialog,(void **)&open)) ||
        FAILED(IFileDialog_QueryInterface(d,&IID_IUnknown,(void **)&identity)) ||
        FAILED(IFileOpenDialog_QueryInterface(open,&IID_IUnknown,(void **)&open_identity)) || identity!=open_identity)return 9;}
    IFileDialogCustomize *custom=NULL;IUnknown *custom_identity=NULL,*base_identity=NULL;IFileDialog *custom_base=NULL;
    if(FAILED(IFileDialog_QueryInterface(d,&IID_IFileDialogCustomize,(void **)&custom)) ||
       FAILED(IFileDialogCustomize_QueryInterface(custom,&IID_IUnknown,(void **)&custom_identity)) ||
       FAILED(IFileDialog_QueryInterface(d,&IID_IUnknown,(void **)&base_identity)) || custom_identity!=base_identity ||
       FAILED(IFileDialogCustomize_QueryInterface(custom,&IID_IFileDialog,(void **)&custom_base)) || custom_base!=d)return 10;
    if(missing_control){CDCONTROLSTATEF state=0;WCHAR *text=NULL;BOOL checked=FALSE;DWORD selected=0;
        HRESULT queries[]={IFileDialogCustomize_GetControlState(custom,0x76543210,&state),
            IFileDialogCustomize_GetEditBoxText(custom,0x76543210,&text),
            IFileDialogCustomize_GetCheckButtonState(custom,0x76543210,&checked),
            IFileDialogCustomize_GetControlItemState(custom,0x76543210,1,&state),
            IFileDialogCustomize_GetSelectedControlItem(custom,0x76543210,&selected)};
        if(text)CoTaskMemFree(text);
        for(unsigned i=0;i<ARRAYSIZE(queries);++i){printf("COM_MISSING_CONTROL index=%u hr=%08lx\n",i,(unsigned long)queries[i]);
            if(SUCCEEDED(queries[i]))return 20;}}
    if(customized && FAILED(IFileDialogCustomize_AddPushButton(custom,43101,L"Probe")))return 11;
    IFileDialog_Release(custom_base);IUnknown_Release(base_identity);IUnknown_Release(custom_identity);IFileDialogCustomize_Release(custom);
    IFileDialog2 *v2=NULL;IUnknown *v2_id=NULL,*base_id=NULL;
    if(FAILED(IFileDialog_QueryInterface(d,&IID_IFileDialog2,(void **)&v2)) ||
       FAILED(IFileDialog2_QueryInterface(v2,&IID_IUnknown,(void **)&v2_id)) ||
       FAILED(IFileDialog_QueryInterface(d,&IID_IUnknown,(void **)&base_id)) || v2_id!=base_id)return 12;
    if(cancel_label && (FAILED(IFileDialog2_SetCancelButtonLabel(v2,L"Original")) || FAILED(IFileDialog2_SetCancelButtonLabel(v2,L"Stop choosing"))))return 12;
    IUnknown_Release(base_id);IUnknown_Release(v2_id);
    IOleWindow *ole=NULL;IUnknown *ole_id=NULL,*base_ole_id=NULL;
    if(FAILED(IFileDialog_QueryInterface(d,&IID_IOleWindow,(void **)&ole)) ||
       FAILED(IOleWindow_QueryInterface(ole,&IID_IUnknown,(void **)&ole_id)) ||
       FAILED(IFileDialog_QueryInterface(d,&IID_IUnknown,(void **)&base_ole_id)) || ole_id!=base_ole_id)return 13;
    IUnknown_Release(base_ole_id);IUnknown_Release(ole_id);IOleWindow_Release(ole);
    owner=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observer,NULL,0,NULL);if(!worker)return 7;
    hr=use_v2?IFileDialog2_Show(v2,NULL):IFileDialog_Show(d,NULL);DWORD wait=WaitForSingleObject(worker,12000);CloseHandle(worker);
    BOOL same=FALSE;HRESULT result=E_FAIL;IShellItem *item=NULL;PWSTR name=NULL;
    if(SUCCEEDED(hr)){result=IFileDialog_GetResult(d,&item);if(SUCCEEDED(result))result=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&name);if(SUCCEEDED(result))same=!lstrcmpiW(path,name);}
    if(SUCCEEDED(hr) && !same)printf("COM_PATH_MISMATCH expected=%ls actual=%ls\n",path,name?name:L"(null)");
    BOOL array_ok=TRUE;
    if(open && SUCCEEDED(hr)){
        IShellItemArray *array=NULL;DWORD count=0;IShellItem *first=NULL;PWSTR array_path=NULL;
        HRESULT ar=IFileOpenDialog_GetResults(open,&array);
        if(SUCCEEDED(ar))ar=IShellItemArray_GetCount(array,&count);
        if(SUCCEEDED(ar) && count==1)ar=IShellItemArray_GetItemAt(array,0,&first);
        if(SUCCEEDED(ar) && first)ar=IShellItem_GetDisplayName(first,SIGDN_FILESYSPATH,&array_path);
        array_ok=SUCCEEDED(ar) && count==1 && array_path && !lstrcmpiW(path,array_path);
        if(array_path)CoTaskMemFree(array_path);
        if(first)IShellItem_Release(first);
        if(array)IShellItemArray_Release(array);
    }
    if(open_identity)IUnknown_Release(open_identity);
    if(identity)IUnknown_Release(identity);
    if(open)IFileOpenDialog_Release(open);
    if(name)CoTaskMemFree(name);
    if(item)IShellItem_Release(item);
    IFileDialog2_Release(v2);IFileDialog_Release(d);CoUninitialize();if(hidden_system_path[0]){SetFileAttributesW(hidden_system_path,FILE_ATTRIBUTE_NORMAL);DeleteFileW(hidden_system_path);}if(folder)RemoveDirectoryW(path);else DeleteFileW(path);
    printf("COM_BRIDGE bits=%u save=%d folder=%d cancel=%d seen=%ld old=%ld hr=%08lx result=%08lx same=%d wait=%lu filter=%ld array=%d labels=%ld hidden=%ld hidden_system=%ld\n",(unsigned)(sizeof(void*)*8),save,folder,cancel,seen,old,(unsigned long)hr,(unsigned long)result,same,wait,filter_seen,array_ok,label_seen,hidden_seen,hidden_system_seen);
    printf("COM_CANCEL_LABEL seen=%ld\n",cancel_label_seen);
    return wait==WAIT_OBJECT_0 && seen && ((customized||foreign_query)?!old:old) && (!filtered || filter_seen) && (!labels || label_seen) && (!cancel_label || cancel_label_seen) && (!hidden || (hidden_seen && !hidden_system_seen)) && array_ok && (cancel?hr==HRESULT_FROM_WIN32(ERROR_CANCELLED):(SUCCEEDED(hr)&&SUCCEEDED(result)&&same))?0:1;
}
