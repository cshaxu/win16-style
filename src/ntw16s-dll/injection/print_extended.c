#include "print_extended.h"
#include "ntw16s-dll/injection/print_ranges.h"
#include <wchar.h>
#define RANGE_CONTROL 9002
#define APPLY_CONTROL 9001
#define MAX_RANGES 32
#define CONTEXT_PROPERTY L"ntw16s.print.extended"
typedef struct state {
    HGLOBAL *live_mode,*live_names;HWND edit,apply;DWORD input_flags,minimum,maximum,capacity;BOOL ansi,initializing;WCHAR initial[512];
    PRINTPAGERANGE ranges[MAX_RANGES],applied_ranges[MAX_RANGES];DWORD count,applied_count,copies,flags;
    HGLOBAL applied_mode,applied_names;HDC applied_dc;BOOL applied,failed;
} state;
static void free_snapshot(state *s)
{if(s->applied_dc)DeleteDC(s->applied_dc);if(s->applied_mode)GlobalFree(s->applied_mode);if(s->applied_names)GlobalFree(s->applied_names);s->applied_dc=NULL;s->applied_mode=NULL;s->applied_names=NULL;s->applied=FALSE;}
static BOOL global_available(HGLOBAL memory)
{if(!memory)return TRUE;UINT flags=GlobalFlags(memory);return flags!=GMEM_INVALID_HANDLE && !(flags & GMEM_LOCKCOUNT);}
static HGLOBAL clone_global(HGLOBAL original)
{
    SIZE_T size=original?GlobalSize(original):0;void *src=size?GlobalLock(original):NULL;
    HGLOBAL copy=src?GlobalAlloc(GMEM_MOVEABLE,size):NULL;void *dst=copy?GlobalLock(copy):NULL;
    if(!dst){if(src)GlobalUnlock(original);if(copy)GlobalFree(copy);return NULL;}
    CopyMemory(dst,src,size);GlobalUnlock(copy);GlobalUnlock(original);return copy;
}
/* Apply is captured before PrintDlg commits the print-to-file selection. */
static HGLOBAL clone_file_devnames(HGLOBAL original,BOOL ansi)
{
    SIZE_T bytes=original?GlobalSize(original):0,unit=ansi?1:sizeof(WCHAR);
    SIZE_T offset=(bytes+unit-1)/unit,extra=6*unit;
    if(bytes<sizeof(DEVNAMES) || offset>65535 || offset*unit>SIZE_MAX-extra)return NULL;
    const void *src=GlobalLock(original);if(!src)return NULL;
    HGLOBAL result=GlobalAlloc(GMEM_MOVEABLE,offset*unit+extra);void *dst=result?GlobalLock(result):NULL;
    if(dst){CopyMemory(dst,src,bytes);((DEVNAMES *)dst)->wOutputOffset=(WORD)offset;
        if(ansi)CopyMemory((BYTE *)dst+offset,"FILE:",6);
        else CopyMemory((WCHAR *)dst+offset,L"FILE:",6*sizeof(WCHAR));
        GlobalUnlock(result);}
    else if(result){GlobalFree(result);result=NULL;}
    GlobalUnlock(original);return result;
}
/* PrintDlgEx embeds the requested copy count in the returned printer DC.
 * The legacy PrintDlg DC can omit it even when nCopies is returned correctly. */
