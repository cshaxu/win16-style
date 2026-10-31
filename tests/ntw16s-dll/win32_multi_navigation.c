#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

static const WCHAR title[]=L"Win16S long-name navigation probe";
static WCHAR root[MAX_PATH],child[MAX_PATH];
static DWORD gui_thread;
static HWND dialog;
static volatile LONG seen,phase,refreshes,dir_ok;
static BOOL noadd;
static WCHAR actual_folder[MAX_PATH];

/* Read the old system dialog's ancestor list; never use process CWD. */
static BOOL dialog_folder(HWND w,WCHAR *out,size_t capacity)
{
    HWND list=GetDlgItem(w,lst2);WCHAR cls[32]=L"";
    if(!list||GetParent(list)!=w||!GetClassNameW(list,cls,ARRAYSIZE(cls))||lstrcmpiW(cls,L"ListBox")||!(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT selected=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(selected<0||selected>=count||selected>255)return FALSE;
    size_t used=0;out[0]=0;
    for(LRESULT i=0;i<=selected;++i){WCHAR part[MAX_PATH];LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(n<=0||n>=MAX_PATH||used+(size_t)n+2>capacity)return FALSE;
        if(SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=n)return FALSE;
        if(i==0){if(!(n==3&&part[1]==L':'&&part[2]==L'\\')&&!(n>3&&part[0]==L'\\'&&part[1]==L'\\'))return FALSE;}
        else if(wcspbrk(part,L"\\/:*?\"<>|")||!lstrcmpW(part,L".")||!lstrcmpW(part,L".."))return FALSE;
        if(used&&out[used-1]!=L'\\')out[used++]=L'\\';
        memcpy(out+used,part,((size_t)n+1)*sizeof(WCHAR));used+=(size_t)n;
    }
    DWORD attrs=GetFileAttributesW(out);return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY)!=0;
}
static void refresh(HWND w)
{
    HWND list=GetDlgItem(w,lst1),combo=GetDlgItem(w,cmb1);WCHAR folder[MAX_PATH],pattern[MAX_PATH];
    if(!list||!combo||!dialog_folder(w,folder,ARRAYSIZE(folder)))return;
    LRESULT filter=SendMessageW(combo,CB_GETCURSEL,0,0);if(filter!=0&&filter!=1)return;
    const WCHAR *suffix=filter==0?L"*.txt":L"*.log";
    int n=swprintf(pattern,ARRAYSIZE(pattern),L"%ls\\%ls",folder,suffix);if(n<0||n>=MAX_PATH)return;
    WIN32_FIND_DATAW f;HANDLE h=FindFirstFileW(pattern,&f);
    if(h!=INVALID_HANDLE_VALUE){do{
        if(!noadd && !(f.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) &&
           SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)f.cFileName)==LB_ERR)
            (void)SendMessageW(list,LB_ADDSTRING,0,(LPARAM)f.cFileName);
    }while(FindNextFileW(h,&f));FindClose(h);}
    lstrcpynW(actual_folder,folder,ARRAYSIZE(actual_folder));InterlockedIncrement(&refreshes);
    printf("NAV_REFRESH folder=%ls filter=%lld\n",folder,(long long)filter);
}
static UINT_PTR CALLBACK hook(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{
    (void)lp;
    if(msg==WM_INITDIALOG)PostMessageW(w,WM_APP+51,0,0);
    if(msg==WM_APP+51){refresh(w);return 1;}
    if(msg==WM_COMMAND && ((LOWORD(wp)==lst2 && HIWORD(wp)==LBN_DBLCLK)||
        (LOWORD(wp)==cmb1 && HIWORD(wp)==CBN_SELCHANGE)))PostMessageW(w,WM_APP+51,0,0);
    return 0;
}
static BOOL CALLBACK locate(HWND w,LPARAM unused)
{
    (void)unused;WCHAR text[128]=L"",cls[32]=L"";DWORD_PTR copied=0;
    if(!IsWindowVisible(w)||!GetClassNameW(w,cls,ARRAYSIZE(cls))||lstrcmpW(cls,L"#32770"))return TRUE;
    if(!SendMessageTimeoutW(w,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,300,&copied)||lstrcmpW(text,title))return TRUE;
    if(!GetDlgItem(w,lst1)||!GetDlgItem(w,lst2))return TRUE;
    dialog=w;InterlockedExchange(&seen,1);return FALSE;
}
static BOOL has(HWND list,PCWSTR name)
{return SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)name)!=LB_ERR;}
static int occurrences(HWND list,PCWSTR name)
{
    LRESULT count=SendMessageW(list,LB_GETCOUNT,0,0);int found=0;
    if(count<0||count>10000)return -1;
    for(LRESULT i=0;i<count;++i){WCHAR value[MAX_PATH];LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(n>=0&&n<MAX_PATH&&SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)value)!=LB_ERR&&!_wcsicmp(value,name))++found;}
    return found;
}
static DWORD WINAPI observer(void *unused)
{
    (void)unused;
    for(unsigned i=0;i<160;++i){
        if(!seen)EnumThreadWindows(gui_thread,locate,0);
        if(seen && IsWindow(dialog)){
            HWND list=GetDlgItem(dialog,lst1),folders=GetDlgItem(dialog,lst2),filters=GetDlgItem(dialog,cmb1);
            WCHAR current[MAX_PATH]=L"";BOOL known=dialog_folder(dialog,current,ARRAYSIZE(current));
            if(phase==0 && known && !_wcsicmp(current,root) && occurrences(list,L"alpha long.txt")==1 && !has(list,L"beta long.txt") && !has(list,L"secret long.txt")){
                LRESULT index=SendMessageW(folders,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"SUB");
                if(index!=LB_ERR && SendMessageW(folders,LB_SETCURSEL,(WPARAM)index,0)!=LB_ERR){InterlockedExchange(&phase,1);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(lst2,LBN_DBLCLK),(LPARAM)folders);}}
            else if(phase==1 && known && !_wcsicmp(current,child) && occurrences(list,L"beta long.txt")==1 && !has(list,L"alpha long.txt") && !has(list,L"gamma long.log")){
                InterlockedExchange(&dir_ok,1);InterlockedExchange(&phase,2);SendMessageW(filters,CB_SETCURSEL,1,0);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(cmb1,CBN_SELCHANGE),(LPARAM)filters);}
            else if(phase==2 && known && !_wcsicmp(current,child) && occurrences(list,L"gamma long.log")==1 && !has(list,L"beta long.txt")){
                InterlockedExchange(&phase,3);SendMessageW(filters,CB_SETCURSEL,0,0);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(cmb1,CBN_SELCHANGE),(LPARAM)filters);}
            else if(phase==3 && known && !_wcsicmp(current,child) && occurrences(list,L"beta long.txt")==1 && !has(list,L"gamma long.log")){
                LRESULT index=SendMessageW(folders,LB_GETCURSEL,0,0);
                if(index>0 && SendMessageW(folders,LB_SETCURSEL,(WPARAM)(index-1),0)!=LB_ERR){InterlockedExchange(&phase,4);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(lst2,LBN_DBLCLK),(LPARAM)folders);}}
            else if(phase==4 && known && !_wcsicmp(current,root) && occurrences(list,L"alpha long.txt")==1 && !has(list,L"beta long.txt") && !has(list,L"secret long.txt")){
                InterlockedExchange(&phase,5);PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);}
            if(i==(noadd?50u:130u))PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);
        }
        if(seen&&!IsWindow(dialog))return 0;
        Sleep(50);
    }
    if(dialog&&IsWindow(dialog))PostMessageW(dialog,WM_CLOSE,0,0);
    return 1;
}
static BOOL touch(PCWSTR path){HANDLE f=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return FALSE;CloseHandle(f);return TRUE;}
static void discard_fixture(PCWSTR a,PCWSTR b,PCWSTR c,PCWSTR secret)
{
    if(secret[0]){SetFileAttributesW(secret,FILE_ATTRIBUTE_NORMAL);DeleteFileW(secret);}
    if(a[0])DeleteFileW(a);
    if(b[0])DeleteFileW(b);
    if(c[0])DeleteFileW(c);
    if(child[0])RemoveDirectoryW(child);
    if(root[0])RemoveDirectoryW(root);
}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=3 && argc!=4)return 2;
    noadd=argc==4 && !wcscmp(argv[3],L"noadd");if(argc==4 && !noadd)return 2;
    BOOL wide=!wcscmp(argv[1],L"wide");if(!wide&&wcscmp(argv[1],L"ansi"))return 2;
    WCHAR base[MAX_PATH],cwd[MAX_PATH],a[MAX_PATH]=L"",b[MAX_PATH]=L"",c[MAX_PATH]=L"",secret[MAX_PATH]=L"";
    DWORD n=GetFullPathNameW(argv[2],ARRAYSIZE(base),base,NULL),cn=GetCurrentDirectoryW(ARRAYSIZE(cwd),cwd);
    if(!n||n>=ARRAYSIZE(base)||!cn||cn>=ARRAYSIZE(cwd)||!GetTempFileNameW(base,L"n9",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 3;
    printf("NAV_FIXTURE %ls\n",root);
    if(swprintf(child,MAX_PATH,L"%ls\\SUB",root)<0||swprintf(a,MAX_PATH,L"%ls\\alpha long.txt",root)<0||
       swprintf(b,MAX_PATH,L"%ls\\beta long.txt",child)<0||swprintf(c,MAX_PATH,L"%ls\\gamma long.log",child)<0 ||
       swprintf(secret,MAX_PATH,L"%ls\\secret long.txt",root)<0||
       !CreateDirectoryW(child,NULL)||!touch(a)||!touch(b)||!touch(c)||!touch(secret)||!SetFileAttributesW(secret,FILE_ATTRIBUTE_HIDDEN)){
        discard_fixture(a,b,c,secret);return 4;
    }
    char ansi_path[MAX_PATH];if(!wide && !WideCharToMultiByte(CP_ACP,0,root,-1,ansi_path,ARRAYSIZE(ansi_path),NULL,NULL)){discard_fixture(a,b,c,secret);return 6;}
    gui_thread=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observer,NULL,0,NULL);if(!worker){discard_fixture(a,b,c,secret);return 5;}
    BOOL result;DWORD error;
    if(wide){WCHAR file[MAX_PATH]=L"";OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrTitle=title;o.lpstrInitialDir=root;o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.lpstrFilter=L"Text\0*.txt\0Log\0*.log\0\0";o.nFilterIndex=1;o.Flags=OFN_ALLOWMULTISELECT|OFN_ENABLEHOOK|OFN_NOCHANGEDIR;o.lpfnHook=hook;result=GetOpenFileNameW(&o);}
    else{char file[MAX_PATH]={0};OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrTitle="Win16S long-name navigation probe";o.lpstrInitialDir=ansi_path;o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.lpstrFilter="Text\0*.txt\0Log\0*.log\0\0";o.nFilterIndex=1;o.Flags=OFN_ALLOWMULTISELECT|OFN_ENABLEHOOK|OFN_NOCHANGEDIR;o.lpfnHook=hook;result=GetOpenFileNameA(&o);}
    error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(worker,9000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&code);CloseHandle(worker);
    BOOL restored=SetCurrentDirectoryW(cwd),ca=DeleteFileW(a),cb=DeleteFileW(b),cc=DeleteFileW(c),normal=SetFileAttributesW(secret,FILE_ATTRIBUTE_NORMAL),cs=DeleteFileW(secret),cd=RemoveDirectoryW(child),cr=RemoveDirectoryW(root);
    printf("MULTI_NAV bits=%u wide=%d noadd=%d seen=%ld phase=%ld dir=%ld refresh=%ld result=%d error=%lu wait=%lu worker=%lu cleanup=%d%d%d%d%d%d%d%d\n",(unsigned)(sizeof(void*)*8),wide,noadd,seen,phase,dir_ok,refreshes,result,error,wait,code,restored,ca,cb,cc,normal,cs,cd,cr);
    BOOL expected=noadd?(phase==0 && dir_ok==0 && refreshes>=1):(phase==5 && dir_ok && refreshes>=5);
    return seen&&expected&&!result&&!error&&wait==WAIT_OBJECT_0&&!code&&restored&&ca&&cb&&cc&&normal&&cs&&cd&&cr?0:1;
}
