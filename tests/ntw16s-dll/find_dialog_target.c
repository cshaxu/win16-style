#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdio.h>
#include <wchar.h>
static UINT find_message;
static int finds, replacements, terms;
static LRESULT CALLBACK owner_proc(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    if(message==find_message){
        FINDREPLACEW *record=(FINDREPLACEW *)lparam;
        if(record->Flags & FR_FINDNEXT)++finds;
        if(record->Flags & (FR_REPLACE|FR_REPLACEALL))++replacements;
        if(record->Flags & FR_DIALOGTERM)++terms;
        return 0;
    }
    return DefWindowProcW(window,message,wparam,lparam);
}
static BOOL CALLBACK list_child(HWND child,LPARAM ignored)
{(void)ignored;WCHAR text[100];GetWindowTextW(child,text,100);printf("FIND_CHILD id=%d text=%ls\n",GetDlgCtrlID(child),text);return TRUE;}
static void pump(void)
{
    MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3)return 2;
    BOOL replace=argv[1][0]==L'r',ansi=argv[2][0]==L'a';
    find_message=RegisterWindowMessageW(FINDMSGSTRINGW);
    WNDCLASSW wc={0};wc.lpfnWndProc=owner_proc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"Win16S.FindProbe.Owner";
    if(!RegisterClassW(&wc))return 3;
    HWND owner=CreateWindowExW(0,wc.lpszClassName,L"Win16S Find Owner",WS_OVERLAPPEDWINDOW,0,0,400,300,NULL,NULL,wc.hInstance,NULL);
    if(!owner)return 4;
    WCHAR find_w[64]=L"needle",replace_w[64]=L"replacement";char find_a[64]="needle",replace_a[64]="replacement";
    FINDREPLACEW fw={0};FINDREPLACEA fa={0};HWND dialog;
    if(ansi){fa.lStructSize=sizeof(fa);fa.hwndOwner=owner;fa.lpstrFindWhat=find_a;fa.wFindWhatLen=64;fa.lpstrReplaceWith=replace_a;fa.wReplaceWithLen=64;fa.Flags=FR_DOWN;
        dialog=replace?ReplaceTextA(&fa):FindTextA(&fa);
    }else{fw.lStructSize=sizeof(fw);fw.hwndOwner=owner;fw.lpstrFindWhat=find_w;fw.wFindWhatLen=64;fw.lpstrReplaceWith=replace_w;fw.wReplaceWithLen=64;fw.Flags=FR_DOWN;
        dialog=replace?ReplaceTextW(&fw):FindTextW(&fw);
    }
    HMODULE module=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    BOOL (__cdecl *attached)(HWND)=module?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    BOOL frame=dialog && attached && attached(dialog);
    HWND edit=dialog?GetDlgItem(dialog,edt1):NULL;
    BOOL edit_attached=edit && attached && attached(edit);
    if(dialog){ShowWindow(dialog,SW_SHOW);pump();EnumChildWindows(dialog,list_child,0);
        HWND next=GetDlgItem(dialog,IDOK);
        if(next)SendMessageW(dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)next);
        pump();
        if(replace){
            HWND one=GetDlgItem(dialog,psh1),all=GetDlgItem(dialog,psh2);
            if(one)SendMessageW(dialog,WM_COMMAND,MAKEWPARAM(psh1,BN_CLICKED),(LPARAM)one);
            if(all)SendMessageW(dialog,WM_COMMAND,MAKEWPARAM(psh2,BN_CLICKED),(LPARAM)all);
            pump();
        }
        SendMessageW(dialog,WM_CLOSE,0,0);pump();}
    BOOL gone=!dialog || !IsWindow(dialog);
    printf("FIND_DIALOG bits=%u ansi=%d replace=%d frame=%d edit=%d find=%d replace_count=%d term=%d gone=%d\n",
        (unsigned)(sizeof(void*)*8),ansi,replace,frame,edit_attached,finds,replacements,terms,gone);
    DestroyWindow(owner);UnregisterClassW(wc.lpszClassName,wc.hInstance);
    return frame && edit_attached && finds==1 && (!replace || replacements==2) && terms==1 && gone?0:1;
}
