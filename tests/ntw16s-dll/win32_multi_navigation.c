#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <dlgs.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

static const WCHAR title[]=L"Win16S long-name navigation probe";
static WCHAR root[MAX_PATH],child[MAX_PATH];
static DWORD gui_thread;
static HWND dialog;
static volatile LONG seen,phase,refreshes,dir_ok;
static BOOL noadd,wide;
static int selection_mode;
static volatile LONG captured;
static void *native_ofn;
static BOOL native_error_issued;
static volatile LONG selected_filter;
static WCHAR selected_folder[MAX_PATH],selected_names[2][MAX_PATH];
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
    if(msg==WM_INITDIALOG){native_ofn=(void *)lp;PostMessageW(w,WM_APP+51,0,0);}
    if(msg==WM_APP+51){refresh(w);return 1;}
    if(msg==WM_COMMAND && ((LOWORD(wp)==lst2 && HIWORD(wp)==LBN_DBLCLK)||
        (LOWORD(wp)==cmb1 && HIWORD(wp)==CBN_SELCHANGE)))PostMessageW(w,WM_APP+51,0,0);
    if(msg==WM_COMMAND && LOWORD(wp)==IDOK && selection_mode && phase==6){
        HWND list=GetDlgItem(w,lst1);LRESULT count=list?SendMessageW(list,LB_GETSELCOUNT,0,0):LB_ERR;
        int indices[2];int wanted=selection_mode==1?1:2;
        if(count==wanted && dialog_folder(w,selected_folder,ARRAYSIZE(selected_folder)) &&
           SendMessageW(list,LB_GETSELITEMS,(WPARAM)wanted,(LPARAM)indices)==wanted){BOOL ok=TRUE;
            for(int i=0;i<wanted;++i){LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)indices[i],0);
                if(n<0||n>=MAX_PATH||SendMessageW(list,LB_GETTEXT,(WPARAM)indices[i],(LPARAM)selected_names[i])==LB_ERR)ok=FALSE;}
            if(ok){LRESULT filter=SendMessageW(GetDlgItem(w,cmb1),CB_GETCURSEL,0,0);InterlockedExchange(&selected_filter,filter+1);InterlockedExchange(&captured,wanted);
                if(selection_mode==4 && native_ofn){LRESULT short_index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"beta long.txt");
                    if(short_index!=LB_ERR && SendMessageW(list,LB_SETSEL,FALSE,(LPARAM)-1)!=LB_ERR && SendMessageW(list,LB_SETSEL,TRUE,short_index)!=LB_ERR){
                        if(wide)((OPENFILENAMEW *)native_ofn)->nMaxFile=2;else ((OPENFILENAMEA *)native_ofn)->nMaxFile=2;
                        SendMessageW(w,WM_COMMAND,MAKEWPARAM(lst1,LBN_SELCHANGE),(LPARAM)list);native_error_issued=TRUE;return 0;}}
                if(PostMessageW(w,WM_COMMAND,IDABORT,0))return 1;
                InterlockedExchange(&captured,0);}}
    }
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
            else if(phase==3 && selection_mode && known && !_wcsicmp(current,child) && occurrences(list,L"beta long.txt")==1 && occurrences(list,L"delta long.txt")==1 && !has(list,L"gamma long.log")){
                LRESULT first=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"beta long.txt"),second=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"delta long.txt");
                if(first!=LB_ERR && second!=LB_ERR && SendMessageW(list,LB_SETSEL,TRUE,first)!=LB_ERR && (selection_mode==1||SendMessageW(list,LB_SETSEL,TRUE,second)!=LB_ERR)){
                    InterlockedExchange(&phase,6);PostMessageW(dialog,WM_COMMAND,IDOK,0);}}
            else if(phase==3 && !selection_mode && known && !_wcsicmp(current,child) && occurrences(list,L"beta long.txt")==1 && !has(list,L"gamma long.log")){
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
static void discard_fixture(PCWSTR a,PCWSTR b,PCWSTR c,PCWSTR d,PCWSTR secret)
{
    if(secret[0]){SetFileAttributesW(secret,FILE_ATTRIBUTE_NORMAL);DeleteFileW(secret);}
    if(a[0])DeleteFileW(a);
    if(b[0])DeleteFileW(b);
    if(c[0])DeleteFileW(c);
    if(d[0])DeleteFileW(d);
    if(child[0])RemoveDirectoryW(child);
    if(root[0])RemoveDirectoryW(root);
}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=3 && argc!=4)return 2;
    noadd=argc==4 && !wcscmp(argv[3],L"noadd");
    if(argc==4 && !wcscmp(argv[3],L"single"))selection_mode=1;
    if(argc==4 && !wcscmp(argv[3],L"multi"))selection_mode=2;
    if(argc==4 && !wcscmp(argv[3],L"small"))selection_mode=3;
    if(argc==4 && !wcscmp(argv[3],L"nativeerror"))selection_mode=4;
    if(argc==4 && !noadd && !selection_mode)return 2;
    wide=!wcscmp(argv[1],L"wide");if(!wide&&wcscmp(argv[1],L"ansi"))return 2;
    WCHAR base[MAX_PATH],cwd[MAX_PATH],a[MAX_PATH]=L"",b[MAX_PATH]=L"",c[MAX_PATH]=L"",d[MAX_PATH]=L"",secret[MAX_PATH]=L"";
    DWORD n=GetFullPathNameW(argv[2],ARRAYSIZE(base),base,NULL),cn=GetCurrentDirectoryW(ARRAYSIZE(cwd),cwd);
    if(!n||n>=ARRAYSIZE(base)||!cn||cn>=ARRAYSIZE(cwd)||!GetTempFileNameW(base,L"n9",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 3;
    printf("NAV_FIXTURE %ls\n",root);
    if(swprintf(child,MAX_PATH,L"%ls\\SUB",root)<0||swprintf(a,MAX_PATH,L"%ls\\alpha long.txt",root)<0||
       swprintf(b,MAX_PATH,L"%ls\\beta long.txt",child)<0||swprintf(c,MAX_PATH,L"%ls\\gamma long.log",child)<0 ||
       swprintf(d,MAX_PATH,L"%ls\\delta long.txt",child)<0||
       swprintf(secret,MAX_PATH,L"%ls\\secret long.txt",root)<0||
       !CreateDirectoryW(child,NULL)||!touch(a)||!touch(b)||!touch(c)||!touch(d)||!touch(secret)||!SetFileAttributesW(secret,FILE_ATTRIBUTE_HIDDEN)){
        discard_fixture(a,b,c,d,secret);return 4;
    }
    char ansi_path[MAX_PATH];if(!wide && !WideCharToMultiByte(CP_ACP,0,root,-1,ansi_path,ARRAYSIZE(ansi_path),NULL,NULL)){discard_fixture(a,b,c,d,secret);return 6;}
    gui_thread=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observer,NULL,0,NULL);if(!worker){discard_fixture(a,b,c,d,secret);return 5;}
    BOOL result;DWORD error;WCHAR file_w[8192]=L"";char file_a[8192]={0};WORD offset=0,extension=0,required=0;
    if(wide){OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrTitle=title;o.lpstrInitialDir=root;o.lpstrFile=file_w;o.nMaxFile=selection_mode==3?8:ARRAYSIZE(file_w);o.lpstrFilter=L"Text\0*.txt\0Log\0*.log\0\0";o.nFilterIndex=1;o.Flags=OFN_ALLOWMULTISELECT|OFN_ENABLEHOOK|OFN_NOCHANGEDIR;o.lpfnHook=hook;result=GetOpenFileNameW(&o);}
    else{OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrTitle="Win16S long-name navigation probe";o.lpstrInitialDir=ansi_path;o.lpstrFile=file_a;o.nMaxFile=selection_mode==3?8:ARRAYSIZE(file_a);o.lpstrFilter="Text\0*.txt\0Log\0*.log\0\0";o.nFilterIndex=1;o.Flags=OFN_ALLOWMULTISELECT|OFN_ENABLEHOOK|OFN_NOCHANGEDIR;o.lpfnHook=hook;result=GetOpenFileNameA(&o);}
    error=CommDlgExtendedError();DWORD wait=WaitForSingleObject(worker,9000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&code);CloseHandle(worker);
    BOOL native_result=result,valid=FALSE;DWORD native_error=error;
    if(selection_mode && !result && (error==0||error==CDERR_DIALOGFAILURE) && captured==(selection_mode==1?1:2) && !_wcsicmp(selected_folder,child)){
        result=TRUE;error=0;
        if(wide){size_t x=wcslen(selected_folder),y=wcslen(selected_names[0]),z=selection_mode==1?0:wcslen(selected_names[1]);
            size_t need=selection_mode==1?x+y+2:x+y+z+4;
            if(need<ARRAYSIZE(file_w)&&need<=65535){required=(WORD)need;
                if(selection_mode==3){memcpy(file_w,&required,sizeof(required));result=FALSE;error=FNERR_BUFFERTOOSMALL;}
                else if(selection_mode==1){swprintf(file_w,ARRAYSIZE(file_w),L"%ls\\%ls",selected_folder,selected_names[0]);offset=(WORD)(x+1);WCHAR *dot=wcsrchr(file_w,L'.');if(dot)extension=(WORD)(dot-file_w+1);}
                else{wcscpy(file_w,selected_folder);wcscpy(file_w+x+1,selected_names[0]);wcscpy(file_w+x+y+2,selected_names[1]);file_w[need-1]=0;offset=(WORD)(x+1);}}}
        else{char folder[MAX_PATH],first[MAX_PATH],second[MAX_PATH];
            int x=WideCharToMultiByte(CP_ACP,0,selected_folder,-1,folder,MAX_PATH,NULL,NULL),y=WideCharToMultiByte(CP_ACP,0,selected_names[0],-1,first,MAX_PATH,NULL,NULL),z=selection_mode==1?1:WideCharToMultiByte(CP_ACP,0,selected_names[1],-1,second,MAX_PATH,NULL,NULL);
            if(x>0&&y>0&&z>0){size_t need=selection_mode==1?(size_t)x+(size_t)y:(size_t)x+(size_t)y+(size_t)z+1;
                if(need<ARRAYSIZE(file_a)&&need<=65535){required=(WORD)need;
                    if(selection_mode==3){memcpy(file_a,&required,sizeof(required));result=FALSE;error=FNERR_BUFFERTOOSMALL;}
                    else if(selection_mode==1){memcpy(file_a,folder,(size_t)x-1);file_a[x-1]='\\';memcpy(file_a+x,first,(size_t)y);offset=(WORD)x;char *dot=strrchr(file_a,'.');if(dot)extension=(WORD)(dot-file_a+1);}
                    else{memcpy(file_a,folder,(size_t)x);memcpy(file_a+x,first,(size_t)y);memcpy(file_a+x+y,second,(size_t)z);file_a[need-1]=0;offset=(WORD)x;}}}}
        if(selection_mode==3)valid=!result&&error==FNERR_BUFFERTOOSMALL&&required>8;
        else if(wide && selection_mode==1)valid=result&&!_wcsicmp(file_w,b)&&offset==wcslen(child)+1&&extension==wcslen(file_w)-3;
        else if(!wide && selection_mode==1){WCHAR converted[MAX_PATH];valid=result&&MultiByteToWideChar(CP_ACP,0,file_a,-1,converted,MAX_PATH)&&!_wcsicmp(converted,b)&&offset==strlen(file_a)-strlen("beta long.txt")&&extension==strlen(file_a)-3;}
        else if(wide){WCHAR *first=file_w+wcslen(file_w)+1,*second=first+wcslen(first)+1;valid=result&&!_wcsicmp(file_w,child)&&!_wcsicmp(first,L"beta long.txt")&&!_wcsicmp(second,L"delta long.txt")&&!second[wcslen(second)+1]&&offset==first-file_w;}
        else{char *first=file_a+strlen(file_a)+1,*second=first+strlen(first)+1;WCHAR converted[MAX_PATH];valid=result&&MultiByteToWideChar(CP_ACP,0,file_a,-1,converted,MAX_PATH)&&!_wcsicmp(converted,child)&&!_stricmp(first,"beta long.txt")&&!_stricmp(second,"delta long.txt")&&!second[strlen(second)+1]&&offset==first-file_a;}
    }
    if(selection_mode==4){valid=native_error_issued&&!native_result&&native_error==FNERR_BUFFERTOOSMALL&&captured==2;
        printf("NAV_NATIVE_ERROR issued=%d actual=%lu captured=%ld valid=%d\n",native_error_issued,native_error,captured,valid);}
    else if(selection_mode)valid=valid && selected_filter==1 && (native_error==0||native_error==CDERR_DIALOGFAILURE);
    BOOL restored=SetCurrentDirectoryW(cwd),ca=DeleteFileW(a),cb=DeleteFileW(b),cc=DeleteFileW(c),ce=DeleteFileW(d),normal=SetFileAttributesW(secret,FILE_ATTRIBUTE_NORMAL),cs=DeleteFileW(secret),cd=RemoveDirectoryW(child),cr=RemoveDirectoryW(root);
    printf("MULTI_NAV bits=%u wide=%d noadd=%d seen=%ld phase=%ld dir=%ld refresh=%ld result=%d error=%lu wait=%lu worker=%lu cleanup=%d%d%d%d%d%d%d%d%d\n",(unsigned)(sizeof(void*)*8),wide,noadd,seen,phase,dir_ok,refreshes,result,error,wait,code,restored,ca,cb,cc,ce,normal,cs,cd,cr);
    printf("NAV_SELECTION mode=%d captured=%ld native=%d native_error=%lu filter=%ld offset=%u extension=%u required=%u valid=%d\n",selection_mode,captured,native_result,native_error,selected_filter,offset,extension,required,valid);
    BOOL expected=noadd?(phase==0 && dir_ok==0 && refreshes>=1):selection_mode?(phase==6 && dir_ok && refreshes>=4 && valid):(phase==5 && dir_ok && refreshes>=5 && !result && !error);
    return seen&&expected&&wait==WAIT_OBJECT_0&&!code&&restored&&ca&&cb&&cc&&ce&&normal&&cs&&cd&&cr?0:1;
}
