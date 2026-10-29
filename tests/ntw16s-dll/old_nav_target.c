#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <wchar.h>
static UINT lbsel;static WCHAR initial[MAX_PATH],sub[MAX_PATH],file[MAX_PATH];static LONG phase,changed,vetoed,unchanged,selection;static int nav_index=-1;
static void current(const char *tag){WCHAR cwd[MAX_PATH]=L"";GetCurrentDirectoryW(MAX_PATH,cwd);printf("NAV_%s cwd=%ls\n",tag,cwd);}
static void CALLBACK timer(HWND w,UINT msg,UINT_PTR id,DWORD tick)
{
    (void)msg;(void)id;(void)tick;HWND dirs=GetDlgItem(w,1121),files=GetDlgItem(w,1120);
    if(phase==0){LRESULT n=SendMessageW(dirs,LB_GETCOUNT,0,0);printf("NAV_DIRCOUNT %lld\n",(long long)n);
        for(LRESULT i=0;i<n;++i){WCHAR text[512]=L"";LRESULT len=SendMessageW(dirs,LB_GETTEXTLEN,i,0);if(len<0||len>=511)continue;
            SendMessageW(dirs,LB_GETTEXT,i,(LPARAM)text);printf("NAV_ENTRY i=%lld text=%ls data=%lld\n",(long long)i,text,(long long)SendMessageW(dirs,LB_GETITEMDATA,i,0));
            if(wcsstr(text,L"alpha")||wcsstr(text,L"ALPHA"))nav_index=(int)i;}
        LRESULT f=SendMessageW(files,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"sample.txt");
        printf("NAV_FILEINDEX %lld dirindex=%d\n",(long long)f,nav_index);
        if(f!=LB_ERR){SendMessageW(files,LB_SETCURSEL,(WPARAM)f,0);SendMessageW(w,WM_COMMAND,MAKEWPARAM(1120,LBN_SELCHANGE),(LPARAM)files);}phase=1;
    }else if(phase==1){if(nav_index>=0){SendMessageW(dirs,LB_SETCURSEL,(WPARAM)nav_index,0);WCHAR dirpath[MAX_PATH]=L"";
            BOOL dirflag=DlgDirSelectExW(w,dirpath,MAX_PATH,1121);printf("NAV_DIRSELECT isdir=%d path=%ls\n",dirflag,dirpath);current("BEFORE_SEND");
            SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_DBLCLK),(LPARAM)dirs);WCHAR after[MAX_PATH]=L"";GetCurrentDirectoryW(MAX_PATH,after);unchanged=!lstrcmpiW(after,initial);
            printf("NAV_VETO unchanged=%ld\n",unchanged);SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_DBLCLK),(LPARAM)dirs);current("AFTER_SEND");}
        phase=2;
    }else{WCHAR cwd[MAX_PATH]=L"";GetCurrentDirectoryW(MAX_PATH,cwd);changed=!lstrcmpiW(cwd,sub);current("AFTER_TIMER");KillTimer(w,911);PostMessageW(w,WM_COMMAND,IDCANCEL,0);phase=3;}
}
static UINT_PTR CALLBACK hook(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{
    if(msg==WM_INITDIALOG){current("INIT");SetTimer(w,911,150,timer);}
    if(msg==WM_COMMAND){printf("NAV_COMMAND id=%u code=%u\n",LOWORD(wp),HIWORD(wp));current("COMMAND");
        if(LOWORD(wp)==1121 && HIWORD(wp)==LBN_DBLCLK && !vetoed){vetoed=1;return 1;}}
    if(msg==lbsel){if(wp==1120)++selection;printf("NAV_LBSEL id=%llu index=%u code=%u\n",(unsigned long long)wp,LOWORD(lp),HIWORD(lp));current("LBSEL");}
    return 0;
}
int wmain(void)
{
    WCHAR temp[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,initial)||!DeleteFileW(initial)||!CreateDirectoryW(initial,NULL))return 2;
    if(swprintf(sub,MAX_PATH,L"%ls\\alpha",initial)<0||!CreateDirectoryW(sub,NULL)||swprintf(file,MAX_PATH,L"%ls\\sample.txt",initial)<0)return 3;
    HANDLE h=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(h==INVALID_HANDLE_VALUE)return 4;CloseHandle(h);
    lbsel=RegisterWindowMessageW(LBSELCHSTRINGW);WCHAR path[MAX_PATH]=L"";OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrInitialDir=initial;
    o.lpstrTitle=L"Win16S legacy navigation probe";o.lpstrFilter=L"All\0*.*\0\0";o.lpstrFile=path;o.nMaxFile=MAX_PATH;
    o.Flags=OFN_ENABLEHOOK|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;o.lpfnHook=hook;BOOL result=GetOpenFileNameW(&o);DWORD error=CommDlgExtendedError();
    current("AFTER_CLOSE");DeleteFileW(file);RemoveDirectoryW(sub);RemoveDirectoryW(initial);
    printf("NAV_RESULT bits=%u result=%d error=%lu phase=%ld changed=%ld veto=%ld unchanged=%ld selection=%ld\n",(unsigned)(sizeof(void*)*8),result,error,phase,changed,vetoed,unchanged,selection);return !result&&!error&&phase==3&&changed&&vetoed&&unchanged&&selection?0:1;
}
