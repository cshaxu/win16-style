/* Select the older Shell tree for a limited, callback-free browse request.
 * Shell owns the dialog and returned PIDL. Other contracts remain native. */
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
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
    WCHAR selected[MAX_PATH], desired[MAX_PATH], requested[MAX_PATH], classic_current[MAX_PATH];
    UINT run_message, attempts;
    DWORD init_tick, changed_tick;
    BOOL showing;
    HWND callback_window;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    ntw16s_folder_observer observer;void *observer_context;
    HWND classic_window;
#endif
} folder_context;
static _Thread_local folder_context *active;
static PIDLIST_ABSOLUTE (WINAPI *native_browse_a)(LPBROWSEINFOA);
static PIDLIST_ABSOLUTE (WINAPI *native_browse_w)(LPBROWSEINFOW);

/* The old template exposes its displayed path as an ancestor list. Never
 * substitute the process current directory: any thread can change it while
 * the dialog remains open. Reject unfamiliar representations instead. */
BOOL ntw16s_old_dialog_folder(HWND window,PWSTR path,size_t capacity)
{
    if(!window || !path || !capacity)return FALSE;
    path[0]=0;HWND list=GetDlgItem(window,lst2);WCHAR cls[32];
    if(!list || GetParent(list)!=window || !GetClassNameW(list,cls,ARRAYSIZE(cls)) ||
       lstrcmpiW(cls,L"ListBox") || !(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT last=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(last<0 || count<=last || last>255)return FALSE;
    size_t used=0;
    for(LRESULT i=0;i<=last;++i){
        LRESULT length=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(length<=0 || (size_t)length>=capacity || used+(size_t)length+2>capacity)return FALSE;
        WCHAR part[MAX_PATH];if((size_t)length>=ARRAYSIZE(part) ||
            SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=length)return FALSE;
        if(i==0){
            if(!(length==3 && part[1]==L':' && part[2]==L'\\') &&
               !(length>3 && part[0]==L'\\' && part[1]==L'\\'))return FALSE;
        }else if(wcspbrk(part,L"\\/:*?\"<>|") || !lstrcmpW(part,L".") || !lstrcmpW(part,L".."))return FALSE;
        if(used && path[used-1]!=L'\\')path[used++]=L'\\';
        memcpy(path+used,part,((size_t)length+1)*sizeof(WCHAR));used+=(size_t)length;
    }
    DWORD attrs=GetFileAttributesW(path);
    return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY)!=0;
}

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

typedef struct folder_create_request {WCHAR parent[MAX_PATH],path[MAX_PATH];} folder_create_request;
static HWND folder_create_control(HWND dialog,PCWSTR cls,PCWSTR text,DWORD style,int x,int y,int cx,int cy,int id)
{
    RECT r={x,y,x+cx,y+cy};MapDialogRect(dialog,&r);
    return CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,r.left,r.top,r.right-r.left,r.bottom-r.top,
        dialog,(HMENU)(UINT_PTR)id,GetModuleHandleW(NULL),NULL);
}
static INT_PTR CALLBACK folder_create_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_INITDIALOG){
        SetWindowLongPtrW(w,DWLP_USER,lp);SetWindowTextW(w,L"New Folder");
        HWND label=folder_create_control(w,L"STATIC",L"Folder name:",0,8,9,190,10,101);
        HWND edit=folder_create_control(w,L"EDIT",L"",WS_BORDER|WS_TABSTOP|ES_AUTOHSCROLL,8,21,190,14,102);
        HWND ok=folder_create_control(w,L"BUTTON",L"OK",WS_TABSTOP|BS_DEFPUSHBUTTON,91,46,50,14,IDOK);
        HWND cancel=folder_create_control(w,L"BUTTON",L"Cancel",WS_TABSTOP|BS_PUSHBUTTON,148,46,50,14,IDCANCEL);
        if(!label || !edit || !ok || !cancel){EndDialog(w,-1);return TRUE;}
        SendMessageW(edit,EM_SETLIMITTEXT,MAX_PATH-1,0);SetFocus(edit);return FALSE;
    }
    if(message==WM_COMMAND && LOWORD(wp)==IDCANCEL){EndDialog(w,0);return TRUE;}
    if(message==WM_COMMAND && LOWORD(wp)==IDOK){
        folder_create_request *request=(folder_create_request *)GetWindowLongPtrW(w,DWLP_USER);
        WCHAR name[MAX_PATH];int length=GetDlgItemTextW(w,102,name,ARRAYSIZE(name));
        if(!request || length<=0 || !lstrcmpW(name,L".") || !lstrcmpW(name,L"..") ||
            wcspbrk(name,L"\\/:*?\"<>|") || name[length-1]==L' ' || name[length-1]==L'.'){
            MessageBoxW(w,L"Enter a valid folder name.",L"New Folder",MB_OK|MB_ICONERROR);return TRUE;
        }
        int written=swprintf(request->path,ARRAYSIZE(request->path),L"%ls%ls%ls",request->parent,
            request->parent[wcslen(request->parent)-1]==L'\\'?L"":L"\\",name);
        if(written<0 || (size_t)written>=ARRAYSIZE(request->path)){
            MessageBoxW(w,L"The folder path is too long.",L"New Folder",MB_OK|MB_ICONERROR);return TRUE;
        }
        if(!CreateDirectoryW(request->path,NULL)){
            DWORD error=GetLastError();WCHAR text[512];
            if(!FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,text,ARRAYSIZE(text),NULL))
                lstrcpynW(text,L"The folder could not be created.",ARRAYSIZE(text));
            MessageBoxW(w,text,L"New Folder",MB_OK|MB_ICONERROR);return TRUE;
        }
        EndDialog(w,1);return TRUE;
    }
    return FALSE;
}
static void classic_create_folder(HWND owner,folder_context *ctx)
{
    folder_create_request request={0};
    if(!ctx || !ctx->classic_current[0]){
        MessageBoxW(owner,L"The current folder could not be determined.",L"New Folder",MB_OK|MB_ICONERROR);return;
    }
    lstrcpynW(request.parent,ctx->classic_current,ARRAYSIZE(request.parent));
    struct {DLGTEMPLATE dialog;WORD menu,window_class,title;} template={0};
    template.dialog.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME;
    template.dialog.cx=206;template.dialog.cy=69;
    if(DialogBoxIndirectParamW(GetModuleHandleW(NULL),&template.dialog,owner,folder_create_proc,(LPARAM)&request)==1 && IsWindow(owner)){
        HWND edit=GetDlgItem(owner,edt1);WCHAR previous[MAX_PATH]=L"";
        if(edit){GetWindowTextW(edit,previous,ARRAYSIZE(previous));SetWindowTextW(edit,request.path);
            SendMessageW(owner,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(owner,IDOK));
            if(IsWindow(owner) && IsWindow(edit))SetWindowTextW(edit,
                !lstrcmpiW(ctx->classic_current,request.path)?L"[Select current folder]":previous);}
    }
}

