#include "print_extended.h"
#include "lib-style/common/print_ranges.h"
#include "lib-style/common/print_resources.h"
#include <wchar.h>
#define RANGE_CONTROL 9002
#define APPLY_CONTROL 9001
#define CURRENT_CONTROL 9003
#define MAX_RANGES 32
#define CONTEXT_PROPERTY L"style.classic.print.extended.v1"
typedef struct state {
    HGLOBAL *live_mode,*live_names;HWND edit,apply,current;DWORD input_flags,minimum,maximum,capacity;BOOL ansi,initializing,current_selected;WCHAR initial[512];
    PRINTPAGERANGE ranges[MAX_RANGES],applied_ranges[MAX_RANGES];DWORD count,applied_count,copies,flags;
    HGLOBAL applied_mode,applied_names;HDC applied_dc;BOOL applied,failed;
} state;
static void free_snapshot(state *s)
{if(s->applied_dc)DeleteDC(s->applied_dc);if(s->applied_mode)GlobalFree(s->applied_mode);if(s->applied_names)GlobalFree(s->applied_names);s->applied_dc=NULL;s->applied_mode=NULL;s->applied_names=NULL;s->applied=FALSE;}
static void free_print_result(state *s,HDC dc,HGLOBAL mode,HGLOBAL names)
{
    if(dc)DeleteDC(dc);
    if(mode)GlobalFree(mode);
    if(names)GlobalFree(names);
    free_snapshot(s);
}
static BOOL read_ranges(HWND window,state *s)
{
    s->count=0;
    if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)!=BST_CHECKED)return TRUE;
    WCHAR text[512];if(!GetWindowTextW(s->edit,text,512))return FALSE;
    return style_print_ranges_parse(text,s->minimum,s->maximum,s->ranges,s->capacity,&s->count);
}
static DWORD input_flags(HWND window,const state *s)
{
    DWORD flags=s->input_flags & ~(PD_PAGENUMS|PD_SELECTION|PD_CURRENTPAGE|PD_COLLATE|PD_PRINTTOFILE);
    if(s->current && SendMessageW(s->current,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_CURRENTPAGE;
    else if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PAGENUMS;
    else if(SendDlgItemMessageW(window,1057,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_SELECTION;
    if(SendDlgItemMessageW(window,1041,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_COLLATE;
    if(SendDlgItemMessageW(window,1040,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PRINTTOFILE;
    return flags;
}
static BOOL save_snapshot(HWND window,state *s)
{
    BOOL translated=FALSE;DWORD copies=GetDlgItemInt(window,1154,&translated,FALSE);if(!translated || !copies || copies>32767 || !read_ranges(window,s))return FALSE;
    DWORD flags=input_flags(window,s);
    HGLOBAL mode=style_print_clone_global(*s->live_mode),names=(flags&PD_PRINTTOFILE)?style_print_clone_file_devnames(*s->live_names,s->ansi):style_print_clone_global(*s->live_names),dc_mode=style_print_clone_global(*s->live_mode);
    if(!mode || !names || !dc_mode){if(mode)GlobalFree(mode);if(names)GlobalFree(names);if(dc_mode)GlobalFree(dc_mode);return FALSE;}
    void *mode_data=GlobalLock(mode),*dc_data=GlobalLock(dc_mode);DEVNAMES *dn=GlobalLock(names);HDC dc=NULL;
    BOOL ready=mode_data && dc_data && dn;
    if(ready){
        BOOL driver_copies=(flags&PD_USEDEVMODECOPIESANDCOLLATE)!=0;
        style_print_set_copies(mode_data,s->ansi,driver_copies?(short)copies:1,driver_copies && (flags&PD_COLLATE));
        style_print_set_copies(dc_data,s->ansi,(short)copies,(flags&PD_COLLATE)!=0);
        dc=style_print_create_dc(dc_data,dn,s->ansi,flags);
    }
    if(mode_data)GlobalUnlock(mode);
    if(dc_data)GlobalUnlock(dc_mode);
    if(dn)GlobalUnlock(names);
    GlobalFree(dc_mode);
    if(!ready || ((flags & (PD_RETURNDC|PD_RETURNIC)) && !dc)){GlobalFree(mode);GlobalFree(names);return FALSE;}
    free_snapshot(s);s->applied=TRUE;s->applied_mode=mode;s->applied_names=names;s->applied_dc=dc;s->copies=(flags&PD_USEDEVMODECOPIESANDCOLLATE)?1:copies;s->flags=(flags&PD_USEDEVMODECOPIESANDCOLLATE)?flags&~(DWORD)PD_COLLATE:flags;
    s->applied_count=s->count;CopyMemory(s->applied_ranges,s->ranges,s->count*sizeof(PRINTPAGERANGE));return TRUE;
}
static UINT_PTR print_hook_common(HWND window,UINT message,WPARAM wparam,state *initial)
{
    state *s=message==WM_INITDIALOG?initial:(state *)GetPropW(window,CONTEXT_PROPERTY);
    if(message==WM_INITDIALOG){s->initializing=TRUE;if(!SetPropW(window,CONTEXT_PROPERTY,s)){goto failed;}
        RECT first,last,ok;HWND a=GetDlgItem(window,1152),b=GetDlgItem(window,1153),button=GetDlgItem(window,IDOK);
        if(!a || !b || !button){goto failed;}
        GetWindowRect(a,&first);GetWindowRect(b,&last);GetWindowRect(button,&ok);
        MapWindowPoints(NULL,window,(POINT*)&first,2);MapWindowPoints(NULL,window,(POINT*)&last,2);MapWindowPoints(NULL,window,(POINT*)&ok,2);
        ShowWindow(GetDlgItem(window,1089),SW_HIDE);ShowWindow(GetDlgItem(window,1090),SW_HIDE);ShowWindow(a,SW_HIDE);ShowWindow(b,SW_HIDE);
        HFONT font=(HFONT)SendMessageW(button,WM_GETFONT,0,0);
        HDC dc=GetDC(window);int dpi=dc?GetDeviceCaps(dc,LOGPIXELSX):96;if(dc)ReleaseDC(window,dc);
        int inset=MulDiv(25,dpi,96),gap=MulDiv(8,dpi,96);
        s->edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",s->initial,WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,first.left-inset,first.top,last.right-first.left+inset,first.bottom-first.top,window,(HMENU)(INT_PTR)RANGE_CONTROL,GetModuleHandleW(NULL),NULL);
        s->apply=CreateWindowW(L"BUTTON",L"Apply",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_DISABLED|BS_PUSHBUTTON,ok.left-(ok.right-ok.left)-gap,ok.top,ok.right-ok.left,ok.bottom-ok.top,window,(HMENU)(INT_PTR)APPLY_CONTROL,GetModuleHandleW(NULL),NULL);
        if(!s->edit || !s->apply){goto failed;}
        if(!(s->input_flags & PD_NOCURRENTPAGE)){
            HWND group=GetDlgItem(window,1072),all=GetDlgItem(window,1056),pages=GetDlgItem(window,1058),selection=GetDlgItem(window,1057);
            RECT gr,ar,pr,sr;
            if(!group || !all || !pages || !selection || !GetWindowRect(group,&gr) || !GetWindowRect(all,&ar) || !GetWindowRect(pages,&pr) || !GetWindowRect(selection,&sr)){
                goto failed;
            }
            MapWindowPoints(NULL,window,(POINT*)&gr,2);MapWindowPoints(NULL,window,(POINT*)&ar,2);
            MapWindowPoints(NULL,window,(POINT*)&pr,2);MapWindowPoints(NULL,window,(POINT*)&sr,2);
            int height=ar.bottom-ar.top,start=gr.top+MulDiv(8,dpi,96),end=gr.bottom-MulDiv(5,dpi,96)-height;
            int step=(end-start)/3;
            if(height<=0 || step<height){goto failed;}
            RECT edit;if(!GetWindowRect(s->edit,&edit)){goto failed;}
            MapWindowPoints(NULL,window,(POINT*)&edit,2);
            if(!SetWindowPos(all,NULL,ar.left,start,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE) ||
               !SetWindowPos(pages,NULL,pr.left,start+step,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE) ||
               !SetWindowPos(selection,NULL,sr.left,start+step*2,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE) ||
               !SetWindowPos(s->edit,NULL,edit.left,start+step,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE)){
                goto failed;
            }
            s->current=CreateWindowW(L"BUTTON",L"C&urrent page",WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
                ar.left,start+step*3,gr.right-ar.left-MulDiv(8,dpi,96),height,window,(HMENU)(INT_PTR)CURRENT_CONTROL,GetModuleHandleW(NULL),NULL);
            if(!s->current || !SetWindowPos(s->current,selection,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE)){
                goto failed;
            }
            SendMessageW(s->current,WM_SETFONT,(WPARAM)font,TRUE);
            if(s->input_flags & PD_CURRENTPAGE){SendDlgItemMessageW(window,1056,BM_SETCHECK,BST_UNCHECKED,0);SendMessageW(s->current,BM_SETCHECK,BST_CHECKED,0);}
        }
        if((s->input_flags & (PD_COLLATE|PD_USEDEVMODECOPIESANDCOLLATE))==(PD_COLLATE|PD_USEDEVMODECOPIESANDCOLLATE))
            SendDlgItemMessageW(window,1041,BM_SETCHECK,BST_CHECKED,0);
        if(s->input_flags & PD_NOPAGENUMS)EnableWindow(s->edit,FALSE);
        if(s->edit)SendMessageW(s->edit,WM_SETFONT,(WPARAM)font,TRUE);
        if(s->apply)SendMessageW(s->apply,WM_SETFONT,(WPARAM)font,TRUE);
        s->initializing=FALSE;
    }
    if(s && message==WM_COMMAND){UINT id=LOWORD(wparam),notice=HIWORD(wparam);
        if(id==CURRENT_CONTROL && notice==BN_CLICKED){for(int radio=1056;radio<=1058;++radio)SendDlgItemMessageW(window,radio,BM_SETCHECK,BST_UNCHECKED,0);SendMessageW(s->current,BM_SETCHECK,BST_CHECKED,0);if(s->apply)EnableWindow(s->apply,TRUE);return 1;}
        if(s->current && notice==BN_CLICKED && (id==1056 || id==1057 || id==1058))SendMessageW(s->current,BM_SETCHECK,BST_UNCHECKED,0);
        if(id==IDOK)s->current_selected=s->current && SendMessageW(s->current,BM_GETCHECK,0,0)==BST_CHECKED;
        if(id==RANGE_CONTROL && notice==EN_SETFOCUS)SendDlgItemMessageW(window,1058,BM_CLICK,0,0);
        if(id==APPLY_CONTROL){if(save_snapshot(window,s))EnableWindow(s->apply,FALSE);else MessageBeep(MB_ICONEXCLAMATION);return 1;}
        if(!s->initializing && s->apply && ((notice==EN_CHANGE && (id==RANGE_CONTROL || id==1154)) ||
            (notice==CBN_SELCHANGE && id==1139) || (notice==BN_CLICKED && (id==1056 || id==1057 || id==1058 || id==1040 || id==1041 || id==1025))))EnableWindow(s->apply,TRUE);
        if(id==IDOK && !read_ranges(window,s)){MessageBeep(MB_ICONEXCLAMATION);return 1;}}
    if(message==WM_DESTROY)RemovePropW(window,CONTEXT_PROPERTY);
    return 0;
failed:
    s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;
}
static UINT_PTR CALLBACK print_hook_a(HWND w,UINT m,WPARAM wp,LPARAM lp)
{state *s=NULL;if(m==WM_INITDIALOG){PRINTDLGA *p=(PRINTDLGA*)lp;s=(state*)p->lCustData;s->live_mode=&p->hDevMode;s->live_names=&p->hDevNames;}return print_hook_common(w,m,wp,s);}
static UINT_PTR CALLBACK print_hook_w(HWND w,UINT m,WPARAM wp,LPARAM lp)
{state *s=NULL;if(m==WM_INITDIALOG){PRINTDLGW *p=(PRINTDLGW*)lp;s=(state*)p->lCustData;s->live_mode=&p->hDevMode;s->live_names=&p->hDevNames;}return print_hook_common(w,m,wp,s);}
#define PRINT_EXTENDED(NAME,EX_TYPE,TYPE,PRINT_TYPE,HOOK,IS_ANSI) \
BOOL NAME(EX_TYPE *p,PRINT_TYPE print,HRESULT *result) \
{ \
    if(!p || p->lStructSize!=sizeof(*p) || !p->hwndOwner || !IsWindow(p->hwndOwner) || (p->Flags & ~(PD_NOPAGENUMS|PD_NOSELECTION|PD_NOWARNING|PD_NOCURRENTPAGE|PD_CURRENTPAGE|PD_HIDEPRINTTOFILE|PD_DISABLEPRINTTOFILE|PD_PRINTTOFILE|PD_COLLATE|PD_PAGENUMS|PD_SELECTION|PD_RETURNDC|PD_RETURNIC|PD_USEDEVMODECOPIESANDCOLLATE))!=0 || ((p->Flags&PD_SELECTION) && (p->Flags&(PD_NOSELECTION|PD_PAGENUMS|PD_CURRENTPAGE))) || ((p->Flags&PD_CURRENTPAGE) && (p->Flags&(PD_NOCURRENTPAGE|PD_PAGENUMS))) || ((p->Flags&PD_PRINTTOFILE) && (p->Flags&(PD_HIDEPRINTTOFILE|PD_DISABLEPRINTTOFILE))) || p->Flags2 || p->ExclusionFlags || p->hDC || (!!p->hDevMode != !!p->hDevNames) || (p->hDevMode && p->hDevMode==p->hDevNames) || !style_print_global_available(p->hDevMode) || !style_print_global_available(p->hDevNames) || ((p->Flags & PD_PAGENUMS) ? (!p->nPageRanges || p->nPageRanges>p->nMaxPageRanges || (p->Flags & PD_NOPAGENUMS)) : p->nPageRanges!=0) || (!(p->Flags & PD_NOPAGENUMS) && (!p->lpPageRanges || !p->nMaxPageRanges || p->nMaxPageRanges>MAX_RANGES || !p->nMinPage || p->nMaxPage>65535 || p->nMinPage>p->nMaxPage)) || !p->nCopies || p->nCopies>32767 || p->lpCallback || p->nPropertyPages || p->lphPropertyPages || p->nStartPage!=START_PAGE_GENERAL)return FALSE; \
    state s={0};s.input_flags=p->Flags;s.minimum=p->nMinPage;s.maximum=p->nMaxPage;s.capacity=p->nMaxPageRanges;s.ansi=IS_ANSI; \
    if(p->nPageRanges){if(!style_print_ranges_format(p->lpPageRanges,p->nPageRanges,p->nMinPage,p->nMaxPage,s.initial,512))return FALSE;}else wsprintfW(s.initial,L"%lu",p->nMinPage);TYPE d={0};d.lStructSize=sizeof(d);d.hwndOwner=p->hwndOwner;d.Flags=(p->Flags & ~(DWORD)(PD_CURRENTPAGE|PD_NOCURRENTPAGE) & ((p->Flags & PD_RETURNDC)?~(DWORD)PD_RETURNIC:~(DWORD)0))|PD_ENABLEPRINTHOOK;d.lpfnPrintHook=HOOK;d.lCustData=(LPARAM)&s;d.nMinPage=(WORD)(p->Flags & PD_NOPAGENUMS?1:p->nMinPage);d.nMaxPage=(WORD)(p->Flags & PD_NOPAGENUMS?1:p->nMaxPage);d.nFromPage=p->nPageRanges?(WORD)p->lpPageRanges[0].nFromPage:d.nMinPage;d.nToPage=p->nPageRanges?(WORD)p->lpPageRanges[0].nToPage:d.nMinPage;d.nCopies=(WORD)p->nCopies; \
    if(p->hDevMode){d.hDevMode=style_print_clone_global(p->hDevMode);if(!d.hDevMode)return FALSE;} \
    if(p->hDevNames){d.hDevNames=style_print_clone_global(p->hDevNames);if(!d.hDevNames){if(d.hDevMode)GlobalFree(d.hDevMode);return FALSE;}} \
    BOOL ok=print(&d);DWORD error=ok?0:CommDlgExtendedError(); \
    if(ok && (p->Flags & (PD_RETURNDC|PD_RETURNIC)) && (!d.hDC || ((p->Flags & PD_RETURNDC) && d.nCopies>1))){HDC replacement=style_print_create_print_dc(d.hDevMode,d.hDevNames,IS_ANSI,d.nCopies,d.Flags|p->Flags); \
        if(!replacement){free_print_result(&s,d.hDC,d.hDevMode,d.hDevNames);*result=E_FAIL;return TRUE;} \
        if(d.hDC){DeleteDC(d.hDC);}d.hDC=replacement;} \
    if(ok && (p->Flags & PD_USEDEVMODECOPIESANDCOLLATE) && !style_print_normalize_driver_copies(d.hDevMode,IS_ANSI,&d.nCopies,&d.Flags)){free_print_result(&s,d.hDC,d.hDevMode,d.hDevNames);*result=E_FAIL;return TRUE;} \
    if(ok && (!style_print_global_available(p->hDevMode) || !style_print_global_available(p->hDevNames))){free_print_result(&s,d.hDC,d.hDevMode,d.hDevNames);*result=E_FAIL;return TRUE;} \
    if(ok){if(p->hDevMode)GlobalFree(p->hDevMode);if(p->hDevNames)GlobalFree(p->hDevNames);p->hDevMode=d.hDevMode;p->hDevNames=d.hDevNames;p->hDC=d.hDC;p->Flags=(d.Flags & ~(PD_ENABLEPRINTHOOK|PD_RETURNDC|PD_RETURNIC))|(p->Flags & (PD_RETURNDC|PD_RETURNIC|PD_NOCURRENTPAGE));if(s.current_selected)p->Flags=(p->Flags & ~(DWORD)(PD_SELECTION|PD_PAGENUMS))|PD_CURRENTPAGE;p->nCopies=d.nCopies;p->nPageRanges=s.count;if(s.count)CopyMemory(p->lpPageRanges,s.ranges,s.count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_PRINT;free_snapshot(&s);*result=S_OK;return TRUE;} \
    if(d.hDC)DeleteDC(d.hDC); \
    if(d.hDevMode)GlobalFree(d.hDevMode); \
    if(d.hDevNames)GlobalFree(d.hDevNames); \
    if(s.failed){free_snapshot(&s);return FALSE;} \
    if(error){free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(s.applied && (!style_print_global_available(p->hDevMode) || !style_print_global_available(p->hDevNames))){free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(s.applied){if(p->hDevMode)GlobalFree(p->hDevMode);if(p->hDevNames)GlobalFree(p->hDevNames);p->hDevMode=s.applied_mode;p->hDevNames=s.applied_names;p->hDC=s.applied_dc;p->Flags=s.flags;p->nCopies=s.copies;p->nPageRanges=s.applied_count;if(s.applied_count)CopyMemory(p->lpPageRanges,s.applied_ranges,s.applied_count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_APPLY;} \
    else p->dwResultAction=PD_RESULT_CANCEL; \
    *result=S_OK;return TRUE; \
}
PRINT_EXTENDED(style_print_extended_a,PRINTDLGEXA,PRINTDLGA,style_print_dialog_a,print_hook_a,TRUE)
PRINT_EXTENDED(style_print_extended_w,PRINTDLGEXW,PRINTDLGW,style_print_dialog_w,print_hook_w,FALSE)
#undef PRINT_EXTENDED
