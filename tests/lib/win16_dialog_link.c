#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <dlgs.h>
#include "lib/win16-dialog/print_ranges.h"
#include "lib/win16-dialog/print_classic.h"
#include "lib/win16-dialog/file_contract.h"
#include "lib/win16-dialog/modern.h"
#include "lib/win16-dialog/shell_item.h"
#include "lib/win16-dialog/v2.h"
#include "lib/win16-dialog/primary.h"
#include "lib/win16-dialog/com_show.h"
#include "lib/win16-dialog/proxy.h"
#include "lib/win16-dialog/browse.h"

static UINT_PTR CALLBACK fake_print_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return 0;}
static void *observed_owner;static win16_dialog_v2_operation observed_op;static ULONG_PTR observed_a,observed_b;
static HRESULT fake_v2_call(void *owner,win16_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{observed_owner=owner;observed_op=op;observed_a=a;observed_b=b;return S_FALSE;}
static HRESULT fake_v2_query(void *owner,REFIID iid,void **out)
{(void)iid;*out=owner;return S_OK;}
static ULONG fake_v2_ref(void *owner){return owner?7:0;}
static BOOL no_session(void){return FALSE;}
static HRESULT fallback_show(void *owner,HWND window){observed_owner=owner;observed_a=(ULONG_PTR)window;return E_ABORT;}
static BOOL test_capture_selection(void)
{
    WCHAR dir[MAX_PATH],path[MAX_PATH];IShellItem *folder=NULL;PWSTR actual=NULL;BOOL ok=FALSE;
    HWND parent=NULL,list=NULL;win16_dialog_proxy proxy={0};
    if(!GetTempPathW(MAX_PATH,dir) || !GetTempFileNameW(dir,L"w16",0,path))return FALSE;
    parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,80,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(parent)list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,40,40,parent,(HMENU)(INT_PTR)lst1,GetModuleHandleW(NULL),NULL);
    if(!list || FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&folder)))goto done;
    proxy.live_folder=folder;
    const WCHAR *name=wcsrchr(path,L'\\');name=name?name+1:path;
    if(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)name)==LB_ERR)goto done;
    SendMessageW(list,LB_SETCURSEL,0,0);win16_dialog_proxy_capture_selection(parent,&proxy);
    if(!proxy.live_selection || FAILED(IShellItem_GetDisplayName(proxy.live_selection,SIGDN_FILESYSPATH,&actual)) || lstrcmpiW(actual,path))goto done;
    SendMessageW(list,LB_SETCURSEL,(WPARAM)-1,0);win16_dialog_proxy_capture_selection(parent,&proxy);
    ok=proxy.live_selection==NULL;
done:
    if(actual)CoTaskMemFree(actual);
    if(proxy.live_selection)IShellItem_Release(proxy.live_selection);
    if(folder)IShellItem_Release(folder);
    if(parent)DestroyWindow(parent);
    if(!DeleteFileW(path))ok=FALSE;
    return ok;
}
static BOOL test_proxy_lifetime(void)
{
    IUnknown *native=NULL,*identity=NULL;IFileDialog2 *v2=NULL;win16_dialog_proxy *proxy=NULL;BOOL ok=FALSE;
    HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IUnknown,(void **)&native);
    if(FAILED(hr))goto done;
    proxy=win16_dialog_proxy_new(native,FALSE,fake_v2_call,FALSE);if(!proxy)goto done;
    hr=win16_dialog_proxy_query(proxy,&IID_IFileDialog2,(void **)&v2);if(FAILED(hr))goto done;
    hr=IFileDialog2_QueryInterface(v2,&IID_IUnknown,(void **)&identity);
    ok=SUCCEEDED(hr) && identity==(IUnknown *)proxy;
