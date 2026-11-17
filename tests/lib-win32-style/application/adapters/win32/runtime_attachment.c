#include "lib-win32-style/application/adapters/win32/attachment.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME_ATTACHMENT FAIL line=%d error=%lu order=%s refs=%u bad=%u\n",__LINE__,GetLastError(),order,current->root.instance.calls,bad);return 1;}}while(0)
typedef struct fixture {style_runtime_attachment root;HWND window,child;unsigned disposed;} fixture;
static fixture fixtures[3],*current;static char order[32];static unsigned length,bad,failures;static BOOL reenter,policy_value=TRUE;static unsigned policy_calls,scroll_calls;
static style_runtime_attachment_ops ops;
static LRESULT CALLBACK observer(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static void record(char value){if(length+1<sizeof(order)){order[length++]=value;order[length]=0;}else ++bad;}
static void dispose(void *owner){++((fixture *)owner)->disposed;}
static BOOL cool(HWND w,void *owner){++policy_calls;if(w!=current->window || owner!=current)++bad;return policy_value;}
static BOOL host(HWND w){return IsWindow(w);}
static BOOL label(HWND w){++policy_calls;return w==current->window && policy_value;}
static BOOL frame_remove(HWND w,const void *owner)
{
    record('F');if(owner!=current || current->root.instance.calls!=2)++bad;
    if(reenter){reenter=FALSE;if(style_runtime_attachment_remove(w,observer,&ops) || GetLastError()!=ERROR_NOT_FOUND)++bad;}
    return FALSE;
}
static BOOL control_remove(HWND w,const void *owner)
{(void)w;record('C');if(owner!=current || current->root.instance.calls!=2)++bad;return TRUE;}
static void theme_remove(HWND w,const void *owner)
{record(owner?'t':'T');if(owner && (owner!=current || w!=current->child))++bad;}
static BOOL frame_terminal(HWND w)
{(void)w;record('F');if(failures&1){SetLastError(0x1111);return FALSE;}return TRUE;}
static BOOL control_terminal(HWND w)
{(void)w;record('C');if(failures&2){SetLastError(0x2222);return FALSE;}return TRUE;}
static BOOL theme_terminal(HWND w,const void *owner)
{(void)w;record(owner?'t':'T');if(failures&4){SetLastError(0x3333);return FALSE;}return TRUE;}
static void scrollbars(HWND w){if(w!=current->window)++bad;++scroll_calls;}
static BOOL create(fixture *f)
{
    HINSTANCE module=GetModuleHandleW(NULL);f->window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,120,80,NULL,NULL,module,NULL);
    f->child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,40,20,f->window,NULL,module,NULL);f->root.themed=f->child;
    style_instance_init(&f->root.instance,f,dispose);style_instance_retain(&f->root.instance);
    return f->window && f->child && style_instance_attach(&f->root.instance,f->window,observer,NULL);
}
typedef struct seed_context {style_runtime_attachment *root;fixture *targets;HWND parent;unsigned selected,disposed;} seed_context;
static void seed_dispose(void *context){++((seed_context *)context)->disposed;}
static void seed_discard(void *context)
{
    seed_context *c=context;++c->disposed;
    if(c->targets[0].root.instance.calls!=1 || c->targets[1].root.instance.calls!=1 ||
        c->root->runtime.roles.references.mdi.held || c->root->runtime.roles.references.popup.held)++bad;
    SetLastError(0x77);
}
static style_runtime_owner_roles select_owner(HWND w,void *context)
{
    seed_context *c=context;++c->selected;style_control_runtime *r=&c->root->runtime;
    if(c->root->instance.calls!=1 || r->cool.notify.parent!=c->parent || r->controlbar.parent!=c->parent ||
        r->roles.references.mdi.held!=&c->targets[0].root.instance || r->roles.references.popup.held!=&c->targets[1].root.instance ||
        c->targets[0].root.instance.calls!=2 || c->targets[1].root.instance.calls!=2 ||
        r->status.present || r->spin.candidate || r->edit.candidate || GetParent(w)!=c->parent)++bad;
    return (style_runtime_owner_roles){TRUE,FALSE,TRUE};
}
static BOOL seed_contract(void)
{
    fixture targets[2]={0};if(!create(&targets[0]) || !create(&targets[1]))return FALSE;
    HWND w=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,targets[0].child,NULL,GetModuleHandleW(NULL),NULL);if(!w)return FALSE;
    const style_control_runtime_candidates candidates={.static_candidate=TRUE,.scroll_candidate=TRUE,.edit_candidate=TRUE,.tab_candidate=TRUE,.vcl_report_candidate=TRUE,
        .track_candidate=TRUE,.color_candidate=TRUE,.bit_candidate=TRUE,.inplace_candidate=TRUE,.nav_candidate=TRUE,
        .group_frame_candidate=TRUE,.panel_frame_candidate=TRUE,.group_button_candidate=TRUE,.grid_candidate=TRUE,
        .vcl_toolbar_candidate=TRUE,.status_candidate=TRUE,.date_candidate=TRUE};
    const style_control_runtime_extras extra={.cool_candidate=TRUE,.box_candidate=TRUE,.controlbar_candidate=TRUE,
        .rich_month=TRUE,.rich_hotkey=TRUE,.rich_popup=TRUE,.rich_unicode=TRUE,.rich_candidate=TRUE,.vb_user_edge=TRUE,.vb_data_edge=TRUE};
    style_runtime_attachment root={0};seed_context context={&root,targets,targets[0].child,0,0};
    style_instance_init(&root.instance,&context,seed_dispose);HMODULE module=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    const style_runtime_attachment_seed seed={&candidates,&extra,module,NULL,TRUE,FALSE,TRUE,TRUE,TRUE,TRUE,
        targets[1].window,GetParent(targets[1].window),&targets[1].root.instance,observer,select_owner,&context};
    DWORD original=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);style_runtime_attachment_initialize(w,original,&root,&seed);
    style_control_runtime *r=&root.runtime;
    BOOL result=context.selected==1 && !bad && root.instance.calls==1 &&
        r->cool.candidate && r->cool.owner==module && r->cool.notify.window==w &&
        r->controlbar.candidate && r->box.candidate && r->rich.candidate && r->rich.parent==GetDesktopWindow() &&
        r->rich.month && r->rich.hotkey && r->rich.popup && r->rich.unicode &&
        r->group_frame.candidate && r->panel_frame.candidate && r->user.candidate && r->data.candidate &&
        r->status.present && r->spin.candidate && !r->spin.modern && r->roles.text.candidate &&
        r->scroll.candidate && r->scroll.parent==targets[0].child && r->edit.candidate && !r->link && !r->combo && !r->spinedit &&
        r->combo_parent==targets[0].child && r->combo_type==(original&3) && r->group.candidate && r->grid.candidate &&
        r->roles.inplace.candidate && r->roles.inplace.owner==module && r->roles.inplace.parent==targets[0].child &&
        r->toolbar.candidate && r->toolbar.owner==module && r->toolbar.parent==targets[0].child &&
        r->grid.owner==module && r->grid.parent==targets[0].child && r->edit_style==original && r->track.candidate && r->tab.candidate &&
        r->tab.parent==targets[0].child && r->report.binding.candidate && r->report.binding.parent==targets[0].child &&
        r->report.binding.owner==module && !r->report.tree_role && r->color.candidate && r->date.candidate && r->status.candidate && r->roles.bit.candidate &&
        r->report.window==w && !r->report.tree && r->report.module==module && r->report.parent==targets[0].child;
    style_instance blocker;style_instance_init(&blocker,NULL,NULL);
    if(!style_instance_attach(&blocker,w,observer,NULL))return FALSE;
    result=!style_runtime_attachment_publish(w,&root,observer,seed_discard) &&
        GetLastError()==ERROR_ALREADY_EXISTS && context.disposed==1 && !bad && result;
    result=style_instance_detach(&blocker) && result;
    style_runtime_attachment fresh={0};style_instance_init(&fresh.instance,&context,seed_dispose);
    style_instance_retain(&fresh.instance);SetLastError(0x55);
    result=style_runtime_attachment_publish(w,&fresh,observer,seed_discard) &&
        fresh.instance.calls==1 && style_instance_lookup(w,observer)==&fresh.instance && result;
    result=style_instance_detach(&fresh.instance) && result;
    style_instance_release(&fresh.instance);result=result && context.disposed==2;
    for(unsigned i=0;i<2;++i){result=style_instance_detach(&targets[i].root.instance) && result;style_instance_release(&targets[i].root.instance);result=targets[i].disposed==1 && DestroyWindow(targets[i].window) && result;}
    return result;
}
typedef struct install_context {
    style_runtime_attachment root;HWND target;
    unsigned themed,list,edit,removed,unthemed,disposed;
    BOOL fail_theme,fail_scroll,cancel;
} install_context;
static install_context *settling;
static void install_dispose(void *context){++((install_context *)context)->disposed;}
static BOOL install_theme(HWND w,const void *context)
{
    install_context *c=settling;++c->themed;
    if(w!=c->target || context!=c || c->root.themed!=w)++bad;
    if(c->cancel)c->root.instance.detached=TRUE;
    SetLastError(0x61);return !c->fail_theme;
}
static BOOL install_list(HWND w)
{++settling->list;if(w!=settling->target)++bad;SetLastError(0x62);return !settling->fail_scroll;}
static BOOL install_edit(HWND w)
{++settling->edit;if(w!=settling->target)++bad;SetLastError(0x63);return !settling->fail_scroll;}
static void install_untheme(HWND w,const void *owner)
{(void)w;if(owner)++bad;++settling->unthemed;SetLastError(0x64);}
static BOOL install_remove(HWND w)
{(void)w;++settling->removed;SetLastError(0x65);return FALSE;}
static BOOL install_combo(HWND w,PCOMBOBOXINFO info)
{return GetComboBoxInfo(w,info);}
static BOOL install_contract(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,120,80,NULL,NULL,module,NULL);
    if(!parent)return FALSE;
    const WCHAR *classes[]={L"LISTBOX",L"EDIT",L"COMBOBOX",L"STATIC"};BOOL result=TRUE;
    const style_runtime_attachment_install_ops v={install_combo,install_theme,install_list,install_edit,install_untheme,install_remove};
    for(unsigned kind=0;kind<4;++kind)for(unsigned mode=0;mode<6;++mode){
        DWORD flags=WS_CHILD|(kind==1?ES_MULTILINE:kind==2?CBS_DROPDOWN:0);
        HWND w=CreateWindowW(classes[kind],L"",flags,0,0,80,40,parent,NULL,module,NULL);if(!w){result=FALSE;break;}
        install_context c={0};settling=&c;style_instance_init(&c.root.instance,&c,install_dispose);style_instance_retain(&c.root.instance);
        c.target=w;if(kind==2){COMBOBOXINFO info={.cbSize=sizeof(info)};if(!GetComboBoxInfo(w,&info)){DestroyWindow(w);result=FALSE;break;}c.target=info.hwndList;}
        c.fail_theme=mode==1;c.fail_scroll=mode==2;c.cancel=mode==3;
        BOOL control=kind!=3;BOOL installed=mode!=4;DWORD error=0;
        SetLastError(0x60);BOOL done=style_runtime_attachment_install_finish(w,&c.root,flags,control,kind==1,mode==5,installed,&error,&v);
        /* file-owner affects only COMBOBOX targeting, not LISTBOX/EDIT. */
        BOOL has_target=control && !(kind==2 && mode==5);
        BOOL cancelled=has_target && c.cancel;
        BOOL expected=installed && !(has_target && (c.fail_theme || c.fail_scroll || c.cancel));
        unsigned themed=has_target && installed;
        DWORD expected_error=cancelled?ERROR_CANCELLED:themed?(c.fail_theme?0x61:kind==1?0x63:0x62):0x60;
        result=result && done==expected && error==expected_error && c.themed==themed &&
            c.list==(themed && !c.fail_theme && kind!=1) && c.edit==(themed && !c.fail_theme && kind==1) &&
            c.removed==(!expected && !cancelled) && c.unthemed==(expected && c.root.themed!=w) &&
            c.root.instance.calls==1 && !bad;
        c.root.instance.detached=TRUE;style_instance_release(&c.root.instance);result=result && c.disposed==1 && DestroyWindow(w);
    }
    settling=NULL;return DestroyWindow(parent) && result;
}
static unsigned render_mode;
static BOOL control_render(HWND w,style_control_runtime_kind kind,void *context,const BOOL *cancelled)
{
    install_context *c=settling;
    if(w!=c->target || context!=c || cancelled!=&c->root.instance.detached || kind!=(render_mode==9?STYLE_RUNTIME_GROUP:STYLE_RUNTIME_EDIT))++bad;
    if(render_mode==4)c->root.instance.detached=TRUE;
    SetLastError(0x66);return render_mode!=1;
}
static BOOL control_retire(HWND w,const void *context)
{
    if(w!=settling->target || context!=settling)++bad;
    ++settling->removed;SetLastError(0x67);return render_mode!=11;
}
static void control_untheme(HWND w,const void *owner)
{
    if(owner!=settling || w!=settling->target || settling->root.themed!=w)++bad;
    ++settling->unthemed;SetLastError(0x64);
}
static BOOL runtime_controls_contract(void)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,120,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)return FALSE;
    BOOL result=TRUE;
    for(render_mode=0;render_mode<12;++render_mode){
        HWND w=CreateWindowW(L"EDIT",L"",WS_CHILD|ES_MULTILINE,0,0,80,40,parent,NULL,GetModuleHandleW(NULL),NULL);if(!w)return FALSE;
        install_context c={0};settling=&c;c.target=w;c.fail_theme=render_mode==2;c.fail_scroll=render_mode==3;c.cancel=render_mode==10;
        style_instance_init(&c.root.instance,&c,install_dispose);style_instance_retain(&c.root.instance);
        c.root.runtime.edit_style=render_mode==8?0:ES_MULTILINE;
        c.root.runtime.link=render_mode==5;c.root.runtime.combo=render_mode==6;c.root.runtime.spinedit=render_mode==7;
        style_runtime_attachment_controls controls={&c.root,control_render,control_retire,install_theme,install_edit,control_untheme};
        style_control_runtime_kind kind=render_mode==9?STYLE_RUNTIME_GROUP:STYLE_RUNTIME_EDIT;
        BOOL done=style_runtime_attachment_control_install(w,kind,&controls,&c.root.instance.detached);DWORD error=GetLastError();
        BOOL theme=render_mode!=1 && render_mode!=4 && (render_mode<5 || render_mode>9);
        result=result && done==(render_mode!=1 && !(theme && (c.fail_theme || c.fail_scroll))) &&
            c.themed==(unsigned)theme && c.edit==(unsigned)(theme && !c.fail_theme) &&
            c.root.themed==(theme?w:NULL) && c.root.instance.calls==1 &&
            error==(theme?(c.fail_theme?0x61u:0x63u):0x66u);
        BOOL removed=style_runtime_attachment_control_remove(w,kind,&controls);
        result=result && removed==(render_mode!=11) && c.removed==1 && c.unthemed==(unsigned)theme && !c.root.themed;
        c.root.instance.detached=TRUE;style_instance_release(&c.root.instance);result=result && c.disposed==1 && DestroyWindow(w) && !bad;
    }
    settling=NULL;return DestroyWindow(parent) && result;
}
static fixture *checking,*replacement;
static unsigned data_calls,user_calls,current_mode;
static BOOL current_data(HWND w)
{
    ++data_calls;SetLastError(0x71);
    if(current_mode==3){
        current_mode=0;
        if(!style_instance_detach(&checking->root.instance))++bad;
        style_instance_init(&replacement->root.instance,replacement,dispose);style_instance_retain(&replacement->root.instance);
        if(!style_instance_attach(&replacement->root.instance,w,observer,NULL))++bad;
        replacement->root.runtime.toolbar.candidate=replacement->root.runtime.toolbar.installed=TRUE;
    }
    return current_mode!=1;
}
static BOOL current_user(HWND w)
{
    (void)w;++user_calls;SetLastError(0x72);
    if(current_mode==4 && !style_instance_detach(&checking->root.instance))++bad;
    return current_mode!=2;
}
static BOOL current_contract(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"TTabControl"};
    if(!RegisterClassW(&cls))return FALSE;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);
    fixture f={0},next={0};f.window=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,60,40,parent,NULL,module,NULL);
    if(!parent || !f.window)return FALSE;
    style_instance_init(&f.root.instance,&f,dispose);style_instance_retain(&f.root.instance);
    if(!style_instance_attach(&f.root.instance,f.window,observer,NULL))return FALSE;
    checking=&f;replacement=&next;style_control_runtime *r=&f.root.runtime;
    const style_runtime_attachment_policy policy={current_data,current_user,MCS_MULTISELECT};
    BOOL result=style_runtime_attachment_current(f.window,observer,&policy) && data_calls==1 && user_calls==1 && GetLastError()==0x72;
    struct pair {BOOL *candidate,*installed;} roles[]={
        {&r->rich.candidate,&r->rich.installed},{&r->box.candidate,&r->box.verified},
        {&r->combo,&r->edit.installed},{&r->group.candidate,&r->group.installed},
        {&r->grid.candidate,&r->grid.installed},{&r->toolbar.candidate,&r->toolbar.installed}};
    for(unsigned i=0;i<ARRAYSIZE(roles);++i){
        *r=(style_control_runtime){0};*roles[i].candidate=TRUE;
        result=style_runtime_attachment_current(f.window,observer,&policy) && result;
        *roles[i].installed=TRUE;data_calls=user_calls=0;
        result=!style_runtime_attachment_current(f.window,observer,&policy) && result;
        result=result && (i==5?(data_calls==1 && user_calls==1):(!data_calls && !user_calls));
    }
    *r=(style_control_runtime){0};r->tab.candidate=r->tab.installed=TRUE;r->tab.parent=parent;
    result=style_runtime_attachment_current(f.window,observer,&policy) && result;
    SetWindowLongPtrW(f.window,GWL_EXSTYLE,WS_EX_LAYOUTRTL);
    result=!style_runtime_attachment_current(f.window,observer,&policy) && result;
    SetWindowLongPtrW(f.window,GWL_EXSTYLE,0);r->tab.parent=NULL;
    result=!style_runtime_attachment_current(f.window,observer,&policy) && result;
    *r=(style_control_runtime){0};data_calls=user_calls=0;current_mode=1;
    result=!style_runtime_attachment_current(f.window,observer,&policy) && data_calls==1 && !user_calls && result;
    current_mode=2;result=!style_runtime_attachment_current(f.window,observer,&policy) && result;
    current_mode=3;result=!style_runtime_attachment_current(f.window,observer,&policy) && f.root.instance.detached && next.root.instance.calls==1 && result;
    result=style_instance_detach(&next.root.instance) && result;style_instance_release(&next.root.instance);
    style_instance_release(&f.root.instance);result=result && f.disposed==1 && next.disposed==1;
    fixture retired={0};checking=&retired;style_instance_init(&retired.root.instance,&retired,dispose);style_instance_retain(&retired.root.instance);
    result=style_instance_attach(&retired.root.instance,f.window,observer,NULL) && result;current_mode=4;
    result=style_runtime_attachment_current(f.window,observer,&policy) && retired.root.instance.detached && result;
    style_instance_release(&retired.root.instance);current_mode=0;data_calls=user_calls=0;
    result=style_runtime_attachment_current(f.window,observer,&policy) && data_calls==1 && user_calls==1 && retired.disposed==1 && !bad && result;
    return DestroyWindow(parent) && UnregisterClassW(cls.lpszClassName,module) && result;
}
static void reset(void){length=0;order[0]=0;}
static DWORD WINAPI foreign(void *context)
{
    HWND w=context;
    if(style_runtime_attachment_attached(w,observer,&ops) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 1;
    if(style_runtime_attachment_remove(w,observer,&ops) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 2;
    if(style_runtime_attachment_terminal(w,observer,&ops) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 3;
    style_runtime_attachment_scroll_changed(w,observer,&ops);return 0;
}
int main(void)
{
    ops=(style_runtime_attachment_ops){cool,host,label,observer,observer,frame_remove,control_remove,theme_remove,frame_terminal,control_terminal,theme_terminal,scrollbars,0};
    current=&fixtures[0];CHECK(seed_contract());CHECK(install_contract());CHECK(runtime_controls_contract());CHECK(current_contract());CHECK(create(current));HWND w=current->window;
    CHECK(style_runtime_attachment_attached(w,observer,&ops));
    style_control_runtime *r=&current->root.runtime;
    struct pair {BOOL *candidate,*installed;} roles[]={
        {&r->report.binding.candidate,&r->report.binding.installed},{&r->cool.candidate,&r->cool.verified},
        {&r->spin.modern,&r->spin.verified},{&r->data.candidate,&r->data.installed},{&r->roles.text.candidate,&r->roles.text.verified},
        {&r->scroll.candidate,&r->scroll.installed},{&r->edit.candidate,&r->edit.installed},{&r->group.candidate,&r->group.installed},
        {&r->grid.candidate,&r->grid.installed},{&r->toolbar.candidate,&r->toolbar.installed},{&r->track.candidate,&r->track.installed},
        {&r->tab.candidate,&r->tab.installed},{&r->color.candidate,&r->color.installed},{&r->date.candidate,&r->date.installed},
        {&r->status.candidate,&r->status.installed},{&r->roles.bit.candidate,&r->roles.bit.verified}};
    for(unsigned i=0;i<ARRAYSIZE(roles);++i){*roles[i].candidate=TRUE;CHECK(!style_runtime_attachment_attached(w,observer,&ops));*roles[i].installed=TRUE;CHECK(style_runtime_attachment_attached(w,observer,&ops));*roles[i].candidate=FALSE;*roles[i].installed=FALSE;}
    r->cool.candidate=r->cool.verified=TRUE;policy_value=FALSE;CHECK(!style_runtime_attachment_attached(w,observer,&ops));r->cool.candidate=FALSE;r->cool.verified=FALSE;
    r->user.candidate=TRUE;CHECK(!style_runtime_attachment_attached(w,observer,&ops));policy_value=TRUE;CHECK(style_runtime_attachment_attached(w,observer,&ops));r->user.candidate=FALSE;CHECK(policy_calls>=4 && !bad);
    style_runtime_attachment_scroll_changed(w,observer,&ops);CHECK(scroll_calls==1);
    HANDLE thread=CreateThread(NULL,0,foreign,w,0,NULL);DWORD code=1;CHECK(thread && WaitForSingleObject(thread,2000)==WAIT_OBJECT_0 && GetExitCodeThread(thread,&code) && !code && CloseHandle(thread) && !length && scroll_calls==1);
    current->root.instance.window=current->child;CHECK(!style_runtime_attachment_remove(w,observer,&ops) && GetLastError()==ERROR_INVALID_STATE && current->root.instance.calls==1 && !current->root.instance.detached && !length);current->root.instance.window=w;
    reenter=TRUE;CHECK(style_runtime_attachment_remove(w,observer,&ops) && !strcmp(order,"FCtT") && current->root.instance.detached && current->root.instance.calls==1 && !bad);
    CHECK(!style_runtime_attachment_remove(w,observer,&ops) && GetLastError()==ERROR_NOT_FOUND);
    style_instance_release(&current->root.instance);CHECK(current->disposed==1 && DestroyWindow(w));
    current=&fixtures[1];reset();CHECK(create(current));w=current->window;failures=7;SetLastError(0x4444);
    CHECK(!style_runtime_attachment_terminal(w,observer,&ops) && GetLastError()==0x1111 && !strcmp(order,"FCtT") && current->root.instance.calls==1 && current->root.instance.detached);
    style_instance_release(&current->root.instance);CHECK(current->disposed==1);reset();failures=0;SetLastError(0x5555);
    CHECK(style_runtime_attachment_terminal(w,observer,&ops) && GetLastError()==0x5555 && !strcmp(order,"FCT") && DestroyWindow(w));
    current=&fixtures[2];reset();CHECK(create(current));w=current->window;current->root.instance.window=current->child;failures=7;
    CHECK(!style_runtime_attachment_terminal(w,observer,&ops) && GetLastError()==ERROR_INVALID_STATE && !strcmp(order,"FCtT") && current->root.instance.calls==1 && !current->root.instance.detached);
    current->root.instance.window=w;CHECK(style_instance_detach(&current->root.instance));style_instance_release(&current->root.instance);CHECK(current->disposed==1 && DestroyWindow(w) && !bad);
    puts("RUNTIME_ATTACHMENT PASS public_root=1 late_control=12 current_roles=6 current_phase=1 replacement=1 tab_identity=1 install_settlement=24 seed_metadata=1 seed_phase=1 related_borrows=1 failed_publication_cleanup=1 native_publish=1 discard_order=1 roles=16 policy=1 thread=1 normal=1 reentrant=1 detach_failure=1 terminal_first_error=1 terminal_unowned=1 last_release=1 cleanup=1 no_style=1");return 0;
}
