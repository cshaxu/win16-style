#include "lib-win32-style/application/menu/owner.h"
#include <commctrl.h>
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"MENU_OWNER line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_menu_owner menu;
    style_menu_popup_scope *last_scope;
    unsigned entries,measures,draws,selected,initialized;
    BOOL fail_entry,stop_entry,stop_init,native_saw_original,application_mnemonic;
} fixture;
static BOOL accept_popup(void *context,HWND window)
{(void)context;(void)window;return TRUE;}
static const style_menu_popup_ops popup_ops={.accept=accept_popup};
static style_menu_popup_scope *begin(void *context,HWND window)
{
    fixture *f=context;++f->entries;
    if(f->fail_entry){f->fail_entry=FALSE;SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    style_menu_popup_scope *scope=style_menu_popup_begin(window,&popup_ops,f);
    f->last_scope=scope;
    if(f->stop_entry)style_menu_owner_stop(&f->menu);
    return scope;
}
static BOOL measure(void *context,style_menu_item *items,MEASUREITEMSTRUCT *value)
{
    fixture *f=context;
    if(value->CtlType!=ODT_MENU || !style_menu_item_find(items,value->itemData))return FALSE;
    ++f->measures;value->itemWidth=40;value->itemHeight=18;return TRUE;
}
static BOOL draw(void *context,style_menu_item *items,const DRAWITEMSTRUCT *value)
{
    fixture *f=context;
    if(value->CtlType!=ODT_MENU || !style_menu_item_find(items,value->itemData))return FALSE;
    ++f->draws;return TRUE;
}
static void selected(void *context,HWND window,BOOL value)
{fixture *f=context;(void)window;(void)value;++f->selected;}
static const style_menu_owner_ops adapted={begin,NULL,measure,draw,selected,TRUE};
static const style_menu_owner_ops native={.begin=begin};
static const style_menu_owner_ops *policy;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    fixture *f=(fixture *)data;LRESULT result=0;(void)id;
    if(style_menu_owner_message(&f->menu,w,m,a,b,policy,f,&result))return result;
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    fixture *f=(fixture *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(f && m==WM_INITMENUPOPUP){
        MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_FTYPE|MIIM_DATA};
        ++f->initialized;
        f->native_saw_original=GetMenuItemInfoW((HMENU)a,0,TRUE,&info) && !(info.fType&MFT_OWNERDRAW) && info.dwItemData==77;
        if(f->stop_init)style_menu_owner_stop(&f->menu);
        return 37;
    }
    if(f && m==WM_MENUCHAR)return MAKELRESULT(0,f->application_mnemonic?MNC_CLOSE:MNC_IGNORE);
    return DefWindowProcW(w,m,a,b);
}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicMenuOwner"};
    CHECK(RegisterClassW(&cls));
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,20,20,260,160,NULL,NULL,cls.hInstance,NULL);CHECK(window);
    HMENU menu=CreatePopupMenu();CHECK(menu && AppendMenuW(menu,MF_STRING,1,L"&Item"));
    MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_DATA,.dwItemData=77};CHECK(SetMenuItemInfoW(menu,0,TRUE,&info));
    for(unsigned mode=0;mode<2;++mode){
        fixture f={0};policy=mode?&native:&adapted;
        SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)&f);CHECK(SetWindowSubclass(window,dispatch,1,(DWORD_PTR)&f));
        SendMessageW(window,WM_ENTERMENULOOP,0,0);
        CHECK(f.menu.depth==1 && f.menu.loops);style_menu_loop *outer=f.menu.loops;
        f.fail_entry=TRUE;SendMessageW(window,WM_ENTERMENULOOP,0,0);
        CHECK(f.menu.depth==2 && f.menu.loops==outer);
        SendMessageW(window,WM_EXITMENULOOP,0,0);CHECK(f.menu.depth==1 && f.menu.loops==outer);
        CHECK(SendMessageW(window,WM_INITMENUPOPUP,(WPARAM)menu,MAKELPARAM(0,TRUE))==37);
        CHECK(f.native_saw_original && f.initialized==1);
        info.fMask=MIIM_FTYPE|MIIM_DATA;CHECK(GetMenuItemInfoW(menu,0,TRUE,&info));
        CHECK(!!(info.fType&MFT_OWNERDRAW)==!mode);
        if(!mode){
            MEASUREITEMSTRUCT size={.CtlType=ODT_MENU,.itemData=info.dwItemData};
            HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
            DRAWITEMSTRUCT item={.CtlType=ODT_MENU,.itemData=info.dwItemData,.hwndItem=(HWND)menu,.hDC=dc};
            CHECK(SendMessageW(window,WM_MEASUREITEM,0,(LPARAM)&size)==TRUE && size.itemWidth==40);
            LRESULT drawn=SendMessageW(window,WM_DRAWITEM,0,(LPARAM)&item);
            printf("MENU_OWNER_DRAW native_result=%lld measures=%u draws=%u\n",(long long)drawn,f.measures,f.draws);
            CHECK(drawn==TRUE && f.measures==1 && f.draws==1 && DeleteDC(dc));
            CHECK(SendMessageW(window,WM_MENUCHAR,L'i',(LPARAM)menu)==MAKELRESULT(0,MNC_EXECUTE));
            f.application_mnemonic=TRUE;
            CHECK(SendMessageW(window,WM_MENUCHAR,L'i',(LPARAM)menu)==MAKELRESULT(0,MNC_CLOSE));
            CHECK(CheckMenuItem(menu,1,MF_BYCOMMAND|MF_CHECKED)!=(UINT)-1);
            CHECK(SendMessageW(window,WM_INITMENUPOPUP,(WPARAM)menu,0)==37 && f.native_saw_original);
        }
        SendMessageW(window,WM_EXITMENULOOP,0,0);CHECK(!f.menu.loops && !f.menu.depth && !f.menu.items);
        info.fMask=MIIM_FTYPE|MIIM_DATA;CHECK(GetMenuItemInfoW(menu,0,TRUE,&info) && !(info.fType&MFT_OWNERDRAW) && info.dwItemData==77);
        SendMessageW(window,WM_ENTERMENULOOP,0,0);f.stop_init=TRUE;
        SendMessageW(window,WM_INITMENUPOPUP,(WPARAM)menu,0);CHECK(f.menu.detached && !f.menu.items && !f.menu.loops);
        style_menu_owner_stop(&f.menu);CHECK(RemoveWindowSubclass(window,dispatch,1));
        fixture entry={.stop_entry=TRUE};
        CHECK(!style_menu_owner_enter(&entry.menu,window,policy,&entry) && entry.menu.detached && !entry.menu.loops);
        CHECK(entry.last_scope && !style_menu_popup_scope_context(entry.last_scope));
    }
    SetWindowLongPtrW(window,GWLP_USERDATA,0);
    ShowWindow(window,SW_SHOWNORMAL);SetActiveWindow(window);
    HWND edit=CreateWindowW(L"EDIT",L"Focus",WS_CHILD|WS_VISIBLE,0,0,100,24,window,NULL,cls.hInstance,NULL);CHECK(edit);
    SetFocus(edit);CHECK(GetFocus()==edit);
    style_menu_owner bar={0};
    CHECK(style_menu_bar_select(&bar,window,2) && bar.bar.keyboard && bar.bar.focus==edit &&
        bar.bar.selected==2 && bar.bar.highlighted==2 && GetFocus()==window);
    style_menu_bar_leave(&bar,window,TRUE);
    CHECK(!bar.bar.keyboard && !bar.bar.focus && bar.bar.selected==-1 && GetFocus()==edit);
    CHECK(style_menu_bar_step(0,-1,3,TRUE)==-2 && style_menu_bar_step(-2,-1,3,TRUE)==2 &&
        style_menu_bar_step(2,1,3,FALSE)==0 && style_menu_bar_step(1,0,3,FALSE)==1);
    CHECK(style_menu_bar_select(&bar,window,1));SetFocus(edit);
    style_menu_bar_leave(&bar,window,TRUE);CHECK(GetFocus()==edit);
    bar.detached=TRUE;CHECK(!style_menu_bar_select(&bar,window,0) && GetFocus()==edit);
    CHECK(DestroyMenu(menu) && DestroyWindow(window) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("MENU_OWNER_OK native=1 adapted=1 depth=1 failed_inner=1 restore=1 mnemonic_priority=1 reentry=1");return 0;
}
