#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
static DWORD ui; static HWND dialog; static volatile LONG seen,done; static BOOL old,cancel,rewrite,small,listmode;static volatile LONG rewritten,entered;static WCHAR injected[MAX_PATH];
static const WCHAR title[]=L"Win16S native link flag";
static UINT_PTR CALLBACK old_hook(HWND w,UINT m,WPARAM p,LPARAM l){(void)l;if(m==WM_COMMAND&&LOWORD(p)==IDOK){WCHAR value[MAX_PATH]=L"",cwd[MAX_PATH]=L"";GetDlgItemTextW(w,edt1,value,MAX_PATH);GetCurrentDirectoryW(MAX_PATH,cwd);printf("OLD_CONFIRM edit=[%ls] cwd=[%ls]\n",value,cwd);
    if(rewrite){HRESULT initialized=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);IShellLinkW *link=NULL;IPersistFile *persist=NULL;WCHAR target[MAX_PATH]=L"";
        HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
        if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
        if(SUCCEEDED(hr))hr=IPersistFile_Load(persist,value,STGM_READ);
        if(SUCCEEDED(hr))hr=IShellLinkW_GetPath(link,target,MAX_PATH,NULL,0);
        if(hr==S_OK&&target[0]&&SetDlgItemTextW(w,edt1,target))InterlockedExchange(&rewritten,1);
        if(persist)IPersistFile_Release(persist);
        if(link)IShellLinkW_Release(link);
        if(SUCCEEDED(initialized))CoUninitialize();
    }}return 0;}
