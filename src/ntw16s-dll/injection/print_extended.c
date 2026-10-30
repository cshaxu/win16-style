#include "print_extended.h"
#include "ntw16s-dll/injection/print_ranges.h"
#include <wchar.h>
#define RANGE_CONTROL 9002
#define APPLY_CONTROL 9001
#define MAX_RANGES 32
#define CONTEXT_PROPERTY L"ntw16s.print.extended"
typedef struct state {
    PRINTDLGEXW *out;PRINTDLGW *live;HWND edit;
    PRINTPAGERANGE ranges[MAX_RANGES],applied_ranges[MAX_RANGES];DWORD count,applied_count,copies,flags;
    HGLOBAL applied_mode,applied_names;HDC applied_dc;BOOL applied,failed;
} state;
static void free_snapshot(state *s)
{if(s->applied_dc)DeleteDC(s->applied_dc);if(s->applied_mode)GlobalFree(s->applied_mode);if(s->applied_names)GlobalFree(s->applied_names);s->applied_dc=NULL;s->applied_mode=NULL;s->applied_names=NULL;s->applied=FALSE;}
static HGLOBAL clone_global(HGLOBAL original)
{
    SIZE_T size=original?GlobalSize(original):0;void *src=size?GlobalLock(original):NULL;
    HGLOBAL copy=src?GlobalAlloc(GMEM_MOVEABLE,size):NULL;void *dst=copy?GlobalLock(copy):NULL;
    if(!dst){if(src)GlobalUnlock(original);if(copy)GlobalFree(copy);return NULL;}
    CopyMemory(dst,src,size);GlobalUnlock(copy);GlobalUnlock(original);return copy;
}
static BOOL read_ranges(HWND window,state *s)
{
    s->count=0;
    if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)!=BST_CHECKED)return TRUE;
    WCHAR text[512];if(!GetWindowTextW(s->edit,text,512))return FALSE;
    return ntw16s_parse_print_ranges(text,s->out->nMinPage,s->out->nMaxPage,s->ranges,s->out->nMaxPageRanges,&s->count);
}
static DWORD input_flags(HWND window)
{
    DWORD flags=PD_RETURNDC;
    if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PAGENUMS;
    else if(SendDlgItemMessageW(window,1057,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_SELECTION;
    if(SendDlgItemMessageW(window,1041,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_COLLATE;
    if(SendDlgItemMessageW(window,1040,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PRINTTOFILE;
    return flags;
}
static BOOL save_snapshot(HWND window,state *s)
{
    BOOL translated=FALSE;DWORD copies=GetDlgItemInt(window,1154,&translated,FALSE);if(!translated || !copies || copies>32767 || !read_ranges(window,s))return FALSE;
    HGLOBAL mode=clone_global(s->live->hDevMode),names=clone_global(s->live->hDevNames),dc_mode=clone_global(s->live->hDevMode);
    if(!mode || !names || !dc_mode){if(mode)GlobalFree(mode);if(names)GlobalFree(names);if(dc_mode)GlobalFree(dc_mode);return FALSE;}
    DEVMODEW *dm=GlobalLock(mode),*dc_dm=GlobalLock(dc_mode);DEVNAMES *dn=GlobalLock(names);HDC dc=NULL;
    DWORD flags=input_flags(window);
    if(dm && dc_dm && dn){dm->dmCopies=1;dm->dmCollate=DMCOLLATE_FALSE;dc_dm->dmCopies=(short)copies;dc_dm->dmCollate=(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
        dc=CreateDCW((WCHAR*)dn+dn->wDriverOffset,(WCHAR*)dn+dn->wDeviceOffset,(WCHAR*)dn+dn->wOutputOffset,dc_dm);}
    if(dm)GlobalUnlock(mode);
    if(dc_dm)GlobalUnlock(dc_mode);
    if(dn)GlobalUnlock(names);
    GlobalFree(dc_mode);
    if(!dc){GlobalFree(mode);GlobalFree(names);return FALSE;}
    free_snapshot(s);s->applied=TRUE;s->applied_mode=mode;s->applied_names=names;s->applied_dc=dc;s->copies=copies;s->flags=flags;
    s->applied_count=s->count;CopyMemory(s->applied_ranges,s->ranges,s->count*sizeof(PRINTPAGERANGE));return TRUE;
}
static UINT_PTR CALLBACK print_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    state *s=(state *)GetPropW(window,CONTEXT_PROPERTY);
    if(message==WM_INITDIALOG){PRINTDLGW *p=(PRINTDLGW*)lparam;s=(state*)p->lCustData;s->live=p;if(!SetPropW(window,CONTEXT_PROPERTY,s)){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        RECT first,last,ok;HWND a=GetDlgItem(window,1152),b=GetDlgItem(window,1153),button=GetDlgItem(window,IDOK);
        if(!a || !b || !button){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        GetWindowRect(a,&first);GetWindowRect(b,&last);GetWindowRect(button,&ok);
        MapWindowPoints(NULL,window,(POINT*)&first,2);MapWindowPoints(NULL,window,(POINT*)&last,2);MapWindowPoints(NULL,window,(POINT*)&ok,2);
        ShowWindow(GetDlgItem(window,1089),SW_HIDE);ShowWindow(GetDlgItem(window,1090),SW_HIDE);ShowWindow(a,SW_HIDE);ShowWindow(b,SW_HIDE);
        HFONT font=(HFONT)SendMessageW(button,WM_GETFONT,0,0);WCHAR initial[16];wsprintfW(initial,L"%lu",s->out->nMinPage);
        HDC dc=GetDC(window);int dpi=dc?GetDeviceCaps(dc,LOGPIXELSX):96;if(dc)ReleaseDC(window,dc);
        int inset=MulDiv(25,dpi,96),gap=MulDiv(8,dpi,96);
        s->edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",initial,WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,first.left-inset,first.top,last.right-first.left+inset,first.bottom-first.top,window,(HMENU)(INT_PTR)RANGE_CONTROL,GetModuleHandleW(NULL),NULL);
        HWND apply=CreateWindowW(L"BUTTON",L"Apply",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,ok.left-(ok.right-ok.left)-gap,ok.top,ok.right-ok.left,ok.bottom-ok.top,window,(HMENU)(INT_PTR)APPLY_CONTROL,GetModuleHandleW(NULL),NULL);
        if(!s->edit || !apply){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        if(s->edit)SendMessageW(s->edit,WM_SETFONT,(WPARAM)font,TRUE);
        if(apply)SendMessageW(apply,WM_SETFONT,(WPARAM)font,TRUE);
    }
    if(s && message==WM_COMMAND){if(LOWORD(wparam)==RANGE_CONTROL && HIWORD(wparam)==EN_SETFOCUS)SendDlgItemMessageW(window,1058,BM_CLICK,0,0);
        if(LOWORD(wparam)==APPLY_CONTROL){if(!save_snapshot(window,s))MessageBeep(MB_ICONEXCLAMATION);return 1;}
        if(LOWORD(wparam)==IDOK && !read_ranges(window,s)){MessageBeep(MB_ICONEXCLAMATION);return 1;}}
    if(message==WM_DESTROY)RemovePropW(window,CONTEXT_PROPERTY);
    return 0;
}
BOOL ntw16s_print_extended_w(PRINTDLGEXW *p,ntw16s_print_dialog_w print,HRESULT *result)
{
    if(!p || p->lStructSize!=sizeof(*p) || !p->hwndOwner || !IsWindow(p->hwndOwner) || p->Flags!=PD_RETURNDC || p->Flags2 || p->ExclusionFlags || p->hDevMode || p->hDevNames || p->hDC || p->nPageRanges || !p->lpPageRanges || !p->nMaxPageRanges || p->nMaxPageRanges>MAX_RANGES || !p->nMinPage || p->nMaxPage>65535 || p->nMinPage>p->nMaxPage || !p->nCopies || p->nCopies>32767 || p->lpCallback || p->nPropertyPages || p->lphPropertyPages || p->nStartPage!=START_PAGE_GENERAL)return FALSE;
    state s={0};s.out=p;PRINTDLGW d={0};d.lStructSize=sizeof(d);d.hwndOwner=p->hwndOwner;d.Flags=PD_RETURNDC|PD_ENABLEPRINTHOOK;d.lpfnPrintHook=print_hook;d.lCustData=(LPARAM)&s;d.nMinPage=(WORD)p->nMinPage;d.nMaxPage=(WORD)p->nMaxPage;d.nFromPage=d.nMinPage;d.nToPage=d.nMinPage;d.nCopies=(WORD)p->nCopies;
    BOOL ok=print(&d);DWORD error=ok?0:CommDlgExtendedError();
    if(ok){p->hDevMode=d.hDevMode;p->hDevNames=d.hDevNames;p->hDC=d.hDC;p->Flags=d.Flags & ~PD_ENABLEPRINTHOOK;p->nCopies=d.nCopies;p->nPageRanges=s.count;CopyMemory(p->lpPageRanges,s.ranges,s.count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_PRINT;free_snapshot(&s);*result=S_OK;return TRUE;}
    if(d.hDC)DeleteDC(d.hDC);
    if(d.hDevMode)GlobalFree(d.hDevMode);
    if(d.hDevNames)GlobalFree(d.hDevNames);
    if(s.failed){free_snapshot(&s);return FALSE;}
    if(error){free_snapshot(&s);*result=E_FAIL;return TRUE;}
    if(s.applied){p->hDevMode=s.applied_mode;p->hDevNames=s.applied_names;p->hDC=s.applied_dc;p->Flags=s.flags;p->nCopies=s.copies;p->nPageRanges=s.applied_count;CopyMemory(p->lpPageRanges,s.applied_ranges,s.applied_count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_APPLY;}
    else p->dwResultAction=PD_RESULT_CANCEL;
    *result=S_OK;return TRUE;
}
