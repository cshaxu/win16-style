#define COBJMACROS
#include "file_helpers.h"
#include <dlgs.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string.h>
#include <wchar.h>

static BOOL old_dialog_folder(HWND window,WCHAR *out,size_t capacity,BOOL for_shortcut)
{
    HWND list=GetDlgItem(window,lst2);WCHAR cls[32]=L"";
    if(!list||GetParent(list)!=window||!GetClassNameW(list,cls,ARRAYSIZE(cls))||
       lstrcmpiW(cls,L"ListBox")||!(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT selected=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(selected<0||selected>=count||selected>255)return FALSE;
    size_t used=0;out[0]=0;
    for(LRESULT i=0;i<=selected;++i){WCHAR part[MAX_PATH];LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(n<=0||n>=MAX_PATH||used+(size_t)n+2>capacity||SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=n)return FALSE;
        if(i==0){if(!(n==3&&part[1]==L':'&&part[2]==L'\\') &&
                     !(!for_shortcut&&n>3&&part[0]==L'\\'&&part[1]==L'\\'))return FALSE;}
        else if(wcspbrk(part,L"\\/:*?\"<>|")||!lstrcmpW(part,L".")||!lstrcmpW(part,L".."))return FALSE;
        if(used&&out[used-1]!=L'\\')out[used++]=L'\\';
        memcpy(out+used,part,((size_t)n+1)*sizeof(WCHAR));used+=(size_t)n;
    }
    if(for_shortcut)return TRUE;
    DWORD attrs=GetFileAttributesW(out);
    return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY)!=0;
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
        if(wcspbrk(path,L"\\/:*?\"<>|")||!old_dialog_folder(window,folder,ARRAYSIZE(folder),TRUE))goto done;
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
    path[0]=0;
    return old_dialog_folder(window,path,capacity,FALSE);
}


/* Return an owned item for a valid old-dialog folder-list selection. */
IShellItem *win16_dialog_classic_list_target(HWND w,IShellItem *folder)
{
    HWND list=GetDlgItem(w,lst2);if(!list || !folder)return NULL;
    LRESULT selected=SendMessageW(list,LB_GETCURSEL,0,0);
    LRESULT length=selected==LB_ERR?LB_ERR:SendMessageW(list,LB_GETTEXTLEN,(WPARAM)selected,0);
    if(length<0 || length>=MAX_PATH)return NULL;
    WCHAR name[MAX_PATH],path[32768];PWSTR base=NULL;
    if(SendMessageW(list,LB_GETTEXT,(WPARAM)selected,(LPARAM)name)==LB_ERR ||
       FAILED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base)))return NULL;
    size_t base_length=wcslen(base),depth=0;
    if(base_length<3 || base[1]!=L':' || base[2]!=L'\\'){CoTaskMemFree(base);return NULL;}
    for(size_t i=3;i<base_length;){while(i<base_length && base[i]==L'\\')++i;if(i>=base_length)break;
        ++depth;while(i<base_length && base[i]!=L'\\')++i;}
    int n;
    if((size_t)selected<=depth){
        size_t end=3,component=0;
        while(component<(size_t)selected && end<base_length){while(end<base_length && base[end]==L'\\')++end;
            size_t begin=end;while(end<base_length && base[end]!=L'\\')++end;++component;
            if(component==(size_t)selected && ((size_t)length!=end-begin || _wcsnicmp(base+begin,name,end-begin))){CoTaskMemFree(base);return NULL;}}
        if(!selected && (length!=3 || _wcsnicmp(base,name,3))){CoTaskMemFree(base);return NULL;}
        if(end>=ARRAYSIZE(path)){CoTaskMemFree(base);return NULL;}
        memcpy(path,base,end*sizeof(WCHAR));path[end]=0;n=(int)end;
    }else n=swprintf(path,ARRAYSIZE(path),L"%ls%ls%ls",base,base[base_length-1]==L'\\'?L"":L"\\",name);
    CoTaskMemFree(base);
    if(n<0 || (size_t)n>=ARRAYSIZE(path))return NULL;
    IShellItem *target=NULL;if(FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&target)))return NULL;
    return target;
}