done:
    if(identity)IUnknown_Release(identity);
    if(v2)IFileDialog2_Release(v2);
    if(proxy)win16_dialog_proxy_release(proxy);
    if(native)IUnknown_Release(native);
    return ok;
}
int main(void)
{
    PRINTPAGERANGE ranges[2];DWORD count=0;WCHAR formatted[32];
    if(!ntw16s_parse_print_ranges(L"1-3,5",1,9,ranges,2,&count) || count!=2)return 1;
    if(!ntw16s_format_print_ranges(ranges,count,1,9,formatted,32) || lstrcmpW(formatted,L"1-3,5"))return 2;
    PRINTDLGA print_a={0},copy_a={0};PRINTDLGW print_w={0},copy_w={0};
    if(win16_dialog_print_prepare_a(NULL,&copy_a))return 37;
    print_a.lStructSize=sizeof(print_a);print_a.Flags=PD_PAGENUMS;print_a.lpfnPrintHook=fake_print_hook;
    if(!win16_dialog_print_prepare_a(&print_a,&copy_a) || !(copy_a.Flags&PD_ENABLEPRINTHOOK) || copy_a.lpfnPrintHook==fake_print_hook)return 38;
    copy_a.nFromPage=3;copy_a.Flags|=PD_COLLATE;win16_dialog_print_result_a(&print_a,&copy_a);
    if((print_a.Flags&PD_ENABLEPRINTHOOK) || !(print_a.Flags&PD_COLLATE) || print_a.nFromPage!=3 || print_a.lpfnPrintHook!=fake_print_hook)return 39;
    print_a.Flags=PD_ENABLEPRINTHOOK;if(win16_dialog_print_prepare_a(&print_a,&copy_a))return 40;
    print_w.lStructSize=sizeof(print_w);print_w.Flags=PD_RETURNDC;
    if(!win16_dialog_print_prepare_w(&print_w,&copy_w))return 41;
    win16_dialog_print_result_w(&print_w,&copy_w);if(print_w.Flags&PD_ENABLEPRINTHOOK)return 42;
    OPENFILENAMEA a={0},al={0};OPENFILENAMEW w={0},wl={0};
    if(win16_dialog_file_prepare_a(NULL,&al,0,NULL))return 3;
    a.lStructSize=sizeof(a);a.Flags=OFN_EXPLORER|OFN_READONLY;SetLastError(0x1234);
    if(!win16_dialog_file_prepare_a(&a,&al,0,NULL) || (al.Flags&OFN_EXPLORER) ||
        !(al.Flags&OFN_ENABLEHOOK) || !(al.Flags&OFN_LONGNAMES) || GetLastError()!=0x1234)return 4;
    al.Flags=(al.Flags&~OFN_READONLY)|OFN_EXTENSIONDIFFERENT;al.nFilterIndex=2;al.nFileOffset=5;al.nFileExtension=8;
    win16_dialog_file_result_a(&a,&al);
    if((a.Flags&OFN_READONLY) || !(a.Flags&OFN_EXTENSIONDIFFERENT) || !(a.Flags&OFN_EXPLORER) ||
        a.nFilterIndex!=2 || a.nFileOffset!=5 || a.nFileExtension!=8)return 5;
    a.Flags=OFN_ALLOWMULTISELECT;if(win16_dialog_file_prepare_a(&a,&al,0,NULL))return 6;
    a.Flags=0;a.FlagsEx=1;if(win16_dialog_file_prepare_a(&a,&al,0,NULL))return 7;
    w.lStructSize=OPENFILENAME_SIZE_VERSION_400W;w.Flags=OFN_NODEREFERENCELINKS;
    if(win16_dialog_file_prepare_w(&w,&wl,0,NULL) ||
       !win16_dialog_file_prepare_w(&w,&wl,OFN_NODEREFERENCELINKS,NULL) ||
       wl.lStructSize!=sizeof(wl))return 8;
    wl.nFilterIndex=3;wl.nFileOffset=4;wl.nFileExtension=6;wl.Flags|=OFN_READONLY;
    win16_dialog_file_result_w(&w,&wl);
    if(!(w.Flags&OFN_READONLY)||w.nFilterIndex!=3||w.nFileOffset!=4||w.nFileExtension!=6)return 9;
    w.lStructSize=1;if(win16_dialog_file_prepare_w(&w,&wl,0,NULL))return 10;
    struct {DWORD fos,ofn;} map[]={
        {FOS_ALLOWMULTISELECT,OFN_ALLOWMULTISELECT},{FOS_OVERWRITEPROMPT,OFN_OVERWRITEPROMPT},
        {FOS_NOCHANGEDIR,OFN_NOCHANGEDIR},{FOS_PATHMUSTEXIST,OFN_PATHMUSTEXIST},
        {FOS_FILEMUSTEXIST,OFN_FILEMUSTEXIST},{FOS_CREATEPROMPT,OFN_CREATEPROMPT},
        {FOS_NOREADONLYRETURN,OFN_NOREADONLYRETURN},{FOS_NOTESTFILECREATE,OFN_NOTESTFILECREATE},
        {FOS_NODEREFERENCELINKS,OFN_NODEREFERENCELINKS}};
    DWORD base=OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_ENABLEHOOK;
    if(win16_dialog_com_file_flags(0)!=base)return 11;
    for(unsigned i=0;i<sizeof(map)/sizeof(map[0]);++i)
        if(win16_dialog_com_file_flags(map[i].fos)!=(base|map[i].ofn))return 12;
    if(!win16_dialog_com_can_bridge(S_OK,FOS_FORCEFILESYSTEM,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(E_FAIL,0,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(S_OK,FOS_ALLOWMULTISELECT,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(S_OK,0,TRUE,FALSE,FALSE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(S_OK,0,FALSE,TRUE,FALSE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(S_OK,FOS_NODEREFERENCELINKS,FALSE,FALSE,TRUE,FALSE,FALSE) ||
       win16_dialog_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       !win16_dialog_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN,FALSE,FALSE,FALSE,FALSE,TRUE) ||
       win16_dialog_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN|FOS_PICKFOLDERS,FALSE,FALSE,FALSE,FALSE,TRUE) ||
       win16_dialog_com_can_bridge(S_OK,FOS_SHAREAWARE,FALSE,FALSE,FALSE,TRUE,FALSE) ||
       !win16_dialog_com_can_bridge(S_OK,FOS_SHAREAWARE,FALSE,TRUE,FALSE,TRUE,FALSE))return 13;
    const COMDLG_FILTERSPEC specs[]={{L"Text",L"*.txt"},{L"All",L"*.*"}};
    const WCHAR expected[]={L'T',L'e',L'x',L't',0,L'*',L'.',L't',L'x',L't',0,L'A',L'l',L'l',0,L'*',L'.',L'*',0,0};
    WCHAR *serialized=NULL;
    if(!win16_dialog_format_filters(2,specs,&serialized) || !serialized ||
       memcmp(serialized,expected,sizeof(expected)))return 14;
    CoTaskMemFree(serialized);serialized=NULL;
    const COMDLG_FILTERSPEC bad[]={{NULL,L"*.txt"}};
    if(win16_dialog_format_filters(1,bad,&serialized) || serialized)return 15;
    if(!win16_dialog_format_filters(0,NULL,&serialized) || !serialized || serialized[0])return 16;
    CoTaskMemFree(serialized);
    WCHAR *copy=(WCHAR *)(UINT_PTR)1;
    if(!win16_dialog_copy_text(NULL,&copy) || copy)return 17;
    if(!win16_dialog_copy_text(L"A\x4e2d",&copy) || !copy || lstrcmpW(copy,L"A\x4e2d"))return 18;
    CoTaskMemFree(copy);
    WCHAR temp[MAX_PATH],unused[MAX_PATH];IShellItem *item=NULL;PWSTR selected=NULL;
    if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 19;
    if(!test_capture_selection()){CoUninitialize();return 37;}
    if(!GetTempPathW(MAX_PATH,temp) || !GetTempFileNameW(temp,L"w16",0,unused) || !DeleteFileW(unused))return 20;
    HRESULT item_hr=win16_dialog_save_item(unused,&item);
    if(SUCCEEDED(item_hr))item_hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&selected);
    BOOL same=SUCCEEDED(item_hr) && selected && !lstrcmpiW(selected,unused) && GetFileAttributesW(unused)==INVALID_FILE_ATTRIBUTES;
    if(selected)CoTaskMemFree(selected);
    if(item)IShellItem_Release(item);
    if(!same){CoUninitialize();return 21;}
    WCHAR root[MAX_PATH]=L"",first[MAX_PATH]=L"",second[MAX_PATH]=L"",input[MAX_PATH*2];
    BOOL root_file=FALSE,root_dir=FALSE,have_first=FALSE,have_second=FALSE,multi_ok=FALSE,cleaned=TRUE;
    IShellItem *single=NULL;IShellItemArray *multiple=NULL;
    do {
        if(!GetTempFileNameW(temp,L"w16",0,root))break;
        root_file=TRUE;
        if(!DeleteFileW(root))break;
        root_file=FALSE;
        if(!CreateDirectoryW(root,NULL))break;
        root_dir=TRUE;
        int n1=swprintf(first,MAX_PATH,L"%ls\\one.txt",root),n2=swprintf(second,MAX_PATH,L"%ls\\two.txt",root);
        int n3=swprintf(input,MAX_PATH*2,L"%ls one.txt two.txt",root);
        if(n1<0||n1>=MAX_PATH||n2<0||n2>=MAX_PATH||n3<0||n3>=MAX_PATH*2)break;
        HANDLE f=CreateFileW(first,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(f==INVALID_HANDLE_VALUE)break;
        have_first=TRUE;CloseHandle(f);
        f=CreateFileW(second,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(f==INVALID_HANDLE_VALUE)break;
        have_second=TRUE;CloseHandle(f);
        BOOL completed=FALSE;DWORD count_items=0;
        HRESULT multi=win16_dialog_multi_result(input,(WORD)(wcslen(root)+1),FALSE,&single,&multiple,&completed);
        if(SUCCEEDED(multi) && multiple)multi=IShellItemArray_GetCount(multiple,&count_items);
        multi_ok=SUCCEEDED(multi) && completed && count_items==2;
    } while(0);
    if(multiple)IShellItemArray_Release(multiple);
    if(single)IShellItem_Release(single);
    if(have_first && !DeleteFileW(first))cleaned=FALSE;
    if(have_second && !DeleteFileW(second))cleaned=FALSE;
    if(root_dir && !RemoveDirectoryW(root))cleaned=FALSE;
    if(root_file && !DeleteFileW(root))cleaned=FALSE;
    BOOL proxy_ok=test_proxy_lifetime();
    CoUninitialize();
    if(!multi_ok || !cleaned || !proxy_ok)return 32;
    int owner=0;BOOL unsupported=FALSE;WCHAR *cancel_label=NULL;win16_dialog_v2 adapter;
    const win16_dialog_customize_services identity={fake_v2_query,fake_v2_ref,fake_v2_ref};
    win16_dialog_v2_init(&adapter,&owner,NULL,&unsupported,&cancel_label,&identity,fake_v2_call);
    if(IFileDialog2_SetFileTypes(&adapter.iface,2,specs)!=S_FALSE || observed_owner!=&owner ||
       observed_op!=V2_TYPES || observed_a!=2 || observed_b!=(ULONG_PTR)specs)return 22;
    if(IFileDialog2_SetOkButtonLabel(&adapter.iface,L"OK")!=S_FALSE || observed_op!=V2_LABEL || observed_b!=TRUE)return 23;
    if(IFileDialog2_Close(&adapter.iface,E_FAIL)!=S_FALSE || observed_op!=V2_CLOSE || (HRESULT)observed_a!=E_FAIL)return 24;
    if(IFileDialog2_AddRef(&adapter.iface)!=7 || IFileDialog2_Release(&adapter.iface)!=7)return 25;
    if(!win16_dialog_proxy_iid(&IID_IFileOpenDialog,FALSE) ||
       win16_dialog_proxy_iid(&IID_IFileOpenDialog,TRUE) ||
       !win16_dialog_proxy_iid(&IID_IFileSaveDialog,TRUE) ||
       win16_dialog_proxy_iid(&IID_IFileSaveDialog,FALSE) ||
       win16_dialog_proxy_iid(&IID_IStream,FALSE) || win16_dialog_proxy_iid(NULL,FALSE))return 36;
    win16_dialog_primary primary;IShellItemArray *items=NULL;
    win16_dialog_primary_init(&primary,&owner,NULL,&unsupported,FALSE,&identity,fake_v2_call);
    IFileOpenDialog *open=(IFileOpenDialog *)&primary;
    if(IFileOpenDialog_AddRef(open)!=7 || IFileOpenDialog_GetResults(open,&items)!=S_FALSE ||
       observed_owner!=&owner || observed_op!=V2_PRIMARY_GET_RESULTS || observed_a!=(ULONG_PTR)&items)return 26;
    win16_dialog_primary_init(&primary,&owner,NULL,&unsupported,TRUE,&identity,fake_v2_call);
    IFileSaveDialog *save=(IFileSaveDialog *)&primary;
    if(IFileSaveDialog_Close(save,E_FAIL)!=S_FALSE || observed_op!=V2_PRIMARY_CLOSE || (HRESULT)observed_a!=E_FAIL)return 27;
    IShellItem *show_result=NULL;IShellItemArray *show_results=NULL;BOOL show_completed=TRUE;
    win16_dialog_com_show_state show_state={.owner=&owner,.result=&show_result,.results=&show_results,
        .completed=&show_completed,.session_active=no_session,.show_native=fallback_show};
    if(win16_dialog_com_show(&show_state,(HWND)(UINT_PTR)123)!=E_ABORT || show_completed ||
       observed_owner!=&owner || observed_a!=123)return 28;
    UINT browse_base=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_VALIDATE;
    struct {UINT input;BOOL callback;win16_dialog_browse_mode mode;UINT output;} plans[]={
        {browse_base,TRUE,WIN16_BROWSE_NATIVE,browse_base},
        {browse_base,FALSE,WIN16_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base|BIF_NONEWFOLDERBUTTON,FALSE,WIN16_BROWSE_VALIDATE,BIF_RETURNONLYFSDIRS|BIF_EDITBOX|BIF_VALIDATE},
        {BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE,FALSE,WIN16_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON,FALSE,WIN16_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base|BIF_BROWSEFORCOMPUTER,FALSE,WIN16_BROWSE_NATIVE,browse_base|BIF_BROWSEFORCOMPUTER},
        {browse_base&~BIF_VALIDATE,FALSE,WIN16_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base&~BIF_VALIDATE,TRUE,WIN16_BROWSE_NATIVE,browse_base&~BIF_VALIDATE}};
    for(unsigned i=0;i<sizeof(plans)/sizeof(plans[0]);++i){
        win16_dialog_browse_plan plan=win16_dialog_plan_browse(plans[i].input,plans[i].callback);
        if(plan.mode!=plans[i].mode || plan.flags!=plans[i].output)return 33;
    }
    win16_dialog_folder_selection selection={0};
    win16_dialog_folder_selection_callback(NULL,BFFM_VALIDATEFAILEDW,0,(LPARAM)&selection);
    if(!selection.invalid || selection.selected)return 34;
    if(win16_dialog_show_modern(NULL,1,NULL,0,NULL)!=E_INVALIDARG)return 35;
    puts("WIN16_DIALOG_STANDALONE_LINK_OK ranges=1 file_contract=1 modern_invalid=1");return 0;
}
