/* Existing old-style New Folder dialog. */
#include "folder_create.h"
#include <wchar.h>
#include <stdio.h>

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
BOOL win16_dialog_create_folder(HWND owner,PCWSTR parent,PWSTR path,size_t capacity)
{
    folder_create_request request={0};
    if(!parent || !parent[0]){
        MessageBoxW(owner,L"The current folder could not be determined.",L"New Folder",MB_OK|MB_ICONERROR);return FALSE;
    }
    lstrcpynW(request.parent,parent,ARRAYSIZE(request.parent));
    struct {DLGTEMPLATE dialog;WORD menu,window_class,title;} template={0};
    template.dialog.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME;
    template.dialog.cx=206;template.dialog.cy=69;
    if(DialogBoxIndirectParamW(GetModuleHandleW(NULL),&template.dialog,owner,folder_create_proc,(LPARAM)&request)!=1)return FALSE;
    lstrcpynW(path,request.path,(int)capacity);return TRUE;
}
