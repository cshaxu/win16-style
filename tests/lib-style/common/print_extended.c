#include "lib-style/common/print_extended.h"
static BOOL accepted,check_resources,resources_ok;static unsigned calls;
static HGLOBAL original_mode,original_names;
static BOOL copied(HGLOBAL mode,HGLOBAL names)
{
    if(!check_resources)return TRUE;
    if(!mode || !names || mode==original_mode || names==original_names)return FALSE;
    BYTE *m=GlobalLock(mode),*n=GlobalLock(names);
    BOOL ok=m && n && m[0]==0x35 && n[0]==0x72;
    if(m)GlobalUnlock(mode);
    if(n)GlobalUnlock(names);
    return ok;
}
static BOOL WINAPI fake_a(PRINTDLGA *p)
{++calls;resources_ok=p && copied(p->hDevMode,p->hDevNames);return p && (p->Flags&PD_ENABLEPRINTHOOK) && p->lpfnPrintHook && resources_ok && accepted;}
static BOOL WINAPI fake_w(PRINTDLGW *p)
{++calls;resources_ok=p && copied(p->hDevMode,p->hDevNames);return p && (p->Flags&PD_ENABLEPRINTHOOK) && p->lpfnPrintHook && resources_ok && accepted;}
#define RESOURCE_CASE(type,fn,callback) do { \
    type p={.lStructSize=sizeof(p),.hwndOwner=owner,.Flags=PD_NOPAGENUMS,.nCopies=1,.nStartPage=START_PAGE_GENERAL}; \
    original_mode=GlobalAlloc(GMEM_MOVEABLE,32);original_names=GlobalAlloc(GMEM_MOVEABLE,32); \
    if(!original_mode || !original_names)return 8; \
    BYTE *m=GlobalLock(original_mode),*n=GlobalLock(original_names);if(!m || !n)return 9; \
    m[0]=0x35;n[0]=0x72;GlobalUnlock(original_mode);GlobalUnlock(original_names); \
    p.hDevMode=original_mode;p.hDevNames=original_names;check_resources=TRUE;accepted=FALSE; \
    if(!fn(&p,callback,&result) || !resources_ok || p.dwResultAction!=PD_RESULT_CANCEL || \
       p.hDevMode!=original_mode || p.hDevNames!=original_names || GlobalSize(original_mode)<32 || GlobalSize(original_names)<32)return 10; \
    accepted=TRUE;if(!fn(&p,callback,&result) || !resources_ok || p.dwResultAction!=PD_RESULT_PRINT || \
       p.hDevMode==original_mode || p.hDevNames==original_names || !copied(p.hDevMode,p.hDevNames))return 11; \
    GlobalFree(p.hDevMode);GlobalFree(p.hDevNames);check_resources=FALSE; \
} while(0)
int main(void)
{
    HWND owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!owner)return 1;
    PRINTDLGEXA a={.lStructSize=sizeof(a),.hwndOwner=owner,.Flags=PD_NOPAGENUMS,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
    PRINTDLGEXW w={.lStructSize=sizeof(w),.hwndOwner=owner,.Flags=PD_NOPAGENUMS,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
    HRESULT result=E_FAIL;
    if(!style_print_extended_a(&a,fake_a,&result) || result!=S_OK || a.dwResultAction!=PD_RESULT_CANCEL || calls!=1)return 2;
    if(!style_print_extended_w(&w,fake_w,&result) || result!=S_OK || w.dwResultAction!=PD_RESULT_CANCEL || calls!=2)return 3;
    accepted=TRUE;
    if(!style_print_extended_a(&a,fake_a,&result) || result!=S_OK || a.dwResultAction!=PD_RESULT_PRINT || calls!=3)return 4;
    if(!style_print_extended_w(&w,fake_w,&result) || result!=S_OK || w.dwResultAction!=PD_RESULT_PRINT || calls!=4)return 5;
    a.Flags|=PD_ENABLEPRINTHOOK;
    if(style_print_extended_a(&a,fake_a,&result) || calls!=4)return 6;
    w.Flags2=1;
    if(style_print_extended_w(&w,fake_w,&result) || calls!=4)return 7;
    RESOURCE_CASE(PRINTDLGEXA,style_print_extended_a,fake_a);
    RESOURCE_CASE(PRINTDLGEXW,style_print_extended_w,fake_w);
    DestroyWindow(owner);return 0;
}
