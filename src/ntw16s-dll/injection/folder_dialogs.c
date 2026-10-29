/* Bridge a limited filesystem Shell browse request to the system's old file
 * dialog. Shell still owns the real callback, selection and returned PIDL.
 * Unsupported browse contracts always go directly to Shell. */
#include "intercept.h"
#include "startup_hooks.h"
#include "ntw16s-dll/controls.h"
#include <dlgs.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <wchar.h>
#include <stdio.h>

#define BRIDGE_INIT_TIMER 0x5715u
#define BRIDGE_TIMER 0x5716u
#define NEW_FOLDER_BUTTON 0x5717u
#define BRIDGE_FLAGS (BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_VALIDATE)

typedef struct folder_context {
    BFFCALLBACK callback;
    PCWSTR ok_label,cancel_label,file_label;
    LPARAM original_data;
    WCHAR selected[MAX_PATH], desired[MAX_PATH], requested[MAX_PATH];
    UINT run_message, attempts;
    DWORD init_tick, changed_tick;
    BOOL new_folder, showing;
    HWND callback_window;
} folder_context;
static _Thread_local folder_context *active;
static PIDLIST_ABSOLUTE (WINAPI *native_browse_a)(LPBROWSEINFOA);
static PIDLIST_ABSOLUTE (WINAPI *native_browse_w)(LPBROWSEINFOW);

/* Only system old-style templates enter here: preserve COMDLG32 item data,
 * owner-draw notifications and images, and apply our existing chrome. */
void ntw16s_old_dialog_chrome(HWND window)
{
    static int owner;
    const int list_ids[]={lst1,lst2};
    for(unsigned i=0;i<sizeof(list_ids)/sizeof(list_ids[0]);++i){
        HWND list=GetDlgItem(window,list_ids[i]);WCHAR cls[32];
        DWORD style=list?(DWORD)GetWindowLongPtrW(list,GWL_STYLE):0;
        if(list && GetParent(list)==window && GetClassNameW(list,cls,32) && !lstrcmpiW(cls,L"ListBox") &&
            (style&LBS_OWNERDRAWFIXED) && (style&LBS_HASSTRINGS) && !(style&LBS_NODATA) &&
            win16_style_grid_install_created(list,NULL,&owner,NULL) &&
            !win16_style_list_scrollbar_enable(list))win16_style_control_remove_owned(list,&owner);
    }
    HWND drives=GetDlgItem(window,cmb2);
    if(drives && GetParent(drives)==window)(void)win16_style_common_dialog_combo_install(drives,&owner);
}

void ntw16s_font_dialog_chrome(HWND window)
{
    static int owner;
    for(int id=cmb1;id<=cmb5;++id){
        HWND combo=GetDlgItem(window,id);
        if(combo && GetParent(combo)==window)(void)win16_style_common_dialog_combo_install(combo,&owner);
    }
}

static UINT_PTR CALLBACK old_folder_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    if(message==WM_INITDIALOG){
        ntw16s_old_dialog_chrome(window);
        OPENFILENAMEW *ofn=(OPENFILENAMEW *)lparam;
        folder_context *ctx=(folder_context *)ofn->lCustData;
        SetPropW(window,L"Win16s.FolderBridge",ctx);
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
    }else if(message==WM_COMMAND && LOWORD(wparam)==NEW_FOLDER_BUTTON){
        folder_context *ctx=GetPropW(window,L"Win16s.FolderBridge");
        if(ctx){ctx->new_folder=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);}
        return 1;
    }else if(message==WM_NCDESTROY)RemovePropW(window,L"Win16s.FolderBridge");
    return 0;
}
/* COM folder requests need a Shell item, not a fabricated PIDL. Reuse the
 * classic system template and return its selected directory to the caller. */
HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity)
{
    if(!result || !capacity)return E_INVALIDARG;
    result[0]=0;if(active)return HRESULT_FROM_WIN32(ERROR_BUSY);
    folder_context ctx={0};ctx.ok_label=ok_label;ctx.cancel_label=cancel_label;ctx.file_label=file_label;ctx.run_message=RegisterWindowMessageW(L"Win16Style.T20.FolderBridge");
    if(!ctx.run_message)return HRESULT_FROM_WIN32(GetLastError());
    WCHAR file[MAX_PATH]=L"[Select current folder]";
    if(initial && *initial && swprintf(file,MAX_PATH,L"%ls%ls[Select current folder]",initial,
        initial[wcslen(initial)-1]==L'\\'?L"":L"\\")<0)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=owner;
    ofn.lpstrTitle=title?title:L"Select Folder";ofn.lpstrFile=file;ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"All files\0*.*\0\0";ofn.lpstrInitialDir=initial;
    ofn.Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_LONGNAMES;
    ofn.lpfnHook=old_folder_hook;ofn.lCustData=(LPARAM)&ctx;
    active=&ctx;BOOL chosen=GetOpenFileNameW(&ofn);DWORD error=chosen?0:CommDlgExtendedError();active=NULL;
    if(ctx.new_folder){
        BROWSEINFOW browse={0};browse.hwndOwner=owner;browse.lpszTitle=title;
        browse.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_VALIDATE;
        active=&ctx;PIDLIST_ABSOLUTE pidl=SHBrowseForFolderW(&browse);active=NULL;
        if(!pidl)return HRESULT_FROM_WIN32(ERROR_CANCELLED);
        WCHAR path[MAX_PATH];BOOL ok=SHGetPathFromIDListW(pidl,path);CoTaskMemFree(pidl);
        if(!ok)return E_FAIL;
        if(wcslen(path)>=capacity)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        wcscpy(result,path);return S_OK;
    }
    if(!chosen)return error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);
    WCHAR *slash=wcsrchr(file,L'\\');if(!slash)return E_FAIL;
    if(slash==file+2)slash[1]=0;else *slash=0;
    if(wcslen(file)>=capacity)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    wcscpy(result,file);return S_OK;
}
static LRESULT CALLBACK shell_subclass(HWND window,UINT message,WPARAM wparam,LPARAM lparam,
    UINT_PTR identity,DWORD_PTR reference)
{
    folder_context *ctx=(folder_context *)reference;
    if(message==ctx->run_message){
        if(ctx->showing)return 0;
        ctx->showing=TRUE;
        WCHAR file[MAX_PATH]=L"[Select current folder]";
        const WCHAR *initial=ctx->requested[0]?ctx->requested:ctx->selected;
        if(initial[0] && swprintf(file,MAX_PATH,L"%ls%ls[Select current folder]",initial,
            initial[wcslen(initial)-1]==L'\\'?L"":L"\\")<0){
            ShowWindow(window,SW_SHOW);return 0;
        }
        OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=window;
        ofn.lpstrTitle=L"Select Folder";ofn.lpstrFile=file;ofn.nMaxFile=MAX_PATH;
        ofn.lpstrFilter=L"All files\0*.*\0\0";
        ofn.lpstrInitialDir=initial[0]?initial:NULL;
        ofn.Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_LONGNAMES;
        ofn.lpfnHook=old_folder_hook;ofn.lCustData=(LPARAM)ctx;
        BOOL accepted=GetOpenFileNameW(&ofn);
        if(ctx->new_folder){ShowWindow(window,SW_SHOW);SetForegroundWindow(window);return 0;}
        if(!accepted){
            if(CommDlgExtendedError()){ShowWindow(window,SW_SHOW);SetForegroundWindow(window);}
            else PostMessageW(window,WM_COMMAND,IDCANCEL,0);
            return 0;
        }
        WCHAR *separator=wcsrchr(file,L'\\');
        if(!separator){ShowWindow(window,SW_SHOW);return 0;}
        if(separator==file+2)separator[1]=0;else *separator=0;
        lstrcpynW(ctx->desired,file,MAX_PATH);
        if(lstrcmpiW(ctx->selected,ctx->desired))
            SendMessageW(window,BFFM_SETSELECTIONW,TRUE,(LPARAM)ctx->desired);
        if(!SetTimer(window,BRIDGE_TIMER,100,NULL))ShowWindow(window,SW_SHOW);
        return 0;
    }
    if(message==WM_TIMER && wparam==BRIDGE_INIT_TIMER){
        DWORD now=GetTickCount();
        if(ctx->selected[0] && now-ctx->changed_tick>=200){
            KillTimer(window,BRIDGE_INIT_TIMER);
            if(!PostMessageW(window,ctx->run_message,0,0))ShowWindow(window,SW_SHOW);
        }else if(now-ctx->init_tick>=2000){
            KillTimer(window,BRIDGE_INIT_TIMER);
            ShowWindow(window,SW_SHOW);SetForegroundWindow(window);
        }
        return 0;
    }
    if(message==WM_TIMER && wparam==BRIDGE_TIMER){
        HWND accept=GetDlgItem(window,IDOK);
        if(!lstrcmpiW(ctx->selected,ctx->desired) && accept && IsWindowEnabled(accept)){
            KillTimer(window,BRIDGE_TIMER);
            PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)accept);
        }else if(++ctx->attempts>=80){
            KillTimer(window,BRIDGE_TIMER);
            ShowWindow(window,SW_SHOW);SetForegroundWindow(window);
        }
        return 0;
    }
    if(message==WM_NCDESTROY){KillTimer(window,BRIDGE_INIT_TIMER);KillTimer(window,BRIDGE_TIMER);RemoveWindowSubclass(window,shell_subclass,identity);}
    return DefSubclassProc(window,message,wparam,lparam);
}
static LRESULT CALLBACK capture_selection(int code,WPARAM wparam,LPARAM data)
{
    folder_context *ctx=active;
    if(ctx && code>=0 && data && ((const CWPSTRUCT *)data)->hwnd==ctx->callback_window){
        const CWPSTRUCT *sent=(const CWPSTRUCT *)data;
        UINT message=sent->message;WPARAM wparam=sent->wParam;LPARAM lparam=sent->lParam;
        if(message==BFFM_SETSELECTIONW || message==BFFM_SETSELECTIONA){
        WCHAR requested[MAX_PATH]=L"";
        if(!wparam && lparam)SHGetPathFromIDListW((PCIDLIST_ABSOLUTE)lparam,requested);
        else if(wparam && lparam && message==BFFM_SETSELECTIONW)lstrcpynW(requested,(LPCWSTR)lparam,MAX_PATH);
        else if(wparam && lparam)MultiByteToWideChar(CP_ACP,0,(LPCSTR)lparam,-1,requested,MAX_PATH);
        DWORD attrs=GetFileAttributesW(requested);
        if(attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY))
            lstrcpynW(ctx->requested,requested,MAX_PATH);
        }
    }
    return CallNextHookEx(NULL,code,wparam,data);
}
static int CALLBACK browse_callback(HWND window,UINT message,LPARAM parameter,LPARAM data)
{
    folder_context *ctx=(folder_context *)data;
    if(message==BFFM_SELCHANGED){ctx->changed_tick=GetTickCount();ctx->selected[0]=0;if(parameter)SHGetPathFromIDListW((PCIDLIST_ABSOLUTE)parameter,ctx->selected);}
    HHOOK capture=NULL;
    if(message==BFFM_INITIALIZED){
        ctx->callback_window=window;
        capture=SetWindowsHookExW(WH_CALLWNDPROC,capture_selection,NULL,GetCurrentThreadId());
    }
    int answer=ctx->callback?ctx->callback(window,message,parameter,ctx->original_data):0;
    if(capture)UnhookWindowsHookEx(capture);
    if(message==BFFM_INITIALIZED)ctx->callback_window=NULL;
    if(message==BFFM_INITIALIZED && capture && IsWindow(window) &&
        SetWindowSubclass(window,shell_subclass,(UINT_PTR)ctx,(DWORD_PTR)ctx)){
        ctx->init_tick=ctx->changed_tick=GetTickCount();
        ShowWindow(window,SW_HIDE);
        if(!SetTimer(window,BRIDGE_INIT_TIMER,100,NULL))ShowWindow(window,SW_SHOW);
    }
    return answer;
}
#define BROWSE_WRAPPER(name,type,native) \
static PIDLIST_ABSOLUTE WINAPI name(type *info) { \
    if(!info || active || info->pidlRoot || info->ulFlags!=BRIDGE_FLAGS) return native(info); \
    folder_context ctx={0};ctx.callback=info->lpfn;ctx.original_data=info->lParam; \
    ctx.run_message=RegisterWindowMessageW(L"Win16Style.T20.FolderBridge"); \
    if(!ctx.run_message)return native(info); \
    type copy=*info;copy.lpfn=browse_callback;copy.lParam=(LPARAM)&ctx;active=&ctx; \
    PIDLIST_ABSOLUTE result=native(&copy);active=NULL;return result; \
}
BROWSE_WRAPPER(browse_a,BROWSEINFOA,native_browse_a)
BROWSE_WRAPPER(browse_w,BROWSEINFOW,native_browse_w)
LONG ntw16s_attach_folder_dialogs(void)
{
    if(!(ntw16s_process_context.flags & NTW16S_POLICY_STYLE))return NO_ERROR;
    native_browse_a=SHBrowseForFolderA;native_browse_w=SHBrowseForFolderW;
    LONG error=(LONG)ntw16s_hooks_attach(&native_browse_a,(void *)browse_a);
    if(!error)error=(LONG)ntw16s_hooks_attach(&native_browse_w,(void *)browse_w);
    return error;
}
