#include "lib-win32-style/application/control/combo.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"COMBO line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct lease {HFONT font,source;unsigned refs;} lease;
static lease fonts[40];static unsigned acquired,live,retires,disposed;
static BOOL invalid_release,remove_on_set;static HFONT during_paint;
static style_control_combo combo;
static style_control_state state,child,successor;
static style_instance instance;
static HWND list;
static lease *find(HFONT font)
{for(unsigned i=acquired;i>0;--i)if(fonts[i-1].font==font)return &fonts[i-1];return NULL;}
static HFONT source(HFONT font){lease *item=find(font);return item?item->source:font;}
static HFONT acquire(HFONT original)
{
    if(acquired==ARRAYSIZE(fonts))return NULL;
    HFONT font=CreateFontW(-12-(int)acquired,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Arial");if(!font)return NULL;
    fonts[acquired++]=(lease){font,original,1};++live;return font;
}
static HFONT acquire_root(HWND w,HFONT font,BOOL native){(void)w;(void)native;return acquire(font);}
static void retain(HFONT font){lease *item=find(font);if(item)++item->refs;}
static void release(HFONT font)
{lease *item=find(font);if(item){if(!item->refs){invalid_release=TRUE;return;}if(!--item->refs){DeleteObject(item->font);--live;}}}
static style_control_state *lookup(HWND w){return w==list?&child:(successor.original_font?&successor:NULL);}
static BOOL retire(HWND w){(void)w;++retires;return TRUE;}
static void dispose(void *context)
{(void)context;++disposed;style_control_combo_dispose(&combo,&state);style_control_state_dispose(&state);}
static LRESULT CALLBACK witness(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_PRINTCLIENT){during_paint=(HFONT)SendMessageW(w,WM_GETFONT,0,0);return 0;}
    if(m==WM_SETFONT && remove_on_set){remove_on_set=FALSE;instance.detached=TRUE;style_control_combo_unbind(&combo,FALSE);}
    return DefSubclassProc(w,m,a,b);
}
typedef struct message_fixture {
    style_instance instance;
    style_control_state state;
    unsigned draws;
    BOOL buffered;
} message_fixture;
static void message_dispose(void *context){(void)context;}
static void message_draw(HWND window,void *context,HDC dc,BOOL buffered)
{(void)window;(void)dc;message_fixture *f=context;++f->draws;f->buffered=buffered;}
static LRESULT CALLBACK message_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;message_fixture *f=(message_fixture *)data;style_instance_retain(&f->instance);
    const style_control_draw draw={f,message_draw};
    LRESULT result=style_control_combo_message(w,&f->instance,&f->state,m,a,b,&draw);
    style_instance_release(&f->instance);return result;
}
static int message_tests(void)
{
    HWND root=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND window=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,0,100,80,root,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root && window);message_fixture f={0};style_instance_init(&f.instance,&f,message_dispose);
    CHECK(style_instance_attach(&f.instance,window,message_proc,NULL));
    CHECK(SendMessageW(window,CB_ADDSTRING,0,(LPARAM)L"first")==0 && SendMessageW(window,CB_ADDSTRING,0,(LPARAM)L"second")==1);
    CHECK(SendMessageW(window,CB_SETCURSEL,1,0)==1 && SendMessageW(window,CB_GETCURSEL,0,0)==1 && !f.state.suppression);
    SendMessageW(window,WM_SETREDRAW,FALSE,0);CHECK(f.state.redraw_off);
    SendMessageW(window,WM_SETREDRAW,TRUE,0);CHECK(!f.state.redraw_off);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,0);
    CHECK(f.draws==1 && !f.buffered);
    CHECK(style_instance_detach(&f.instance) && DeleteDC(dc) && DestroyWindow(root));return 0;
}
typedef struct setup_fixture {
    style_instance instance;
    style_control_combo combo;
    style_control_state child;
    HWND parent,list;
    unsigned queries,installs,edits,parent_removes,child_removes,disposed;
    BOOL query_fail,install_fail,install_detach,edit_fail,edit_detach,remove_fail;
} setup_fixture;
static setup_fixture *setup_active;
static void setup_dispose(void *context){++((setup_fixture *)context)->disposed;}
static LRESULT CALLBACK setup_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static style_control_state *setup_lookup(HWND w){return w==setup_active->list?&setup_active->child:NULL;}
static BOOL setup_remove(HWND w)
{
    setup_fixture *f=setup_active;
    if(w==f->list){++f->child_removes;f->child.owner=NULL;return TRUE;}
    ++f->parent_removes;if(f->remove_fail)return FALSE;
    (void)style_control_combo_list_retire(&f->combo,&f->instance,setup_lookup,setup_remove);
    return style_instance_detach(&f->instance);
}
static BOOL setup_info(HWND w,PCOMBOBOXINFO info)
{setup_fixture *f=setup_active;++f->queries;if(f->query_fail)return FALSE;info->hwndCombo=w;info->hwndList=f->list;info->hwndItem=w;return TRUE;}
static BOOL setup_list(HWND w,void *context)
{
    setup_fixture *f=context;++f->installs;if(w!=f->list)return FALSE;
    if(f->install_detach)(void)setup_remove(f->parent);
    return !f->install_fail;
}
static BOOL setup_edit(HWND parent,HWND child,void *context)
{
    setup_fixture *f=context;++f->edits;if(parent!=f->parent || child!=parent)return FALSE;
    if(f->edit_detach)(void)setup_remove(parent);
    if(f->edit_fail){SetLastError(ERROR_INVALID_DATA);return FALSE;}return TRUE;
}
static int setup_tests(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    for(unsigned mode=0;mode<7;++mode){
        setup_fixture f={0};setup_active=&f;
        f.parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,200,100,NULL,NULL,module,NULL);
        f.list=CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,0,100,80,f.parent,NULL,module,NULL);CHECK(f.parent && f.list);
        style_instance_init(&f.instance,&f,setup_dispose);CHECK(style_instance_attach(&f.instance,f.parent,setup_proc,NULL));style_instance_retain(&f.instance);
        f.query_fail=mode==1 || mode==6;f.install_fail=mode==2;f.install_detach=mode==3;f.edit_fail=mode==4;f.edit_detach=mode==5;f.remove_fail=mode==6;
        const style_control_combo_ops bindings={NULL,setup_lookup};const style_control_combo_setup_ops ops={&f,setup_info,setup_list,setup_remove,setup_edit};
        BOOL result=style_control_combo_setup(f.parent,&f.instance,&f.combo,&bindings,&ops);
        CHECK(result==(mode==0) && f.queries==1 && !f.disposed && f.instance.calls==1);
        if(mode==0){CHECK(f.combo.list==f.list && f.child.owner==&f.instance && f.edits==1 && !f.parent_removes);CHECK(setup_remove(f.parent));}
        else if(mode==6){CHECK(!f.instance.detached && !f.combo.list && !f.installs && f.parent_removes==1);f.remove_fail=FALSE;CHECK(setup_remove(f.parent));}
        else{CHECK(f.instance.detached && GetLastError()==ERROR_CANCELLED && !f.combo.list && !f.child.owner);}
        if(mode==3 || mode==4 || mode==5)CHECK(f.child_removes==1);
        if(mode==1 || mode==2)CHECK(!f.edits && !f.child_removes);
        style_instance_release(&f.instance);CHECK(f.disposed==1 && DestroyWindow(f.parent));
    }
    return 0;
}
int main(void)
{
    CHECK(!setup_tests());
    CHECK(!message_tests());
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND edit=parent?CreateWindowW(L"EDIT",L"",WS_CHILD,0,0,100,20,parent,NULL,GetModuleHandleW(NULL),NULL):NULL;
    list=parent?CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,20,100,60,parent,NULL,GetModuleHandleW(NULL),NULL):NULL;CHECK(parent && edit && list);
    HFONT app=(HFONT)GetStockObject(DEFAULT_GUI_FONT);SendMessageW(edit,WM_SETFONT,(WPARAM)app,FALSE);
    CHECK(SetWindowSubclass(edit,witness,1,0));
    const style_control_fonts operations={source,acquire_root,retain,release};const style_control_combo_ops ops={acquire,lookup};
    style_control_state_init(&state,&operations);style_instance_init(&instance,NULL,dispose);style_instance_retain(&instance);
    CHECK(style_control_combo_bind(&combo,&instance,&state,parent,edit,&ops));
    CHECK(combo.bound && instance.calls==2 && combo.font_count==1 && live==1 && (HFONT)SendMessageW(edit,WM_GETFONT,0,0)==app);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SendMessageW(edit,WM_PRINTCLIENT,(WPARAM)dc,0);DeleteDC(dc);CHECK(during_paint==combo.edit_font && state.native_paints==0);
    style_control_combo duplicate={0};CHECK(!style_control_combo_bind(&duplicate,&instance,&state,parent,edit,&ops) && GetLastError()==ERROR_ALREADY_EXISTS && instance.calls==2);
    for(unsigned i=1;i<33;++i)SendMessageW(edit,WM_SETFONT,(WPARAM)app,FALSE);
    CHECK(acquired==33 && combo.font_count==32 && live==32 && combo.edit_font==app && !invalid_release);
    style_control_combo_list_bind(&combo,&instance,list,lookup);CHECK(child.owner==&instance);
    child.owner=NULL;CHECK(style_control_combo_list_retire(&combo,&instance,lookup,retire) && !retires && !combo.list);
    style_control_combo_list_bind(&combo,&instance,list,lookup);CHECK(style_control_combo_list_retire(&combo,&instance,lookup,retire) && retires==1);
    successor.original_font=(HFONT)GetStockObject(SYSTEM_FONT);remove_on_set=TRUE;
    SendMessageW(edit,WM_SETFONT,(WPARAM)app,FALSE);
    CHECK(instance.detached && !combo.bound && instance.calls==1 && (HFONT)SendMessageW(edit,WM_GETFONT,0,0)==successor.original_font);
    style_instance_release(&instance);CHECK(disposed==1 && !live && !invalid_release);
    CHECK(RemoveWindowSubclass(edit,witness,1) && DestroyWindow(parent));
    puts("COMBO native edit/source/paint fonts, 32/33 pool boundary, duplicate bind, child replacement, setter removal/successor and final disposal PASS");return 0;
}