BOOL win16_dialog_classic_typed_target(HWND w,IShellItem *folder,IShellItem **out)
{
    *out=NULL;
    HWND edit=GetDlgItem(w,edt1);int length=edit?GetWindowTextLengthW(edit):-1;
    if(length<=0 || length>=32768)return TRUE;
    WCHAR *path=CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR));if(!path)return FALSE;
    GetWindowTextW(edit,path,length+1);
    /* A single leading separator uses the dialog folder's drive, not the
     * process's potentially unrelated current drive. UNC stays untouched. */
    if((path[0]==L'\\' || path[0]==L'/') && path[1]!=L'\\' && path[1]!=L'/' && folder){
        PWSTR base=NULL;if(SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
            if(base[0] && base[1]==L':'){WCHAR *rooted=CoTaskMemAlloc(((size_t)length+3)*sizeof(WCHAR));
                if(!rooted){CoTaskMemFree(base);CoTaskMemFree(path);return FALSE;}
                rooted[0]=base[0];rooted[1]=L':';memcpy(rooted+2,path,((size_t)length+1)*sizeof(WCHAR));
                CoTaskMemFree(path);path=rooted;}
            CoTaskMemFree(base);}
    }
    /* The legacy dialog resolves a drive-relative name on its displayed
     * drive against the displayed folder, even if the process current
     * directory on that drive differs. Normalize only that same-drive case;
     * another drive retains its native per-drive current-directory rules. */
    if(length>2 && path[1]==L':' && path[2]!=L'\\' && path[2]!=L'/' && folder){
        PWSTR base=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
            if(base[0] && base[1]==L':' && _wcsnicmp(path,base,1)==0){
                memmove(path,path+2,((size_t)length-1)*sizeof(WCHAR));length-=2;
            }
            CoTaskMemFree(base);
        }
    }
    /* Relative input belongs to the dialog's displayed folder, which can
     * differ from the process-wide current directory. */
    if(path[0]!=L'\\' && path[0]!=L'/' && !(length>=2 && path[1]==L':') && folder){
        PWSTR base=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
            size_t n=wcslen(base),total=n+(n && base[n-1]!=L'\\'?1:0)+(size_t)length+1;
            if(total<=32768){WCHAR *combined=CoTaskMemAlloc(total*sizeof(WCHAR));
                if(!combined){CoTaskMemFree(base);CoTaskMemFree(path);return FALSE;}
                swprintf(combined,total,L"%ls%ls%ls",base,n && base[n-1]!=L'\\'?L"\\":L"",path);
                CoTaskMemFree(path);path=combined;}
            CoTaskMemFree(base);
        }
    }
    WCHAR normalized[32768];DWORD written=GetFullPathNameW(path,ARRAYSIZE(normalized),normalized,NULL);
    if(!written || written>=ARRAYSIZE(normalized)){CoTaskMemFree(path);return TRUE;}
    CoTaskMemFree(path);DWORD attrs=GetFileAttributesW(normalized);
    if(attrs==INVALID_FILE_ATTRIBUTES || !(attrs&FILE_ATTRIBUTE_DIRECTORY))return TRUE;
    IShellItem *target=NULL;
    if(FAILED(SHCreateItemFromParsingName(normalized,NULL,&IID_IShellItem,(void **)&target)))return TRUE;
    *out=target;
    return TRUE;
}

IShellItem *win16_dialog_classic_drive_target(HWND w,LRESULT *selected,HWND *combo_out)
{
    HWND combo=GetDlgItem(w,cmb2);LRESULT index=combo?SendMessageW(combo,CB_GETCURSEL,0,0):CB_ERR;
    LRESULT length=index==CB_ERR?CB_ERR:SendMessageW(combo,CB_GETLBTEXTLEN,(WPARAM)index,0);
    if(length<2 || length>=MAX_PATH)return NULL;
    WCHAR label[MAX_PATH],relative[4],path[32768];
    if(SendMessageW(combo,CB_GETLBTEXT,(WPARAM)index,(LPARAM)label)==CB_ERR || label[1]!=L':' ||
       !((label[0]>=L'A'&&label[0]<=L'Z')||(label[0]>=L'a'&&label[0]<=L'z')))return NULL;
    relative[0]=label[0];relative[1]=L':';relative[2]=L'.';relative[3]=0;
    DWORD written=GetFullPathNameW(relative,ARRAYSIZE(path),path,NULL);
    if(!written || written>=ARRAYSIZE(path))return NULL;
    IShellItem *target=NULL;if(FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&target)))return NULL;
    *selected=index;*combo_out=combo;return target;
}

IShellItem *win16_dialog_classic_overwrite_target(HWND w,IShellItem *folder,PCWSTR extension)
{
    WCHAR entered[32768],path[32768];HWND edit=GetDlgItem(w,edt1);
    int length=edit?GetWindowTextW(edit,entered,ARRAYSIZE(entered)):0;
    if(length<=0 || length>=(int)ARRAYSIZE(entered)-1)return NULL;
    PCWSTR source=entered;WCHAR combined[32768];PWSTR base=NULL;
    if((entered[0]==L'\\' || entered[0]==L'/') && entered[1]!=L'\\' && entered[1]!=L'/' && folder &&
       SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
        if(base[0] && base[1]==L':' && (size_t)length+3<=ARRAYSIZE(combined)){
            combined[0]=base[0];combined[1]=L':';memcpy(combined+2,entered,((size_t)length+1)*sizeof(WCHAR));source=combined;}
        CoTaskMemFree(base);base=NULL;
    }
    PCWSTR relative=entered;
    if(length>2 && entered[1]==L':' && entered[2]!=L'\\' && entered[2]!=L'/' && folder &&
       SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
        if(base[0] && base[1]==L':' && _wcsnicmp(entered,base,1)==0)relative=entered+2;
        CoTaskMemFree(base);base=NULL;
    }
    if(relative[0]!=L'\\' && relative[0]!=L'/' && !(relative[0] && relative[1]==L':') && folder &&
       SUCCEEDED(IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&base))){
        size_t n=wcslen(base);int written=swprintf(combined,ARRAYSIZE(combined),L"%ls%ls%ls",base,
            n && base[n-1]!=L'\\'?L"\\":L"",relative);
        CoTaskMemFree(base);
        if(written<0 || (size_t)written>=ARRAYSIZE(combined))return NULL;
        source=combined;
    }
    DWORD written=GetFullPathNameW(source,ARRAYSIZE(path),path,NULL);
    if(!written || written>=ARRAYSIZE(path))return NULL;
    if(extension && *extension){
        WCHAR *name=wcsrchr(path,L'\\');name=name?name+1:path;
        if(*name && !wcschr(name,L'.')){size_t base=wcslen(path),ext=wcslen(extension);
            if(base+1+ext>=ARRAYSIZE(path))return NULL;
            path[base]=L'.';memcpy(path+base+1,extension,(ext+1)*sizeof(WCHAR));}
    }
    DWORD attrs=GetFileAttributesW(path);
    if(attrs==INVALID_FILE_ATTRIBUTES || (attrs&FILE_ATTRIBUTE_DIRECTORY))return NULL;
    IShellItem *item=NULL;if(FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&item)))return NULL;
    return item;
}