static UINT_PTR CALLBACK old_folder_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    if(message==WM_INITDIALOG){
        ntw16s_old_dialog_chrome(window);
        OPENFILENAMEW *ofn=(OPENFILENAMEW *)lparam;
        folder_context *ctx=(folder_context *)ofn->lCustData;
        SetPropW(window,L"Win16s.FolderBridge",ctx);
        if(ctx)(void)ntw16s_old_dialog_folder(window,ctx->classic_current,ARRAYSIZE(ctx->classic_current));
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        if(ctx)ctx->classic_window=window;
#endif
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
        folder_context *ctx=GetPropW(window,L"Win16s.FolderBridge");
        if(ctx)classic_create_folder(window,ctx);
        return 1;
    }
    if(message>=0xc000 && message<=0xffff){
        UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);
        if(selection && message==selection && (wparam==lst2 || wparam==cmb2)){
            folder_context *ctx=GetPropW(window,L"Win16s.FolderBridge");
            if(ctx){WCHAR folder[MAX_PATH];
                if(ntw16s_old_dialog_folder(window,folder,ARRAYSIZE(folder)))lstrcpynW(ctx->classic_current,folder,ARRAYSIZE(ctx->classic_current));
                else ctx->classic_current[0]=0;}
        }
    }
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    folder_context *observer_ctx=message==WM_INITDIALOG && lparam?
        (folder_context *)((OPENFILENAMEW *)lparam)->lCustData:GetPropW(window,L"Win16s.FolderBridge");
    if(observer_ctx && observer_ctx->observer){UINT_PTR handled=observer_ctx->observer(window,message,wparam,lparam,observer_ctx->observer_context);
        if(handled)return handled;}
