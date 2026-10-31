#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

static const WCHAR title[]=L"Win16S Win32 multi probe";
static const WCHAR *names[2];
static WCHAR full_names[2][MAX_PATH];
static DWORD gui_thread;
static HWND window;
static BOOL explorer;
static BOOL supplement,cancel_mode;
static volatile LONG inserted,captured;
static WCHAR chosen[2][MAX_PATH];
static volatile LONG seen,layout_old,listed,posted;
static HWND filename_edit;
static BOOL CALLBACK find_edit(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[64]=L"";
    if(GetClassNameW(w,cls,ARRAYSIZE(cls)) && !lstrcmpiW(cls,L"Edit")){
        printf("MULTI_EDIT id=%d visible=%d\n",GetDlgCtrlID(w),IsWindowVisible(w));
        if(IsWindowVisible(w))filename_edit=w;
    }
    return TRUE;
}

static BOOL CALLBACK inspect(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[32]=L"",text[128]=L"";
    if(!IsWindowVisible(w)||!GetClassNameW(w,cls,ARRAYSIZE(cls))||lstrcmpW(cls,L"#32770"))return TRUE;
    DWORD_PTR copied=0;if(!SendMessageTimeoutW(w,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,300,&copied)||lstrcmpW(text,title))return TRUE;
    HWND list=GetDlgItem(w,lst1),folders=GetDlgItem(w,lst2);
    BOOL old=list && folders;InterlockedExchange(&layout_old,old);window=w;InterlockedExchange(&seen,1);
    return FALSE;
}
static DWORD WINAPI observer(void *unused)
{
    (void)unused;BOOL reported=FALSE;
    for(unsigned i=0;i<140;++i){
        if(!seen)EnumThreadWindows(gui_thread,inspect,0);
        if(seen && IsWindow(window)){
            if(layout_old && !posted){
                HWND list=GetDlgItem(window,lst1);LRESULT count=SendMessageW(list,LB_GETCOUNT,0,0);LONG found=0;
                if(count>=0 && count<10000)for(LRESULT j=0;j<count;++j){WCHAR value[512]=L"";LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)j,0);
                    if(n<0||n>=511)continue;
                    SendMessageW(list,LB_GETTEXT,(WPARAM)j,(LPARAM)value);
                    if(!reported && j<12)printf("MULTI_OLD_ITEM %ls\n",value);
                    for(unsigned k=0;k<2;++k)if(!_wcsicmp(value,names[k])){if(SendMessageW(list,LB_SETSEL,TRUE,j)!=LB_ERR)++found;}}
                InterlockedExchange(&listed,found);
                reported=TRUE;
                if(found==2){SendMessageW(window,WM_COMMAND,MAKEWPARAM(lst1,LBN_SELCHANGE),(LPARAM)list);InterlockedExchange(&posted,1);}
            }else if(explorer && !layout_old && !posted && i>20 && i%10==0){
                WCHAR input[MAX_PATH*2+8];swprintf(input,ARRAYSIZE(input),L"\"%ls\" \"%ls\"",full_names[0],full_names[1]);
                EnumChildWindows(window,find_edit,0);
                printf("MULTI_EDIT_ENUM_DONE present=%d\n",filename_edit!=NULL);
                DWORD_PTR set=0;
                if(filename_edit && SendMessageTimeoutW(filename_edit,WM_SETTEXT,0,(LPARAM)input,SMTO_ABORTIFHUNG|SMTO_BLOCK,500,&set) && set)InterlockedExchange(&posted,1);
                printf("MULTI_SET_DONE posted=%ld error=%lu\n",posted,GetLastError());
            }
            if(i>20 && i%10==0 && posted){HWND button=GetDlgItem(window,cancel_mode?IDCANCEL:IDOK);if(button)PostMessageW(button,BM_CLICK,0,0);}
            if(i==90)PostMessageW(window,WM_COMMAND,IDCANCEL,0);
        }
        if(seen&&!IsWindow(window))return 0;
        Sleep(50);
    }
    if(window&&IsWindow(window))PostMessageW(window,WM_CLOSE,0,0);
    return 1;
}
static UINT_PTR CALLBACK old_hook(HWND w,UINT m,WPARAM p,LPARAM l)
{
    (void)l;
    if(!supplement)return 0;
    if(m==WM_INITDIALOG)PostMessageW(w,WM_APP+23,0,0);
    if(m==WM_APP+23){HWND list=GetDlgItem(w,lst1);LONG count=0;
        if(list)for(unsigned i=0;i<2;++i)if(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)names[i])!=LB_ERR)++count;
        InterlockedExchange(&inserted,count);printf("MULTI_SUPPLEMENT inserted=%ld\n",count);return 1;}
    if(m==WM_COMMAND && LOWORD(p)==IDOK){HWND list=GetDlgItem(w,lst1);LRESULT count=list?SendMessageW(list,LB_GETSELCOUNT,0,0):LB_ERR;
        if(count==2){int indices[2];LRESULT got=SendMessageW(list,LB_GETSELITEMS,2,(LPARAM)indices);
            if(got==2){BOOL ok=TRUE;for(unsigned i=0;i<2;++i){LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)indices[i],0);
                    if(n<0||n>=MAX_PATH || SendMessageW(list,LB_GETTEXT,(WPARAM)indices[i],(LPARAM)chosen[i])==LB_ERR)ok=FALSE;}
                if(ok)InterlockedExchange(&captured,2);}}
        printf("MULTI_SUPPLEMENT_CAPTURE count=%lld captured=%ld\n",(long long)count,captured);
        if(captured==2){EndDialog(w,IDOK);return 1;}}
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);
    if(argc!=5 && argc!=6)return 2;
    BOOL small=argc==6 && (!wcscmp(argv[5],L"small")||!wcscmp(argv[5],L"supplement-small"));cancel_mode=argc==6 && !wcscmp(argv[5],L"supplement-cancel");supplement=argc==6 && (!wcscmp(argv[5],L"supplement")||!wcscmp(argv[5],L"supplement-small")||cancel_mode);if(argc==6 && !small && !supplement)return 2;
    explorer=!wcscmp(argv[1],L"explorer");BOOL wide=!wcscmp(argv[2],L"wide"),long_names=!wcscmp(argv[3],L"long");
    if(!explorer && wcscmp(argv[1],L"old"))return 2;
    if(!wide && wcscmp(argv[2],L"ansi"))return 2;
    if(!long_names && wcscmp(argv[3],L"short"))return 2;
    names[0]=long_names?L"alpha long.txt":L"ONE.TXT";names[1]=long_names?L"beta long.txt":L"TWO.TXT";
    WCHAR root[MAX_PATH],dir[MAX_PATH],file1[MAX_PATH],file2[MAX_PATH],initial_cwd[MAX_PATH];
    DWORD cwd_length=GetCurrentDirectoryW(ARRAYSIZE(initial_cwd),initial_cwd);
    if(!cwd_length||cwd_length>=ARRAYSIZE(initial_cwd))return 3;
    DWORD root_length=GetFullPathNameW(argv[4],ARRAYSIZE(root),root,NULL);
    if(!root_length||root_length>=ARRAYSIZE(root))return 3;
    if(!GetTempFileNameW(root,L"m7",0,dir)||!DeleteFileW(dir)||!CreateDirectoryW(dir,NULL))return 3;
    if(swprintf(file1,MAX_PATH,L"%ls\\%ls",dir,names[0])<0||swprintf(file2,MAX_PATH,L"%ls\\%ls",dir,names[1])<0)return 3;
    lstrcpynW(full_names[0],file1,MAX_PATH);lstrcpynW(full_names[1],file2,MAX_PATH);
    HANDLE f=CreateFileW(file1,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);
    f=CreateFileW(file2,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);
    WIN32_FIND_DATAW info1={0},info2={0};HANDLE find1=FindFirstFileW(file1,&info1),find2=FindFirstFileW(file2,&info2);
    BOOL aliases_known=find1!=INVALID_HANDLE_VALUE && find2!=INVALID_HANDLE_VALUE;
    if(find1!=INVALID_HANDLE_VALUE && find2!=INVALID_HANDLE_VALUE)printf("MULTI_ALIASES first=%ls second=%ls\n",info1.cAlternateFileName,info2.cAlternateFileName);
    if(find1!=INVALID_HANDLE_VALUE)FindClose(find1);
    if(find2!=INVALID_HANDLE_VALUE)FindClose(find2);
    printf("MULTI_FIXTURE dir=%ls\n",dir);
    gui_thread=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observer,NULL,0,NULL);if(!worker)return 5;
    BOOL result;DWORD error;WCHAR result_w[8192]=L"";char result_a[8192]={0};WORD offset=0;
    DWORD flags=OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|(explorer?OFN_EXPLORER:OFN_ENABLEHOOK);
    if(wide){OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrTitle=title;o.lpstrInitialDir=dir;o.lpstrFile=result_w;o.nMaxFile=small?8:ARRAYSIZE(result_w);o.lpstrFilter=L"All\0*.*\0\0";o.Flags=flags;if(!explorer)o.lpfnHook=old_hook;
        result=GetOpenFileNameW(&o);error=CommDlgExtendedError();offset=o.nFileOffset;
    }else{char dir_a[MAX_PATH];if(!WideCharToMultiByte(CP_ACP,0,dir,-1,dir_a,ARRAYSIZE(dir_a),NULL,NULL))return 6;
        OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrTitle="Win16S Win32 multi probe";o.lpstrInitialDir=dir_a;o.lpstrFile=result_a;o.nMaxFile=small?8:ARRAYSIZE(result_a);o.lpstrFilter="All\0*.*\0\0";o.Flags=flags;if(!explorer)o.lpfnHook=old_hook;
        result=GetOpenFileNameA(&o);error=CommDlgExtendedError();offset=o.nFileOffset;}
    DWORD wait=WaitForSingleObject(worker,8000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&code);CloseHandle(worker);
    BOOL native_result=result;
    if(supplement && result && captured==2){
        if(wide){size_t a=wcslen(dir),b=wcslen(chosen[0]),c=wcslen(chosen[1]),need=a+b+c+4;
            if(small){WORD required=(WORD)need;memcpy(result_w,&required,sizeof(required));result=FALSE;error=FNERR_BUFFERTOOSMALL;}
            else{wcscpy(result_w,dir);wcscpy(result_w+a+1,chosen[0]);wcscpy(result_w+a+b+2,chosen[1]);result_w[need-1]=0;offset=(WORD)(a+1);}}
        else{char directory[MAX_PATH],first[MAX_PATH],second[MAX_PATH];
            int a=WideCharToMultiByte(CP_ACP,0,dir,-1,directory,MAX_PATH,NULL,NULL),b=WideCharToMultiByte(CP_ACP,0,chosen[0],-1,first,MAX_PATH,NULL,NULL),c=WideCharToMultiByte(CP_ACP,0,chosen[1],-1,second,MAX_PATH,NULL,NULL);
            if(a<=0||b<=0||c<=0){result=FALSE;error=FNERR_INVALIDFILENAME;}
            else{size_t need=(size_t)a+(size_t)b+(size_t)c+1;
                if(small){WORD required=(WORD)need;memcpy(result_a,&required,sizeof(required));result=FALSE;error=FNERR_BUFFERTOOSMALL;}
                else{memcpy(result_a,directory,(size_t)a);memcpy(result_a+a,first,(size_t)b);memcpy(result_a+a+b,second,(size_t)c);result_a[need-1]=0;offset=(WORD)a;}}}
    }
    if(wide){printf("MULTI_RESULT_W first=%ls offset=%u\n",result_w,offset);if(explorer&&result)printf("MULTI_NAMES_W a=%ls b=%ls\n",result_w+wcslen(result_w)+1,result_w+wcslen(result_w)+1+wcslen(result_w+wcslen(result_w)+1)+1);}
    else{printf("MULTI_RESULT_A first=%s offset=%u\n",result_a,offset);if(explorer&&result)printf("MULTI_NAMES_A a=%s b=%s\n",result_a+strlen(result_a)+1,result_a+strlen(result_a)+1+strlen(result_a+strlen(result_a)+1)+1);}
    WORD required=0;if(small)memcpy(&required,wide?(void *)result_w:(void *)result_a,sizeof(required));
    BOOL valid=FALSE;
    if(small)valid=!result && error==FNERR_BUFFERTOOSMALL && required>8;
    else if((explorer||supplement) && result){
        if(wide){WCHAR *first=result_w+wcslen(result_w)+1,*second=first+wcslen(first)+1;valid=!_wcsicmp(result_w,dir)&&!_wcsicmp(first,names[0])&&!_wcsicmp(second,names[1])&&!second[wcslen(second)+1]&&offset==(WORD)(first-result_w);}
        else{char dir_a[MAX_PATH],first_a[MAX_PATH],second_a[MAX_PATH];WideCharToMultiByte(CP_ACP,0,dir,-1,dir_a,MAX_PATH,NULL,NULL);WideCharToMultiByte(CP_ACP,0,names[0],-1,first_a,MAX_PATH,NULL,NULL);WideCharToMultiByte(CP_ACP,0,names[1],-1,second_a,MAX_PATH,NULL,NULL);char *first=result_a+strlen(result_a)+1,*second=first+strlen(first)+1;valid=!_stricmp(result_a,dir_a)&&!_stricmp(first,first_a)&&!_stricmp(second,second_a)&&!second[strlen(second)+1]&&offset==(WORD)(first-result_a);}
    }else if(!explorer && !long_names && result)valid=wide?!_wcsicmp(result_w+offset,L"ONE.TXT TWO.TXT"):!_stricmp(result_a+offset,"ONE.TXT TWO.TXT");
    else if(!explorer && long_names && aliases_known && !info1.cAlternateFileName[0] && !info2.cAlternateFileName[0])valid=!result && listed==0;
    if(supplement)valid=cancel_mode?(inserted==2 && listed==2 && !native_result && error==0):(valid && inserted==2 && captured==2 && native_result);
    BOOL cwd_restored=SetCurrentDirectoryW(initial_cwd);
    BOOL clean1=DeleteFileW(file1),clean2=DeleteFileW(file2),clean_dir=RemoveDirectoryW(dir);
    printf("WIN32_MULTI_CWD restored=%d\n",cwd_restored);
    printf("WIN32_MULTI bits=%u explorer=%d wide=%d long=%d seen=%ld old=%ld listed=%ld posted=%ld result=%d error=%lu wait=%lu worker=%lu cleanup=%d%d%d\n",(unsigned)(sizeof(void*)*8),explorer,wide,long_names,seen,layout_old,listed,posted,result,error,wait,code,clean1,clean2,clean_dir);
    printf("WIN32_MULTI_CONTRACT small=%d required=%u valid=%d\n",small,required,valid);
    if(supplement)printf("MULTI_SUPPLEMENT_RESULT inserted=%ld captured=%ld system_result=%d\n",inserted,captured,native_result);
    return seen&&wait==WAIT_OBJECT_0&&code==0&&cwd_restored&&clean1&&clean2&&clean_dir&&valid?0:1;
}
