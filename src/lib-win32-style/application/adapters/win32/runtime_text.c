#include "runtime_text.h"
#include "../../control/report.h"
#include "../../control/control_class.h"
static _Thread_local style_runtime_text_scope *current,*native_current;
void style_runtime_native_paint_begin(style_runtime_text_scope *scope,HWND w,UINT message,HDC dc)
{
    *scope=(style_runtime_text_scope){0};
    if(message==WM_PAINT || message==WM_PRINT || message==WM_PRINTCLIENT || message==WM_ERASEBKGND){
        scope->native_previous=native_current;scope->native_window=w;scope->native_dc=dc;
        scope->native_active=TRUE;native_current=scope;
    }
}
void style_runtime_text_begin(style_runtime_text_scope *scope,HWND w,UINT message,void *context,const style_runtime_text_ops *ops)
{
    style_runtime_native_paint_begin(scope,w,message,NULL);
    if(!ops->include_text(w,message))return;
    scope->previous=current;scope->active=TRUE;scope->context=context;current=scope;
    const style_runtime_text_role role=ops->role(w,ops->context);scope->window=role.window;scope->label_only=role.label_only;
}
void style_runtime_text_end(style_runtime_text_scope *scope)
{
    if(scope->native_active)native_current=scope->native_previous;
    if(scope->active)current=scope->previous;
}
const style_runtime_text_scope *style_runtime_text_current(void){return current;}
BOOL style_runtime_text_native_target(HDC dc,HWND *w)
{*w=native_current?native_current->native_window:NULL;return dc && native_current && native_current->native_dc==dc;}
void style_runtime_text_report_enable(style_runtime_text_scope *scope,HWND w,HMODULE module,const BOOL *cancelled,BOOL (*label)(HWND))
{
    if(!scope->active || !module)return;
    scope->report=TRUE;scope->window=w;scope->report_module=module;scope->cancelled=cancelled;
    if(label(w))scope->label_only=TRUE;
}
void style_runtime_text_report_bind(HWND w,HDC dc)
{if(current && current->report && current->window==w)current->report_dc=dc;}
BOOL style_runtime_text_report_active(void){return current && current->report;}
BOOL style_runtime_text_report_target(HDC dc,HWND *w)
{
    *w=NULL;
    if(!current || !current->report || !dc || current->report_dc!=dc || (current->cancelled && *current->cancelled))return FALSE;
    *w=current->window;return *w!=NULL;
}
BOOL style_runtime_text_report_caller(HMODULE module)
{return current && current->report && (module==current->report_module || module==GetModuleHandleW(NULL));}
BOOL style_runtime_text_active(void)
{return current && current->window && (!current->cancelled || !*current->cancelled);}
BOOL style_runtime_text_direct_allowed(void){return !current || !current->label_only;}

BOOL style_runtime_text_caller(const void *address)
{
    HMODULE caller=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller))return FALSE;
    return style_runtime_text_report_caller(caller) || caller==GetModuleHandleW(L"MSVBVM60.DLL") || caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"USER32.DLL") ||
        caller==GetModuleHandleW(L"LPK.DLL") || caller==GetModuleHandleW(L"USP10.DLL") || caller==GetModuleHandleW(L"GDI32FULL.DLL");
}
BOOL style_runtime_text_label_caller(const void *address)
{
    if(style_runtime_text_direct_allowed())return TRUE;
    HMODULE caller=NULL;
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)address,&caller) && (caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"MSVBVM60.DLL"));
}


void style_runtime_text_prepare_report(style_runtime_text_scope *paint,HWND w,WPARAM p,style_instance *instance,
    style_control_runtime *runtime,BOOL file_owner,SUBCLASSPROC notify,void *owner,BOOL (*label)(HWND))
{
    if(paint->native_active)paint->native_dc=(HDC)p;
    if(paint->active && !instance->detached && !runtime->protocol && !runtime->report.binding.candidate)
        style_control_report_bind(w,instance,runtime,notify,owner);
    if(paint->active && runtime->report.module && !file_owner && !instance->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((runtime->report.tree || (((style&LVS_TYPEMASK)==LVS_REPORT || runtime->report.binding.candidate) &&
            !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) && !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0) &&
            !instance->detached && (DWORD)GetWindowLongPtrW(w,GWL_STYLE)==style)) && !instance->detached)
            style_runtime_text_report_enable(paint,w,runtime->report.module,&instance->detached,label);
    }
}

static BOOL include_text(HWND w,UINT message)
{
    BOOL edit=style_control_vb_edit(w);
    if(message!=WM_PAINT && message!=WM_PRINT && message!=WM_PRINTCLIENT && !edit)return FALSE;
    return message!=WM_NCDESTROY && message!=WM_DESTROY;
}
static style_runtime_text_role text_role(HWND w,void *context)
{
    BOOL (*user_label_target)(HWND)=*(BOOL (**)(HWND))context;
    BOOL label_only=style_control_vb_picture(w) || user_label_target(w);
    HWND target=style_control_vb_frame(w) || style_control_vb_edit(w) || label_only ||
        (style_control_vb_class(w,L"ThunderRT6FormDC") || style_control_vb5_class(w,L"ThunderRT5Form"))?w:NULL;
    return (style_runtime_text_role){target,label_only};
}
void style_runtime_vb_paint_begin(style_runtime_text_scope *scope,HWND w,UINT message,style_font_cache *fonts,unsigned points,const style_font_ops *font_ops,BOOL (*user_label_target)(HWND))
{
    const style_runtime_text_ops ops={&user_label_target,include_text,text_role};
    style_runtime_text_begin(scope,w,message,fonts,&ops);
    scope->font_recipe=points;scope->font_ops=font_ops;
}
HFONT style_runtime_vb_paint_font(HDC dc)
{
    const style_runtime_text_scope *scope=style_runtime_text_current();HFONT source;
    /* The scope also covers a runtime-owned memory DC during WM_PRINT or
     * double buffering. Never change a DC belonging to another HWND. */
    if(!style_runtime_text_active() || !scope->context || !scope->font_recipe || (scope->report && dc!=scope->report_dc) || (WindowFromDC(dc) && WindowFromDC(dc)!=scope->window))return NULL;
    style_font_cache *fonts=scope->context;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source)return NULL;
    if(scope->report){LOGFONTW lf;
        if(GetObjectW(source,sizeof(lf),&lf)!=sizeof(lf) || lf.lfEscapement || lf.lfOrientation)return NULL;}
    return style_font_cache_get(fonts,source,scope->font_recipe,scope->font_ops);
}

BOOL style_runtime_vb_decode_glyphs(HDC dc,const WCHAR *input,UINT count,WCHAR *decoded)
{
    const style_runtime_text_scope *scope=style_runtime_text_current();
    if(!scope || !scope->window || !scope->context || !scope->font_recipe)return FALSE;
    return style_font_cache_decode(scope->context,dc,input,count,decoded,
        (style_font_glyph_range){1252,32,224});
}