#endif
    if(message==WM_NCDESTROY){
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        if(observer_ctx && observer_ctx->classic_window==window)observer_ctx->classic_window=NULL;
#endif
        RemovePropW(window,L"Win16s.FolderBridge");
    }
    return 0;
}
/* COM folder requests need a Shell item, not a fabricated PIDL. Reuse the
 * classic system template and return its selected directory to the caller. */
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static void classic_folder_active_cleanup(void *value)
{
    folder_context *ctx=value;HWND window=ctx->classic_window;
    if(window && IsWindow(window))DestroyWindow(window);
    ctx->classic_window=NULL;active=NULL;
}
NTW16S_UNWIND_FUNCTION HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity,ntw16s_folder_observer observer,void *context)
#else
HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity)
#endif
{
    if(!result || !capacity)return E_INVALIDARG;
    result[0]=0;if(active)return HRESULT_FROM_WIN32(ERROR_BUSY);
    folder_context ctx={0};ctx.ok_label=ok_label;ctx.cancel_label=cancel_label;ctx.file_label=file_label;ctx.run_message=RegisterWindowMessageW(L"Win16Style.T20.FolderBridge");
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    ctx.observer=observer;ctx.observer_context=context;
#endif
    if(!ctx.run_message)return HRESULT_FROM_WIN32(GetLastError());
    WCHAR file[MAX_PATH]=L"[Select current folder]";
    if(initial && *initial && swprintf(file,MAX_PATH,L"%ls%ls[Select current folder]",initial,
        initial[wcslen(initial)-1]==L'\\'?L"":L"\\")<0)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=owner;
    ofn.lpstrTitle=title?title:L"Select Folder";ofn.lpstrFile=file;ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"All files\0*.*\0\0";ofn.lpstrInitialDir=initial;
    ofn.Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_LONGNAMES;
    ofn.lpfnHook=old_folder_hook;ofn.lCustData=(LPARAM)&ctx;
    active=&ctx;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    ntw16s_unwind_scope unwind;BOOL chosen;DWORD error;
    NTW16S_UNWIND_BEGIN(unwind,classic_folder_active_cleanup,&ctx);
    chosen=GetOpenFileNameW(&ofn);error=chosen?0:CommDlgExtendedError();
    NTW16S_UNWIND_END(unwind);
#else
    BOOL chosen=GetOpenFileNameW(&ofn);DWORD error=chosen?0:CommDlgExtendedError();active=NULL;
#endif
    if(!chosen)return error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);
    WCHAR *slash=wcsrchr(file,L'\\');if(!slash)return E_FAIL;
    if(slash==file+2)slash[1]=0;else *slash=0;
    if(wcslen(file)>=capacity)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    wcscpy(result,file);return S_OK;
}
#define BROWSE_WRAPPER(name,type,native) \
static PIDLIST_ABSOLUTE WINAPI name(type *info) { \
    if(!info || active || info->lpfn || info->ulFlags!=BRIDGE_FLAGS) return native(info); \
    type copy=*info;copy.ulFlags=BIF_RETURNONLYFSDIRS;return native(&copy); \
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
