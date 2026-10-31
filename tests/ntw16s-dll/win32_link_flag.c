#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <stdio.h>
#include <wchar.h>
static DWORD ui; static HWND dialog; static volatile LONG seen,done; static BOOL old,cancel;
static const WCHAR title[]=L"Win16S native link flag";
static UINT_PTR CALLBACK old_hook(HWND w,UINT m,WPARAM p,LPARAM l){(void)w;(void)m;(void)p;(void)l;return 0;}
static BOOL CALLBACK find_dialog(HWND w,LPARAM p){(void)p;WCHAR t[80];GetWindowTextW(w,t,80);if(!IsWindowVisible(w)||lstrcmpW(t,title))return TRUE;dialog=w;old=GetDlgItem(w,1121)!=NULL;seen=1;return FALSE;}
static DWORD WINAPI observe(void *p){(void)p;for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(ui,find_dialog,0);if(seen&&i>10&&i%10==0){int id=cancel?IDCANCEL:IDOK;PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),(LPARAM)GetDlgItem(dialog,id));}if(done)return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}
int wmain(int argc,WCHAR **argv){
    if(argc!=5&&argc!=6)return 2;
    BOOL wide=!lstrcmpW(argv[1],L"wide"),legacy=!lstrcmpW(argv[2],L"old"),keep=!lstrcmpW(argv[3],L"keep");
    cancel=argc==6&&!lstrcmpW(argv[5],L"cancel");
    WCHAR base[MAX_PATH],root[MAX_PATH],target[MAX_PATH],link[MAX_PATH],file[MAX_PATH];char afile[MAX_PATH],aroot[MAX_PATH];
    DWORD n=GetFullPathNameW(argv[4],MAX_PATH,base,NULL);
    if(!n||n>=MAX_PATH||!GetTempFileNameW(base,L"f15",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 3;
    if(swprintf(target,MAX_PATH,L"%ls\\target.txt",root)<0||swprintf(link,MAX_PATH,L"%ls\\shortcut.lnk",root)<0)return 4;
    HANDLE f=CreateFileW(target,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return 5;CloseHandle(f);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 6;IShellLinkW *sl=NULL;IPersistFile *pf=NULL;
    hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&sl);
    if(FAILED(hr)||FAILED(IShellLinkW_SetPath(sl,target))||FAILED(IShellLinkW_QueryInterface(sl,&IID_IPersistFile,(void **)&pf))||FAILED(IPersistFile_Save(pf,link,TRUE)))return 7;
    IPersistFile_Release(pf);IShellLinkW_Release(sl);lstrcpynW(file,link,MAX_PATH);
    if(!WideCharToMultiByte(CP_ACP,0,root,-1,aroot,MAX_PATH,NULL,NULL)||!WideCharToMultiByte(CP_ACP,0,link,-1,afile,MAX_PATH,NULL,NULL))return 8;
    ui=GetCurrentThreadId();HANDLE th=CreateThread(NULL,0,observe,NULL,0,NULL);if(!th)return 9;
    DWORD flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_LONGNAMES|(keep?OFN_NODEREFERENCELINKS:0)|(legacy?OFN_ENABLEHOOK:OFN_EXPLORER);BOOL result;
    if(wide){OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrTitle=title;o.lpstrInitialDir=root;o.lpstrFile=file;o.nMaxFile=MAX_PATH;o.lpstrFilter=L"All\0*.*\0\0";o.Flags=flags;o.lpfnHook=legacy?old_hook:NULL;result=GetOpenFileNameW(&o);}
    else{OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrTitle="Win16S native link flag";o.lpstrInitialDir=aroot;o.lpstrFile=afile;o.nMaxFile=MAX_PATH;o.lpstrFilter="All\0*.*\0\0";o.Flags=flags;o.lpfnHook=legacy?old_hook:NULL;result=GetOpenFileNameA(&o);MultiByteToWideChar(CP_ACP,0,afile,-1,file,MAX_PATH);}
    DWORD error=CommDlgExtendedError();InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(th,10000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(th,&code);CloseHandle(th);
    BOOL link_result=!lstrcmpiW(file,link),target_result=!lstrcmpiW(file,target);BOOL cl=DeleteFileW(link),ct=DeleteFileW(target),cr=RemoveDirectoryW(root);CoUninitialize();
    printf("LINK_PATH file=[%ls] link=[%ls] target=[%ls]\n",file,link,target);
    printf("LINK_CANCEL requested=%d\n",cancel);
    printf("LINK_FLAG bits=%u wide=%d legacy=%d keep=%d seen=%ld old=%d ok=%d error=%lu link=%d target=%d wait=%lu worker=%lu cleanup=%d%d%d\n",(unsigned)(sizeof(void*)*8),wide,legacy,keep,seen,old,result,error,link_result,target_result,wait,code,cl,ct,cr);
    return seen&&(cancel?!result:(result&&(keep||legacy?link_result:target_result)))&&!error&&wait==WAIT_OBJECT_0&&!code&&cl&&ct&&cr?0:1;
}