static BOOL CALLBACK find_dialog(HWND w,LPARAM p){(void)p;WCHAR t[80];GetWindowTextW(w,t,80);if(!IsWindowVisible(w)||lstrcmpW(t,title))return TRUE;dialog=w;old=GetDlgItem(w,1121)!=NULL;seen=1;return FALSE;}
static DWORD WINAPI observe(void *p){(void)p;for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(ui,find_dialog,0);if(seen&&small&&!entered&&SetDlgItemTextW(dialog,edt1,injected))InterlockedExchange(&entered,1);if(seen&&listmode&&!entered){HWND list=GetDlgItem(dialog,lst1);LRESULT item=list?SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"shortcut.lnk"):LB_ERR;if(item!=LB_ERR&&SendMessageW(list,LB_SETCURSEL,(WPARAM)item,0)!=LB_ERR){SendMessageW(dialog,WM_COMMAND,MAKEWPARAM(lst1,LBN_SELCHANGE),(LPARAM)list);InterlockedExchange(&entered,1);}}if(seen&&i>10&&i%10==0){int id=cancel?IDCANCEL:IDOK;PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),(LPARAM)GetDlgItem(dialog,id));}if(done)return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}
int wmain(int argc,WCHAR **argv){
    if(argc!=5&&argc!=6)return 2;
    BOOL wide=!lstrcmpW(argv[1],L"wide"),legacy=!lstrcmpW(argv[2],L"old"),keep=!lstrcmpW(argv[3],L"keep");
    cancel=argc==6&&!lstrcmpW(argv[5],L"cancel");
    small=argc==6&&!lstrcmpW(argv[5],L"small");rewrite=(small&&legacy)||(argc==6&&!lstrcmpW(argv[5],L"rewrite"));
    listmode=argc==6&&!lstrcmpW(argv[5],L"list");
    WCHAR base[MAX_PATH],root[MAX_PATH],target[MAX_PATH],link[MAX_PATH],file[MAX_PATH];char afile[MAX_PATH],aroot[MAX_PATH];
    DWORD n=GetFullPathNameW(argv[4],MAX_PATH,base,NULL);
    if(!n||n>=MAX_PATH||!GetTempFileNameW(base,L"f15",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 3;
    if(swprintf(target,MAX_PATH,L"%ls\\target.txt",root)<0||swprintf(link,MAX_PATH,L"%ls\\shortcut.lnk",root)<0)return 4;
    HANDLE f=CreateFileW(target,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return 5;CloseHandle(f);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 6;IShellLinkW *sl=NULL;IPersistFile *pf=NULL;
    hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&sl);
    if(FAILED(hr)||FAILED(IShellLinkW_SetPath(sl,target))||FAILED(IShellLinkW_QueryInterface(sl,&IID_IPersistFile,(void **)&pf))||FAILED(IPersistFile_Save(pf,link,TRUE)))return 7;
    IPersistFile_Release(pf);IShellLinkW_Release(sl);CoUninitialize();lstrcpynW(file,link,MAX_PATH);lstrcpynW(injected,link,MAX_PATH);
    if(!WideCharToMultiByte(CP_ACP,0,root,-1,aroot,MAX_PATH,NULL,NULL)||!WideCharToMultiByte(CP_ACP,0,link,-1,afile,MAX_PATH,NULL,NULL))return 8;
    ui=GetCurrentThreadId();HANDLE th=CreateThread(NULL,0,observe,NULL,0,NULL);if(!th)return 9;
    DWORD flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_LONGNAMES|(keep?OFN_NODEREFERENCELINKS:0)|(legacy?OFN_ENABLEHOOK:OFN_EXPLORER);BOOL result;WORD needed=0,offset=0,extension=0;
    if(small||listmode){file[0]=0;afile[0]=0;}
    if(wide){OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrTitle=title;o.lpstrInitialDir=root;o.lpstrFile=file;o.nMaxFile=small?2:MAX_PATH;o.lpstrFilter=L"All\0*.*\0\0";o.Flags=flags;o.lpfnHook=legacy?old_hook:NULL;result=GetOpenFileNameW(&o);offset=o.nFileOffset;extension=o.nFileExtension;if(small)memcpy(&needed,file,sizeof(needed));}
    else{OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrTitle="Win16S native link flag";o.lpstrInitialDir=aroot;o.lpstrFile=afile;o.nMaxFile=small?2:MAX_PATH;o.lpstrFilter="All\0*.*\0\0";o.Flags=flags;o.lpfnHook=legacy?old_hook:NULL;result=GetOpenFileNameA(&o);offset=o.nFileOffset;extension=o.nFileExtension;if(small)memcpy(&needed,afile,sizeof(needed));else MultiByteToWideChar(CP_ACP,0,afile,-1,file,MAX_PATH);}
    DWORD error=CommDlgExtendedError();InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(th,10000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(th,&code);CloseHandle(th);
    BOOL link_result=!lstrcmpiW(file,link),target_result=!lstrcmpiW(file,target);BOOL cl=DeleteFileW(link),ct=DeleteFileW(target),cr=RemoveDirectoryW(root);
    printf("LINK_PATH file=[%ls] link=[%ls] target=[%ls]\n",file,link,target);
    printf("LINK_CANCEL requested=%d\n",cancel);
    printf("LINK_REWRITE requested=%d done=%ld\n",rewrite,rewritten);
    if(listmode)printf("LINK_LIST entered=%ld\n",entered);
    if(small)printf("LINK_SMALL entered=%ld needed=%u targetlen=%zu\n",entered,needed,wcslen(target));
    const WCHAR *slash=wcsrchr(file,L'\\'),*dot=wcsrchr(file,L'.');BOOL offsets=!result||(slash&&dot&&offset==(WORD)(slash-file+1)&&extension==(WORD)(dot-file+1));
    printf("LINK_OFFSETS offset=%u extension=%u valid=%d\n",offset,extension,offsets);
    printf("LINK_FLAG bits=%u wide=%d legacy=%d keep=%d seen=%ld old=%d ok=%d error=%lu link=%d target=%d wait=%lu worker=%lu cleanup=%d%d%d\n",(unsigned)(sizeof(void*)*8),wide,legacy,keep,seen,old,result,error,link_result,target_result,wait,code,cl,ct,cr);
    return seen&&offsets&&(!listmode||entered)&&(small?(!result&&error==0x3003&&entered&&needed>wcslen(target)):(cancel?!result:(result&&(rewrite?target_result:(keep||legacy?link_result:target_result)))))&&(!rewrite||rewritten)&&(small||!error)&&wait==WAIT_OBJECT_0&&!code&&cl&&ct&&cr?0:1;
}
