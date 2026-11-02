#define COBJMACROS
#include "file_helpers.h"
#include <dlgs.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string.h>
#include <wchar.h>

static BOOL old_dialog_folder(HWND window,WCHAR *out,size_t capacity)
{
    HWND list=GetDlgItem(window,lst2);WCHAR cls[32]=L"";
    if(!list||GetParent(list)!=window||!GetClassNameW(list,cls,ARRAYSIZE(cls))||
       lstrcmpiW(cls,L"ListBox")||!(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT selected=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(selected<0||selected>=count||selected>255)return FALSE;
    size_t used=0;out[0]=0;
    for(LRESULT i=0;i<=selected;++i){WCHAR part[MAX_PATH];LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(n<=0||n>=MAX_PATH||used+(size_t)n+2>capacity||SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=n)return FALSE;
        if(i==0){if(!(n==3&&part[1]==L':'&&part[2]==L'\\'))return FALSE;}
        else if(wcspbrk(part,L"\\/:*?\"<>|")||!lstrcmpW(part,L".")||!lstrcmpW(part,L".."))return FALSE;
        if(used&&out[used-1]!=L'\\')out[used++]=L'\\';
        memcpy(out+used,part,((size_t)n+1)*sizeof(WCHAR));used+=(size_t)n;
    }
    return TRUE;
}
/* Let the system finish validation and produce offsets/buffer errors after a
 * regular file shortcut is resolved. Other link forms retain native handling. */
void win16_dialog_resolve_file(HWND window)
{
    WCHAR path[MAX_PATH]=L"",target[MAX_PATH]=L"";
    DWORD saved=GetLastError();
    UINT length=GetDlgItemTextW(window,edt1,path,ARRAYSIZE(path));
    if(!length||length>=ARRAYSIZE(path)-1)goto done;
    if(!(path[0]&&path[1]==L':'&&path[2]==L'\\')){
        WCHAR folder[MAX_PATH],combined[MAX_PATH];
        if(wcspbrk(path,L"\\/:*?\"<>|")||!old_dialog_folder(window,folder,ARRAYSIZE(folder)))goto done;
        int count=swprintf(combined,ARRAYSIZE(combined),L"%ls\\%ls",folder,path);
        if(count<0||count>=MAX_PATH)goto done;
        memcpy(path,combined,((size_t)count+1)*sizeof(WCHAR));
    }
    const WCHAR *extension=wcsrchr(path,L'.');if(!extension||lstrcmpiW(extension,L".lnk"))goto done;
    HRESULT initialized=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)&&initialized!=RPC_E_CHANGED_MODE)goto done;
    IShellLinkW *link=NULL;IPersistFile *persist=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
    if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
    if(SUCCEEDED(hr))hr=IPersistFile_Load(persist,path,STGM_READ);
    if(SUCCEEDED(hr))hr=IShellLinkW_GetPath(link,target,ARRAYSIZE(target),NULL,0);
    if(hr==S_OK&&target[0]&&target[1]==L':'&&target[2]==L'\\'){DWORD attributes=GetFileAttributesW(target);
        if(attributes!=INVALID_FILE_ATTRIBUTES&&!(attributes&FILE_ATTRIBUTE_DIRECTORY))(void)SetDlgItemTextW(window,edt1,target);}
    if(persist)IPersistFile_Release(persist);
    if(link)IShellLinkW_Release(link);
    if(SUCCEEDED(initialized))CoUninitialize();
done:
    SetLastError(saved);
}

/* The old template exposes its displayed path as an ancestor list. Never
 * substitute the process current directory: any thread can change it while
 * the dialog remains open. Reject unfamiliar representations instead. */
BOOL ntw16s_old_dialog_folder(HWND window,PWSTR path,size_t capacity)
{
    if(!window || !path || !capacity)return FALSE;
    path[0]=0;HWND list=GetDlgItem(window,lst2);WCHAR cls[32];
    if(!list || GetParent(list)!=window || !GetClassNameW(list,cls,ARRAYSIZE(cls)) ||
       lstrcmpiW(cls,L"ListBox") || !(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT last=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(last<0 || count<=last || last>255)return FALSE;
    size_t used=0;
    for(LRESULT i=0;i<=last;++i){
        LRESULT length=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(length<=0 || (size_t)length>=capacity || used+(size_t)length+2>capacity)return FALSE;
        WCHAR part[MAX_PATH];if((size_t)length>=ARRAYSIZE(part) ||
            SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=length)return FALSE;
        if(i==0){
            if(!(length==3 && part[1]==L':' && part[2]==L'\\') &&
               !(length>3 && part[0]==L'\\' && part[1]==L'\\'))return FALSE;
        }else if(wcspbrk(part,L"\\/:*?\"<>|") || !lstrcmpW(part,L".") || !lstrcmpW(part,L".."))return FALSE;
        if(used && path[used-1]!=L'\\')path[used++]=L'\\';
        memcpy(path+used,part,((size_t)length+1)*sizeof(WCHAR));used+=(size_t)length;
    }
    DWORD attrs=GetFileAttributesW(path);
    return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY)!=0;
}

