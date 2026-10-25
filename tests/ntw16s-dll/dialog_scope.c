#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "isolated_process.h"
/* Account only for the real coordinator's own allocations; USER/IME may
 * initialize unrelated process GDI caches after a dialog is already closed. */
static void *records[256];static unsigned live;static BOOL fail_next;
static void *scope_calloc(size_t n,size_t s)
{if(fail_next){fail_next=FALSE;return NULL;}void *p=calloc(n,s);if(p){if(live==256)abort();records[live++]=p;}return p;}
static void scope_free(void *p)
{if(p){for(unsigned i=0;i<live;++i)if(records[i]==p){records[i]=records[--live];free(p);return;}abort();}}
#define calloc scope_calloc
#define free scope_free
#include "../../src/ntw16s-dll/attachment.c"
#undef calloc
#undef free
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DIALOG_SCOPE_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static BOOL nested,detach,unsupported;static unsigned failures,observed;
static void finish(HWND w)
{
    if(!Ntw16sWindowAttached(w))++failures;
    if(detach && (!Ntw16sWindowRemove(w) || Ntw16sWindowAttached(w)))++failures;
    ++observed;EndDialog(w,73);
}
static VOID CALLBACK finish_timer(HWND w,UINT m,UINT_PTR id,DWORD time)
{(void)m;(void)time;KillTimer(w,id);finish(w);}
static INT_PTR CALLBACK dialog(HWND w,UINT m,WPARAM a,LPARAM b)
{
    (void)a;(void)b;if(m!=WM_INITDIALOG)return FALSE;
    ntw16s_creation_scope *inner=nested?Ntw16sCreationBegin(TRUE):NULL;
    if(nested && !inner)++failures;
    LONG_PTR original_ex=GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(unsupported){SetWindowLongPtrW(w,GWL_EXSTYLE,original_ex|WS_EX_LAYERED);SetLayeredWindowAttributes(w,0,255,LWA_ALPHA);}
    ShowWindow(w,SW_HIDE);ShowWindow(w,SW_SHOW);UpdateWindow(w);
    if(!IsWindowVisible(w) || Ntw16sWindowAttached(w)==unsupported)++failures;
    if(inner){ntw16s_creation_result result={0};if(!Ntw16sCreationEnd(inner,&result) || result.failure || result.installed)++failures;}
    if(unsupported){SetWindowLongPtrW(w,GWL_EXSTYLE,original_ex);if(!SetTimer(w,1,10,finish_timer))++failures;return TRUE;}
    finish(w);return TRUE;
}
static LRESULT CALLBACK reject(HWND w,UINT m,WPARAM a,LPARAM b)
{return m==WM_CREATE?-1:DefWindowProcW(w,m,a,b);}
static unsigned destroy_callback;static BOOL draw_mutated;
static LRESULT CALLBACK owned_menu(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_MEASUREITEM){MEASUREITEMSTRUCT *item=(MEASUREITEMSTRUCT*)b;
        if(item->CtlType==ODT_MENU && item->itemData==0x5921){
            item->itemWidth=87;item->itemHeight=22;
            if(destroy_callback==1)DestroyWindow(w);
            if(destroy_callback==3){MENUITEMINFOW changed={.cbSize=sizeof(changed),.fMask=MIIM_DATA,.dwItemData=0x5929};
                if(!SetMenuItemInfoW(GetMenu(w),0,TRUE,&changed))++failures;}
            return TRUE;}}
    if(m==WM_DRAWITEM){DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT*)b;
        if(item->CtlType==ODT_MENU && item->itemData==0x5921){
            FillRect(item->hDC,&item->rcItem,(HBRUSH)GetStockObject(GRAY_BRUSH));
            if(destroy_callback==2)DestroyWindow(w);
            if(destroy_callback==4){MENUITEMINFOW changed={.cbSize=sizeof(changed),.fMask=MIIM_DATA,.dwItemData=0x5933};
                if(!SetMenuItemInfoW(GetMenu(w),1,TRUE,&changed))++failures;
                draw_mutated=TRUE;}
            return TRUE;}}
    if(m==WM_MEASUREITEM && ((MEASUREITEMSTRUCT*)b)->itemData==0x5922){
        ((MEASUREITEMSTRUCT*)b)->itemWidth=45;((MEASUREITEMSTRUCT*)b)->itemHeight=22;return TRUE;}
    if(m==WM_DRAWITEM && ((DRAWITEMSTRUCT*)b)->itemData==0x5922 && draw_mutated)++failures;
    return DefWindowProcW(w,m,a,b);
}
static int owner_reentry(void)
{
    WNDCLASSW c={.lpfnWndProc=owned_menu,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ScopeOwnerMenu"};
    CHECK(RegisterClassW(&c));
    for(unsigned mode=0;mode<5;++mode){
        destroy_callback=0;draw_mutated=FALSE;
        HWND w=CreateWindowW(c.lpszClassName,L"Owner callback lifetime",WS_OVERLAPPEDWINDOW,0,0,320,180,NULL,NULL,c.hInstance,NULL);
        HMENU menu=CreateMenu();CHECK(w && menu && AppendMenuW(menu,MF_OWNERDRAW,81,(LPCWSTR)(ULONG_PTR)0x5921) && SetMenu(w,menu));
        if(mode==4)CHECK(AppendMenuW(menu,MF_OWNERDRAW,82,(LPCWSTR)(ULONG_PTR)0x5922));
        destroy_callback=mode;BOOL result=win16_style_install(w,NULL);
        if(!mode){CHECK(result && win16_style_remove(w) && DestroyWindow(w));}
        else if(mode==1)CHECK(!result && !IsWindow(w));
        else if(mode==3){CHECK(!result && IsWindow(w));MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_DATA};
            CHECK(GetMenuItemInfoW(menu,0,TRUE,&item) && item.dwItemData==0x5929 && DestroyWindow(w));}
        else {
            CHECK(result);HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
            BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=640,.biHeight=-360,.biPlanes=1,.biBitCount=32}};
            void *pixels=NULL;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap);
            HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && old!=HGDI_ERROR);
            (void)PrintWindow(w,dc,0);
            if(mode==2)CHECK(!IsWindow(w));
            else {MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_DATA};
                CHECK(draw_mutated && !failures && GetMenuItemInfoW(menu,1,TRUE,&item) && item.dwItemData==0x5933);
                CHECK(win16_style_remove(w) && DestroyWindow(w));}
            CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc));
        }
        CHECK(!IsMenu(menu));
    }
    CHECK(UnregisterClassW(c.lpszClassName,c.hInstance));return 0;
}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;CHECK(!owner_reentry());fail_next=TRUE;CHECK(!Ntw16sCreationBegin(TRUE) && !live);
    struct {DLGTEMPLATE d;WORD menu,cls,title;} t={.d={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU,.cx=180,.cy=100}};
    for(unsigned i=0;i<12;++i){nested=(i&1)!=0;detach=(i&2)!=0;unsupported=(i&4)!=0;
        ntw16s_creation_scope *scope=Ntw16sCreationBegin(TRUE);CHECK(scope);
        unsigned before=observed;CHECK(DialogBoxIndirectParamW(GetModuleHandleW(NULL),&t.d,NULL,dialog,0)==73);
        ntw16s_creation_result result={0};CHECK(Ntw16sCreationEnd(scope,&result));
        printf("SCOPE_DIAG i=%u installed=%lu failure=%lu observed=%u before=%u errors=%u live=%u current=%p\n",i,(unsigned long)result.installed,(unsigned long)result.failure,observed,before,failures,live,(void*)current);
        CHECK(result.installed==1 && !result.failure && observed==before+1 && !failures && !live && !current && !creation_before_hook && !creation_return_hook);
    }
    WNDCLASSW c={0};c.lpfnWndProc=reject;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=L"ScopeRejectedCreate";
    CHECK(RegisterClassW(&c));ntw16s_creation_scope *scope=Ntw16sCreationBegin(TRUE);CHECK(scope);
    CHECK(!CreateWindowW(c.lpszClassName,L"reject",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,c.hInstance,NULL));
    CHECK(Ntw16sCreationEnd(scope,NULL) && !live && !current && !creation_before_hook && !creation_return_hook);
    scope=Ntw16sCreationBegin(TRUE);CHECK(scope);fail_next=TRUE;
    CHECK(!CreateWindowW(c.lpszClassName,L"pending OOM",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,c.hInstance,NULL));
    ntw16s_creation_result failed={0};CHECK(Ntw16sCreationEnd(scope,&failed) && failed.failure==ERROR_NOT_ENOUGH_MEMORY && !live && !current && !creation_before_hook && !creation_return_hook);CHECK(UnregisterClassW(c.lpszClassName,c.hInstance));
    printf("DIALOG_SCOPE_OWNERSHIP_OK bits=%u dialogs=12 nested_owner=1 detach=1 failed_create=1 unsupported_retry=1 owner_callback_destroy=2 owner_callback_mutation=2 oom=1 allocations=0\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