static HDC create_print_dc(HGLOBAL mode,HGLOBAL names,BOOL ansi,DWORD copies,DWORD flags)
{
    if(!copies || copies>32767)return NULL;
    HGLOBAL dc_mode=clone_global(mode);if(!dc_mode)return NULL;
    void *data=GlobalLock(dc_mode);DEVNAMES *dn=names?GlobalLock(names):NULL;HDC dc=NULL;
    if(data && dn){
        if(ansi){DEVMODEA *dm=data;dm->dmCopies=(short)copies;dm->dmCollate=(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
            if(flags & PD_RETURNDC)dc=CreateDCA((char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,(char*)dn+dn->wOutputOffset,dm);
            else if(flags & PD_RETURNIC)dc=CreateICA((char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,(char*)dn+dn->wOutputOffset,dm);}
        else{DEVMODEW *dm=data;dm->dmCopies=(short)copies;dm->dmCollate=(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
            if(flags & PD_RETURNDC)dc=CreateDCW((WCHAR*)dn+dn->wDriverOffset,(WCHAR*)dn+dn->wDeviceOffset,(WCHAR*)dn+dn->wOutputOffset,dm);
            else if(flags & PD_RETURNIC)dc=CreateICW((WCHAR*)dn+dn->wDriverOffset,(WCHAR*)dn+dn->wDeviceOffset,(WCHAR*)dn+dn->wOutputOffset,dm);}}
    if(data)GlobalUnlock(dc_mode);
    if(dn)GlobalUnlock(names);
    GlobalFree(dc_mode);return dc;
}
/* Old PrintDlg may return application-managed copies even when the driver
 * flag was requested, especially for collated copies. Convert that result
 * before passing the device mode back through the PrintDlgEx interface. */
static BOOL normalize_driver_copies(HGLOBAL mode,BOOL ansi,WORD *copies,DWORD *flags)
{
    if(!mode || !copies || !*copies || *copies>32767 || !flags)return FALSE;
    if(*copies>1 || (*flags&PD_COLLATE)){
        SIZE_T needed=ansi?offsetof(DEVMODEA,dmCollate)+sizeof(((DEVMODEA *)0)->dmCollate):
            offsetof(DEVMODEW,dmCollate)+sizeof(((DEVMODEW *)0)->dmCollate);
        if(GlobalSize(mode)<needed)return FALSE;
        void *data=GlobalLock(mode);if(!data)return FALSE;
        if(ansi){DEVMODEA *dm=data;dm->dmCopies=(short)*copies;dm->dmCollate=(*flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;}
        else{DEVMODEW *dm=data;dm->dmCopies=(short)*copies;dm->dmCollate=(*flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;}
        GlobalUnlock(mode);
    }
    *copies=1;*flags &= ~(DWORD)PD_COLLATE;return TRUE;
}
static BOOL read_ranges(HWND window,state *s)
{
    s->count=0;
    if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)!=BST_CHECKED)return TRUE;
    WCHAR text[512];if(!GetWindowTextW(s->edit,text,512))return FALSE;
    return ntw16s_parse_print_ranges(text,s->minimum,s->maximum,s->ranges,s->capacity,&s->count);
}
static DWORD input_flags(HWND window,const state *s)
{
    DWORD flags=s->input_flags & ~(PD_PAGENUMS|PD_SELECTION|PD_COLLATE|PD_PRINTTOFILE);
    if(SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PAGENUMS;
    else if(SendDlgItemMessageW(window,1057,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_SELECTION;
    if(SendDlgItemMessageW(window,1041,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_COLLATE;
    if(SendDlgItemMessageW(window,1040,BM_GETCHECK,0,0)==BST_CHECKED)flags|=PD_PRINTTOFILE;
    return flags;
}
static BOOL save_snapshot(HWND window,state *s)
{
    BOOL translated=FALSE;DWORD copies=GetDlgItemInt(window,1154,&translated,FALSE);if(!translated || !copies || copies>32767 || !read_ranges(window,s))return FALSE;
    DWORD flags=input_flags(window,s);
    HGLOBAL mode=clone_global(*s->live_mode),names=(flags&PD_PRINTTOFILE)?clone_file_devnames(*s->live_names,s->ansi):clone_global(*s->live_names),dc_mode=clone_global(*s->live_mode);
    if(!mode || !names || !dc_mode){if(mode)GlobalFree(mode);if(names)GlobalFree(names);if(dc_mode)GlobalFree(dc_mode);return FALSE;}
    void *mode_data=GlobalLock(mode),*dc_data=GlobalLock(dc_mode);DEVNAMES *dn=GlobalLock(names);HDC dc=NULL;
    BOOL ready=mode_data && dc_data && dn;
    if(ready){
        if(s->ansi){DEVMODEA *dm=mode_data,*dc_dm=dc_data;dm->dmCopies=(flags&PD_USEDEVMODECOPIESANDCOLLATE)?(short)copies:1;dm->dmCollate=(flags&PD_USEDEVMODECOPIESANDCOLLATE)&&(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;dc_dm->dmCopies=(short)copies;dc_dm->dmCollate=(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
            if(flags & PD_RETURNDC)dc=CreateDCA((char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,(char*)dn+dn->wOutputOffset,dc_dm);
            else if(flags & PD_RETURNIC)dc=CreateICA((char*)dn+dn->wDriverOffset,(char*)dn+dn->wDeviceOffset,(char*)dn+dn->wOutputOffset,dc_dm);}
        else {DEVMODEW *dm=mode_data,*dc_dm=dc_data;dm->dmCopies=(flags&PD_USEDEVMODECOPIESANDCOLLATE)?(short)copies:1;dm->dmCollate=(flags&PD_USEDEVMODECOPIESANDCOLLATE)&&(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;dc_dm->dmCopies=(short)copies;dc_dm->dmCollate=(flags&PD_COLLATE)?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
            if(flags & PD_RETURNDC)dc=CreateDCW((WCHAR*)dn+dn->wDriverOffset,(WCHAR*)dn+dn->wDeviceOffset,(WCHAR*)dn+dn->wOutputOffset,dc_dm);
            else if(flags & PD_RETURNIC)dc=CreateICW((WCHAR*)dn+dn->wDriverOffset,(WCHAR*)dn+dn->wDeviceOffset,(WCHAR*)dn+dn->wOutputOffset,dc_dm);}}
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
    if(message==WM_INITDIALOG){s->initializing=TRUE;if(!SetPropW(window,CONTEXT_PROPERTY,s)){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        RECT first,last,ok;HWND a=GetDlgItem(window,1152),b=GetDlgItem(window,1153),button=GetDlgItem(window,IDOK);
        if(!a || !b || !button){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        GetWindowRect(a,&first);GetWindowRect(b,&last);GetWindowRect(button,&ok);
        MapWindowPoints(NULL,window,(POINT*)&first,2);MapWindowPoints(NULL,window,(POINT*)&last,2);MapWindowPoints(NULL,window,(POINT*)&ok,2);
        ShowWindow(GetDlgItem(window,1089),SW_HIDE);ShowWindow(GetDlgItem(window,1090),SW_HIDE);ShowWindow(a,SW_HIDE);ShowWindow(b,SW_HIDE);
        HFONT font=(HFONT)SendMessageW(button,WM_GETFONT,0,0);
        HDC dc=GetDC(window);int dpi=dc?GetDeviceCaps(dc,LOGPIXELSX):96;if(dc)ReleaseDC(window,dc);
        int inset=MulDiv(25,dpi,96),gap=MulDiv(8,dpi,96);
        s->edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",s->initial,WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,first.left-inset,first.top,last.right-first.left+inset,first.bottom-first.top,window,(HMENU)(INT_PTR)RANGE_CONTROL,GetModuleHandleW(NULL),NULL);
        s->apply=CreateWindowW(L"BUTTON",L"Apply",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_DISABLED|BS_PUSHBUTTON,ok.left-(ok.right-ok.left)-gap,ok.top,ok.right-ok.left,ok.bottom-ok.top,window,(HMENU)(INT_PTR)APPLY_CONTROL,GetModuleHandleW(NULL),NULL);
        if(!s->edit || !s->apply){s->failed=TRUE;PostMessageW(window,WM_COMMAND,IDCANCEL,0);return 0;}
        if(s->input_flags & PD_NOPAGENUMS)EnableWindow(s->edit,FALSE);
        if(s->edit)SendMessageW(s->edit,WM_SETFONT,(WPARAM)font,TRUE);
        if(s->apply)SendMessageW(s->apply,WM_SETFONT,(WPARAM)font,TRUE);
        s->initializing=FALSE;
    }
    if(s && message==WM_COMMAND){UINT id=LOWORD(wparam),notice=HIWORD(wparam);
        if(id==RANGE_CONTROL && notice==EN_SETFOCUS)SendDlgItemMessageW(window,1058,BM_CLICK,0,0);
        if(id==APPLY_CONTROL){if(save_snapshot(window,s))EnableWindow(s->apply,FALSE);else MessageBeep(MB_ICONEXCLAMATION);return 1;}
        if(!s->initializing && s->apply && ((notice==EN_CHANGE && (id==RANGE_CONTROL || id==1154)) ||
            (notice==CBN_SELCHANGE && id==1139) || (notice==BN_CLICKED && (id==1056 || id==1057 || id==1058 || id==1040 || id==1041 || id==1025))))EnableWindow(s->apply,TRUE);
        if(id==IDOK && !read_ranges(window,s)){MessageBeep(MB_ICONEXCLAMATION);return 1;}}
    if(message==WM_DESTROY)RemovePropW(window,CONTEXT_PROPERTY);
    return 0;
}
static UINT_PTR CALLBACK print_hook_a(HWND w,UINT m,WPARAM wp,LPARAM lp)
{state *s=NULL;if(m==WM_INITDIALOG){PRINTDLGA *p=(PRINTDLGA*)lp;s=(state*)p->lCustData;s->live_mode=&p->hDevMode;s->live_names=&p->hDevNames;}return print_hook_common(w,m,wp,s);}
static UINT_PTR CALLBACK print_hook_w(HWND w,UINT m,WPARAM wp,LPARAM lp)
{state *s=NULL;if(m==WM_INITDIALOG){PRINTDLGW *p=(PRINTDLGW*)lp;s=(state*)p->lCustData;s->live_mode=&p->hDevMode;s->live_names=&p->hDevNames;}return print_hook_common(w,m,wp,s);}
#define PRINT_EXTENDED(NAME,EX_TYPE,TYPE,PRINT_TYPE,HOOK,IS_ANSI) \
BOOL NAME(EX_TYPE *p,PRINT_TYPE print,HRESULT *result) \
{ \
    if(!p || p->lStructSize!=sizeof(*p) || !p->hwndOwner || !IsWindow(p->hwndOwner) || (p->Flags & ~(PD_NOPAGENUMS|PD_NOSELECTION|PD_NOWARNING|PD_HIDEPRINTTOFILE|PD_DISABLEPRINTTOFILE|PD_PRINTTOFILE|PD_COLLATE|PD_PAGENUMS|PD_SELECTION|PD_RETURNDC|PD_RETURNIC|PD_USEDEVMODECOPIESANDCOLLATE))!=0 || ((p->Flags&PD_SELECTION) && (p->Flags&(PD_NOSELECTION|PD_PAGENUMS))) || ((p->Flags&PD_PRINTTOFILE) && (p->Flags&(PD_HIDEPRINTTOFILE|PD_DISABLEPRINTTOFILE))) || ((p->Flags&PD_COLLATE) && (p->Flags&PD_USEDEVMODECOPIESANDCOLLATE)) || p->Flags2 || p->ExclusionFlags || p->hDC || (!!p->hDevMode != !!p->hDevNames) || (p->hDevMode && p->hDevMode==p->hDevNames) || !global_available(p->hDevMode) || !global_available(p->hDevNames) || ((p->Flags & PD_PAGENUMS) ? (!p->nPageRanges || p->nPageRanges>p->nMaxPageRanges || (p->Flags & PD_NOPAGENUMS)) : p->nPageRanges!=0) || (!(p->Flags & PD_NOPAGENUMS) && (!p->lpPageRanges || !p->nMaxPageRanges || p->nMaxPageRanges>MAX_RANGES || !p->nMinPage || p->nMaxPage>65535 || p->nMinPage>p->nMaxPage)) || !p->nCopies || p->nCopies>32767 || p->lpCallback || p->nPropertyPages || p->lphPropertyPages || p->nStartPage!=START_PAGE_GENERAL)return FALSE; \
    state s={0};s.input_flags=p->Flags;s.minimum=p->nMinPage;s.maximum=p->nMaxPage;s.capacity=p->nMaxPageRanges;s.ansi=IS_ANSI; \
    if(p->nPageRanges){if(!ntw16s_format_print_ranges(p->lpPageRanges,p->nPageRanges,p->nMinPage,p->nMaxPage,s.initial,512))return FALSE;}else wsprintfW(s.initial,L"%lu",p->nMinPage);TYPE d={0};d.lStructSize=sizeof(d);d.hwndOwner=p->hwndOwner;d.Flags=(p->Flags & ((p->Flags & PD_RETURNDC)?~(DWORD)PD_RETURNIC:~(DWORD)0))|PD_ENABLEPRINTHOOK;d.lpfnPrintHook=HOOK;d.lCustData=(LPARAM)&s;d.nMinPage=(WORD)(p->Flags & PD_NOPAGENUMS?1:p->nMinPage);d.nMaxPage=(WORD)(p->Flags & PD_NOPAGENUMS?1:p->nMaxPage);d.nFromPage=p->nPageRanges?(WORD)p->lpPageRanges[0].nFromPage:d.nMinPage;d.nToPage=p->nPageRanges?(WORD)p->lpPageRanges[0].nToPage:d.nMinPage;d.nCopies=(WORD)p->nCopies; \
    if(p->hDevMode){d.hDevMode=clone_global(p->hDevMode);if(!d.hDevMode)return FALSE;} \
    if(p->hDevNames){d.hDevNames=clone_global(p->hDevNames);if(!d.hDevNames){if(d.hDevMode)GlobalFree(d.hDevMode);return FALSE;}} \
    BOOL ok=print(&d);DWORD error=ok?0:CommDlgExtendedError(); \
    if(ok && (p->Flags & (PD_RETURNDC|PD_RETURNIC)) && (!d.hDC || ((p->Flags & PD_RETURNDC) && d.nCopies>1))){HDC replacement=create_print_dc(d.hDevMode,d.hDevNames,IS_ANSI,d.nCopies,d.Flags|p->Flags); \
        if(!replacement){if(d.hDC)DeleteDC(d.hDC);if(d.hDevMode)GlobalFree(d.hDevMode);if(d.hDevNames)GlobalFree(d.hDevNames);free_snapshot(&s);*result=E_FAIL;return TRUE;} \
        if(d.hDC){DeleteDC(d.hDC);}d.hDC=replacement;} \
    if(ok && (p->Flags & PD_USEDEVMODECOPIESANDCOLLATE) && !normalize_driver_copies(d.hDevMode,IS_ANSI,&d.nCopies,&d.Flags)){if(d.hDC)DeleteDC(d.hDC);if(d.hDevMode)GlobalFree(d.hDevMode);if(d.hDevNames)GlobalFree(d.hDevNames);free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(ok && (!global_available(p->hDevMode) || !global_available(p->hDevNames))){if(d.hDC)DeleteDC(d.hDC);if(d.hDevMode)GlobalFree(d.hDevMode);if(d.hDevNames)GlobalFree(d.hDevNames);free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(ok){if(p->hDevMode)GlobalFree(p->hDevMode);if(p->hDevNames)GlobalFree(p->hDevNames);p->hDevMode=d.hDevMode;p->hDevNames=d.hDevNames;p->hDC=d.hDC;p->Flags=(d.Flags & ~(PD_ENABLEPRINTHOOK|PD_RETURNDC|PD_RETURNIC))|(p->Flags & (PD_RETURNDC|PD_RETURNIC));p->nCopies=d.nCopies;p->nPageRanges=s.count;if(s.count)CopyMemory(p->lpPageRanges,s.ranges,s.count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_PRINT;free_snapshot(&s);*result=S_OK;return TRUE;} \
    if(d.hDC)DeleteDC(d.hDC); \
    if(d.hDevMode)GlobalFree(d.hDevMode); \
    if(d.hDevNames)GlobalFree(d.hDevNames); \
    if(s.failed){free_snapshot(&s);return FALSE;} \
    if(error){free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(s.applied && (!global_available(p->hDevMode) || !global_available(p->hDevNames))){free_snapshot(&s);*result=E_FAIL;return TRUE;} \
    if(s.applied){if(p->hDevMode)GlobalFree(p->hDevMode);if(p->hDevNames)GlobalFree(p->hDevNames);p->hDevMode=s.applied_mode;p->hDevNames=s.applied_names;p->hDC=s.applied_dc;p->Flags=s.flags;p->nCopies=s.copies;p->nPageRanges=s.applied_count;if(s.applied_count)CopyMemory(p->lpPageRanges,s.applied_ranges,s.applied_count*sizeof(PRINTPAGERANGE));p->dwResultAction=PD_RESULT_APPLY;} \
    else p->dwResultAction=PD_RESULT_CANCEL; \
    *result=S_OK;return TRUE; \
}
PRINT_EXTENDED(ntw16s_print_extended_a,PRINTDLGEXA,PRINTDLGA,ntw16s_print_dialog_a,print_hook_a,TRUE)
PRINT_EXTENDED(ntw16s_print_extended_w,PRINTDLGEXW,PRINTDLGW,ntw16s_print_dialog_w,print_hook_w,FALSE)
#undef PRINT_EXTENDED
