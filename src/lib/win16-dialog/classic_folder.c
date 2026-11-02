/* Classic folder template behavior; the caller owns thread and unwind state. */
#include "classic_folder.h"
#include "folder_create.h"
#include "file_helpers.h"
#include <dlgs.h>
#include <wchar.h>
#include <stdio.h>
#define NEW_FOLDER_BUTTON 0x5717u

static void classic_create_folder(HWND owner,win16_dialog_classic_folder_context *ctx)
{
    WCHAR path[MAX_PATH];
    if(win16_dialog_create_folder(owner,ctx?ctx->classic_current:NULL,path,ARRAYSIZE(path)) && IsWindow(owner)){
        HWND edit=GetDlgItem(owner,edt1);WCHAR previous[MAX_PATH]=L"";
        if(edit){GetWindowTextW(edit,previous,ARRAYSIZE(previous));SetWindowTextW(edit,path);
            SendMessageW(owner,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(owner,IDOK));
            if(IsWindow(owner) && IsWindow(edit))SetWindowTextW(edit,
                !lstrcmpiW(ctx->classic_current,path)?L"[Select current folder]":previous);}
    }
}

UINT_PTR win16_dialog_classic_folder_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam,
    win16_dialog_classic_folder_context *active,void (*chrome)(HWND))
{
    if(message==WM_INITDIALOG){
        chrome(window);
        OPENFILENAMEW *ofn=(OPENFILENAMEW *)lparam;
        win16_dialog_classic_folder_context *ctx=(win16_dialog_classic_folder_context *)ofn->lCustData;
        SetPropW(window,L"Win16s.FolderBridge",ctx);
        if(ctx)(void)ntw16s_old_dialog_folder(window,ctx->classic_current,ARRAYSIZE(ctx->classic_current));
        if(ctx && ctx->experimental)ctx->classic_window=window;
        if(ctx && ctx->ok_label)SetDlgItemTextW(window,IDOK,ctx->ok_label);
        if(ctx && ctx->cancel_label)SetDlgItemTextW(window,IDCANCEL,ctx->cancel_label);
        PostMessageW(window,ctx->run_message,0,0);
        HWND cancel=GetDlgItem(window,IDCANCEL);
        HWND readonly=GetDlgItem(window,chx1);
        if(readonly)ShowWindow(readonly,SW_HIDE);
        RECT rc;
        if(cancel && GetWindowRect(cancel,&rc)){
            MapWindowPoints(NULL,window,(POINT *)&rc,2);
            HWND button=CreateWindowExW(0,L"BUTTON",L"New Folder...",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
                rc.left,rc.bottom+8,rc.right-rc.left,rc.bottom-rc.top,window,
                (HMENU)(UINT_PTR)NEW_FOLDER_BUTTON,GetModuleHandleW(NULL),NULL);
            if(button)SendMessageW(button,WM_SETFONT,SendMessageW(cancel,WM_GETFONT,0,0),TRUE);
        }
    }else if(active && message==active->run_message){
        SetDlgItemTextW(window,1152,L"[Select current folder]");
        SetDlgItemTextW(window,1090,active->file_label?active->file_label:L"Folder selection:");
        return 1;
    }else if(message==WM_COMMAND && LOWORD(wparam)==IDOK){
        /* The old file template replaces the name with its default wildcard
         * after navigating to a typed directory. In a folder picker, OK on
         * that default (or an empty name) selects the current directory. */
        WCHAR filename[MAX_PATH]=L"";
        GetDlgItemTextW(window,1152,filename,ARRAYSIZE(filename));
        if(!filename[0] || !lstrcmpW(filename,L"*.*"))
            SetDlgItemTextW(window,1152,L"[Select current folder]");
    }else if(message==WM_COMMAND && LOWORD(wparam)==NEW_FOLDER_BUTTON){
        win16_dialog_classic_folder_context *ctx=GetPropW(window,L"Win16s.FolderBridge");
        if(ctx)classic_create_folder(window,ctx);
        return 1;
    }
    if(message>=0xc000 && message<=0xffff){
        UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);
        if(selection && message==selection && (wparam==lst2 || wparam==cmb2)){
            win16_dialog_classic_folder_context *ctx=GetPropW(window,L"Win16s.FolderBridge");
            if(ctx){WCHAR folder[MAX_PATH];
                if(ntw16s_old_dialog_folder(window,folder,ARRAYSIZE(folder)))lstrcpynW(ctx->classic_current,folder,ARRAYSIZE(ctx->classic_current));
                else ctx->classic_current[0]=0;}
        }
    }
    win16_dialog_classic_folder_context *observer_ctx=message==WM_INITDIALOG && lparam?
        (win16_dialog_classic_folder_context *)((OPENFILENAMEW *)lparam)->lCustData:GetPropW(window,L"Win16s.FolderBridge");
    if(observer_ctx && observer_ctx->experimental && observer_ctx->observer){UINT_PTR handled=observer_ctx->observer(window,message,wparam,lparam,observer_ctx->observer_context);
        if(handled)return handled;}
    if(message==WM_NCDESTROY){
        if(observer_ctx && observer_ctx->experimental && observer_ctx->classic_window==window)observer_ctx->classic_window=NULL;
        RemovePropW(window,L"Win16s.FolderBridge");
    }
    return 0;
}

HRESULT win16_dialog_classic_folder_prepare(win16_dialog_classic_folder_context *ctx,OPENFILENAMEW *ofn,WCHAR *file,
    HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,LPOFNHOOKPROC hook)
{
    ctx->ok_label=ok_label;ctx->cancel_label=cancel_label;ctx->file_label=file_label;
    ctx->run_message=RegisterWindowMessageW(L"Win16Style.T20.FolderBridge");
    if(!ctx->run_message)return HRESULT_FROM_WIN32(GetLastError());
    lstrcpynW(file,L"[Select current folder]",MAX_PATH);
    if(initial && *initial && swprintf(file,MAX_PATH,L"%ls%ls[Select current folder]",initial,
        initial[wcslen(initial)-1]==L'\\'?L"":L"\\")<0)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    ZeroMemory(ofn,sizeof(*ofn));ofn->lStructSize=sizeof(*ofn);ofn->hwndOwner=owner;
    ofn->lpstrTitle=title?title:L"Select Folder";ofn->lpstrFile=file;ofn->nMaxFile=MAX_PATH;
    ofn->lpstrFilter=L"All files\0*.*\0\0";ofn->lpstrInitialDir=initial;
    ofn->Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_LONGNAMES;
    ofn->lpfnHook=hook;ofn->lCustData=(LPARAM)ctx;
    return S_OK;
}
HRESULT win16_dialog_classic_folder_finish(BOOL chosen,DWORD error,WCHAR *file,PWSTR result,DWORD capacity)
{
    if(!chosen)return error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);
    WCHAR *slash=wcsrchr(file,L'\\');if(!slash)return E_FAIL;
    if(slash==file+2)slash[1]=0;else *slash=0;
    if(wcslen(file)>=capacity)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    wcscpy(result,file);return S_OK;
}
