#include "control.h"
#include "button_paint.h"
#include "static_paint.h"
#include "edit_paint.h"
#include "list_combo.h"
#include "scroll.h"
#include "tab_header.h"
#include "report_paint.h"
#include "progress_paint.h"
#include "trackbar_paint.h"
#include "updown_paint.h"
#include "bars.h"
#include "date_paint.h"
#include "lib-win32-style/application/control/combo.h"
#include "lib-win32-style/application/control/list.h"
#include "lib-win32-style/application/control/edit.h"
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/basic.h"
#include "lib-win32-style/application/control/retire.h"
#include "lib-win32-style/application/control/native_overlay.h"
#include "lib-win32-style/application/control/control_class.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "lib-win32-style/application/lifecycle/dispatch.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <stdlib.h>
#include <string.h>

extern IMAGE_DOS_HEADER __ImageBase;
typedef enum win2k_report_role { WIN2K_REPORT_NONE,WIN2K_REPORT_LIST,WIN2K_REPORT_TREE } win2k_report_role;
typedef enum win2k_bar_role { WIN2K_BAR_NONE,WIN2K_BAR_TOOL,WIN2K_BAR_REBAR,WIN2K_BAR_STATUS } win2k_bar_role;
typedef struct win2k_control {
    style_instance instance;
    style_control_state state;
    style_native_theme_fn native_theme;
    BOOL frame_owned;
    classic_push_colors push;
    classic_choice_colors choice;
    classic_text_colors label;
    classic_edit_border edit[CLASSIC_EDIT_STATE_COUNT];
    classic_list_colors list;
    classic_combo_colors combo_colors;
    classic_tab_colors tab_colors;
    classic_header_colors header_colors;
    classic_progress_colors progress_colors[2];
    classic_trackbar_colors trackbar_colors[2];
    classic_group_colors group;
    classic_frame_colors frame;
    style_control_notify_state parent_binding;
    style_control_combo combo;
    HWND combo_parent;
    win2k_scroll scroll;
    style_control_native_state native;
    BOOL tab_control,header_control,track_control,updown_control;
    BOOL progress_control,legacy_progress;
    win2k_report_role report;
    win2k_bar_role bar;
    style_control_status_state status;
    BOOL date_control,month_control;
    style_control_native_combo_state date;
    style_calendar_colors calendar;
    HWND date_popup;
    style_control_protocol *protocol;
} win2k_control;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK parent_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static win2k_control *lookup(HWND window)
{return (win2k_control *)style_instance_lookup(window,control_proc);}
static BOOL attach(HWND,style_native_theme_fn,HWND);
static BOOL matches(HWND window,const WCHAR *name,const WCHAR *expected)
{
    if(!lstrcmpiW(name,expected))return TRUE;
    if(!lstrcmpiW(expected,TOOLBARCLASSNAMEW))return style_control_protocol_toolbar(window);
    if(!lstrcmpiW(expected,PROGRESS_CLASSW) && style_control_protocol_legacy_progress(window))return TRUE;
    return style_control_class_is(window,expected);
}
static BOOL class_is(HWND window,const WCHAR *expected)
{
    WCHAR name[64];
    return GetClassNameW(window,name,ARRAYSIZE(name)) && matches(window,name,expected);
}
static style_control_state *lookup_state(HWND window)
{
    win2k_control *control=lookup(window);
    return control?&control->state:NULL;
}
static BOOL list_style(DWORD style)
{return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA));}
static BOOL static_class(HWND window)
{return style_control_is_static(window);}
static style_control_basic_kind basic_kind(HWND window)
{
    if(!static_class(window))return STYLE_CONTROL_BUTTON;
    UINT type=(UINT)GetWindowLongPtrW(window,GWL_STYLE)&SS_TYPEMASK;
    if(type==SS_ICON || type==SS_BITMAP)return STYLE_CONTROL_BITMAP;
    if(type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME)return STYLE_CONTROL_SEPARATOR;
    return STYLE_CONTROL_STATIC;
}
BOOL win2k_control_supported(HWND window)
{
    WCHAR name[64];DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!(style&WS_CHILD) || (GetWindowLongPtrW(window,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    if(matches(window,name,L"LISTBOX"))return list_style(style);
    if(matches(window,name,WC_TABCONTROLW))return win2k_tab_supported(window);
    if(matches(window,name,WC_HEADERW))return win2k_header_supported(window);
    if(matches(window,name,WC_LISTVIEWW))return win2k_listview_supported(window);
    if(matches(window,name,WC_TREEVIEWW))return win2k_treeview_supported(window);
    if(matches(window,name,PROGRESS_CLASSW))return win2k_progress_supported(window);
    if(matches(window,name,TRACKBAR_CLASSW))return win2k_trackbar_supported(window);
    if(matches(window,name,UPDOWN_CLASSW))return win2k_updown_supported(window);
    if(!lstrcmpiW(name,DATETIMEPICK_CLASSW))return win2k_date_supported(window);
    if(!lstrcmpiW(name,MONTHCAL_CLASSW))return win2k_month_supported(window);
    if(matches(window,name,TOOLBARCLASSNAMEW) || !lstrcmpiW(name,REBARCLASSNAMEW) || !lstrcmpiW(name,STATUSCLASSNAMEW))
        return TRUE;
    if(matches(window,name,L"SCROLLBAR"))return TRUE;
    if(matches(window,name,L"COMBOBOX"))
        return (style&3) && !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(matches(window,name,L"STATIC")){
        UINT type=style&SS_TYPEMASK;
        return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
            type==SS_ICON || type==SS_BITMAP || (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) ||
            ((type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME) && !(style&SS_SUNKEN));
    }
    win2k_edit_kind edit=win2k_edit_class(window);
    if(edit!=WIN2K_EDIT_NONE){
        DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
        if((style&WS_CAPTION)==WS_CAPTION || (style&WS_THICKFRAME) || (ex&WS_EX_DLGMODALFRAME))return FALSE;
        return edit==WIN2K_EDIT_STANDARD || (style&(WS_BORDER|WS_DLGFRAME)) ||
            (ex&(WS_EX_CLIENTEDGE|WS_EX_WINDOWEDGE|WS_EX_STATICEDGE));
    }
    if(!matches(window,name,L"BUTTON"))return FALSE;
    UINT type=style_control_button_type(window,style);
    if(type==BS_GROUPBOX)return !(style&(BS_ICON|BS_BITMAP|BS_PUSHLIKE|BS_MULTILINE));
    BOOL push=type<=BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE);
    BOOL choice=type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
    return (type<=BS_DEFPUSHBUTTON || choice) && (push || !(style&(BS_ICON|BS_BITMAP)));
}
static void dispose(void *context)
{
    win2k_control *control=context;
    style_control_protocol_release(control->protocol);
    style_control_state_dispose(&control->state);free(control);
}
static void close_date_popup(win2k_control *control)
{
    HWND popup=control->date_popup;
    control->date_popup=NULL;
    if(popup && style_theme_owned(popup,control))
        style_theme_remove(popup,control,control->native_theme);
}
static style_control_retired release_theme(HWND window,void *context,BOOL terminal)
{
    win2k_control *control=context;
    win2k_scroll_retire(window,&control->scroll,TRUE);
    BOOL list_removed=style_control_combo_list_retire(&control->combo,&control->instance,
        lookup_state,win2k_control_remove);
    style_control_notify_unbind(&control->parent_binding,parent_proc,control);
    if(control->date_control)close_date_popup(control);
    if(!terminal && (control->date_control || control->month_control))
        style_calendar_colors_restore(window,&control->calendar,control->date_control);
    if(terminal)(void)style_theme_terminal(window,control);
    else style_theme_remove(window,control,control->native_theme);
    BOOL result=terminal || !control->frame_owned || !IsWindow(window) ||
        SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return (style_control_retired){list_removed && result,FALSE};
}
static void finish_input(HWND window,void *context)
{
    win2k_control *control=context;
    win2k_scroll_retire(window,&control->scroll,FALSE);
}
static BOOL retire(HWND window,win2k_control *control,BOOL terminal)
{
    static const style_control_retire_ops ops={finish_input,release_theme};
    return style_control_retire(window,&control->instance,&control->state,terminal,control,&ops);
}
static win2k_palette list_palette(const win2k_control *control)
{
    win2k_palette palette=win2k_standard_palette;
    if(control->list.present){
        palette.window=control->list.background;
        palette.selection=control->list.selection;
        palette.selection_text=control->list.selection_text;
    }
    if(control->label.present){
        palette.text=control->label.normal;
        palette.gray=control->label.disabled;
    }
    return palette;
}
static win2k_palette frame_palette(const win2k_control *control)
{
    win2k_palette palette=win2k_standard_palette;
    if(control->frame.present){
        palette.face=palette.light=control->frame.face;
        palette.highlight=control->frame.light;
        palette.shadow=control->frame.shadow;
    }
    if(control->list.present)palette.dark=control->list.dark;
    if(control->label.present)palette.text=control->label.normal;
    return palette;
}
static void paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    BOOL painted=TRUE;
    if(control->updown_control){
        win2k_palette palette=frame_palette(control);
        painted=win2k_updown_paint(window,&control->instance,dc,&palette);
    }else if(control->progress_control){
        style_progress_snapshot snapshot;
        painted=style_progress_snapshot_read(window,&control->instance,&control->native,win2k_progress_supported,&snapshot);
        if(painted){
            DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
            const classic_progress_colors *colors=&control->progress_colors[(style&PBS_VERTICAL)!=0];
            win2k_palette palette=win2k_standard_palette;
            if(colors->present){
                palette.face=colors->trough;
                palette.selection=colors->fill;
            }
            painted=win2k_progress_paint(dc,&snapshot,style,&palette);
        }
    }else if(control->tab_control){
        unsigned epoch=control->native.epoch;
        HFONT font=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
        painted=style_control_native_current(window,&control->instance,&control->native,epoch) &&
            win2k_tab_paint(window,dc,font,&control->instance,&control->native,&control->tab_colors);
    }else if(control->combo_parent || class_is(window,L"LISTBOX")){
        HWND parent=control->combo_parent?control->combo_parent:GetParent(window);
        win2k_palette palette=list_palette(control);
        win2k_list_paint(window,&control->instance,parent,dc,&palette);
    }else if(class_is(window,L"COMBOBOX")){
        painted=win2k_combo_paint(window,&control->instance,dc,&control->combo_colors);
    }else if(static_class(window)){
        win2k_palette palette=win2k_standard_palette;
        if(control->label.present){
            palette.text=control->label.normal;
            palette.gray=control->label.disabled;
        }
        if(control->frame.present){
            palette.shadow=control->frame.shadow;
            palette.highlight=control->frame.light;
        }
        painted=win2k_static_paint(window,&control->instance,dc,&palette);
    }else painted=win2k_button_paint(window,&control->instance,dc,&control->push,&control->choice,&control->group);
    if(!painted &&
       !control->instance.detached){
        int saved=SaveDC(dc);
        if(saved){DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
    }
}
static BOOL is_group(HWND window,void *context)
{(void)context;return class_is(window,L"BUTTON") && style_control_button_type(window,(DWORD)GetWindowLongPtrW(window,GWL_STYLE))==BS_GROUPBOX;}
static void render(HWND window,void *context,HDC dc,BOOL buffered)
{
    win2k_control *control=context;
    if(buffered && !control->updown_control){const style_present_ops ops={paint,is_group};
        style_present(window,&control->instance,control,win2k_standard_palette.face,&ops,dc);}
    else paint(window,control,dc);
}
static void status_render(HWND window,void *context,HDC dc,BOOL buffered)
{
    win2k_control *control=context;
    (void)buffered;
    if(!dc)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    unsigned epoch=control->status.epoch;
    DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    RestoreDC(dc,saved);
    if(epoch==control->status.epoch){
        win2k_palette palette=frame_palette(control);
        win2k_status_panes(window,dc,&control->instance,&control->status,&palette);
    }
}
static void edit_border(HWND window,void *context,HDC dc)
{
    const win2k_control *control=context;
    win2k_palette palette=win2k_standard_palette;
    if(control->scroll.controller.resources.list){
        if(control->list.present)palette.dark=control->list.dark;
        if(control->frame.present){
            palette.highlight=control->frame.light;
            palette.shadow=control->frame.shadow;
        }
    }
    win2k_edit_kind kind=win2k_edit_class(window);
    if(kind==WIN2K_EDIT_STANDARD){
        const classic_edit_border *border=&control->edit[classic_edit_state_for(window)];
        if(border->present){
            palette.text=border->line;
            palette.dark=border->dark;
        }
    }else if(kind==WIN2K_EDIT_NATIVE_CLIENT){
        palette=frame_palette(control);
        if(control->frame.present)palette.text=control->frame.edge;
    }
    (void)win2k_edit_border(window,dc,&palette);
}
static BOOL embedded_input(HWND window,void *context,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;
    const style_scroll_ops ops=win2k_scroll_operations(&control->scroll);
    if(message==WM_ENABLE && !a)
        style_scroll_finish(window,&control->instance,&control->scroll.controller,&ops,TRUE);
    return control->instance.detached || style_scroll_embedded_input(window,&control->instance,&control->state,
        &control->scroll.controller,&ops,message,a,b);
}
static void embedded_paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    win2k_scroll_paint_nc(window,&control->scroll,dc);
}
static BOOL control_supported(HWND window,const win2k_control *control)
{
    if(!control->combo_parent)return win2k_control_supported(window);
    win2k_control *parent=lookup(control->combo_parent);
    return parent && !parent->instance.detached &&
        (!control->state.owner || control->state.owner==&parent->instance) &&
        list_style((DWORD)GetWindowLongPtrW(window,GWL_STYLE));
}
static LRESULT dispatch(void *context,HWND window,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;LRESULT result;
    style_control_native_kind kind=control->progress_control?STYLE_CONTROL_PROGRESS:STYLE_CONTROL_TAB;
    if(control->progress_control || control->tab_control)style_control_native_observe(&control->native,kind,message);
    if(control->bar==WIN2K_BAR_STATUS)style_control_status_invalidate(message,NULL,&control->status);
    if(control->date_control || control->month_control)
        style_calendar_colors_observe(&control->calendar,control->date_control,message,a);
    if(message==WM_NCDESTROY){
        (void)retire(window,control,TRUE);result=DefSubclassProc(window,message,a,b);
    }else if(!control_supported(window,control) ||
        (control->parent_binding.parent && GetParent(window)!=control->parent_binding.parent) ||
        (control->date.parent && GetParent(window)!=control->date.parent)){
        result=DefSubclassProc(window,message,a,b);
        if(!control->instance.detached)(void)retire(window,control,FALSE);
    }else if(control->progress_control && (control->legacy_progress || message==WM_NCPAINT || message==WM_PRINT)){
        const style_control_edit_ops ops={.context=control,.border=edit_border};
        result=style_control_edit_message(window,&control->instance,&control->state,NULL,message,a,b,&ops);
    }else if(control->updown_control){
        const style_control_draw draw={control,render};
        result=style_control_native_message(window,&control->instance,&control->native,STYLE_CONTROL_UPDOWN,
            message,a,b,win2k_updown_supported,&draw);
    }else if(control->tab_control || control->progress_control){
        const style_control_draw draw={control,render};
        result=style_control_native_message(window,&control->instance,&control->native,kind,message,a,b,
            control->progress_control?win2k_progress_supported:win2k_tab_supported,&draw);
    }else if(control->bar==WIN2K_BAR_STATUS){
        const style_control_draw draw={control,status_render};
        result=style_control_native_message(window,&control->instance,&control->native,STYLE_CONTROL_STATUS,
            message,a,b,win2k_control_supported,&draw);
    }else if(control->header_control || control->track_control || control->bar || control->date_control || control->month_control){
        result=DefSubclassProc(window,message,a,b);
    }else if(class_is(window,L"SCROLLBAR")){
        if(GetWindowLongPtrW(window,GWL_STYLE)&(SBS_SIZEBOX|SBS_SIZEGRIP)){
            const style_control_draw draw={&control->scroll,win2k_scroll_sizebox};
            result=style_scroll_sizebox_message(window,message,a,b,&draw);
        }else{
            const style_scroll_ops ops=win2k_scroll_operations(&control->scroll);
            result=style_scroll_control_message(window,&control->instance,&control->scroll.controller,&message,&a,&b,&ops);
        }
    }else if(control->scroll.controller.resources.list){
        if(GetWindowLongPtrW(window,GWL_STYLE)&LBS_USETABSTOPS){
            /* Native tab positions can predate attachment and have no getter. */
            const style_control_edit_ops ops={.context=control,.input=embedded_input,.scrollbars=embedded_paint,.border=edit_border};
            result=style_control_edit_message(window,&control->instance,&control->state,NULL,message,a,b,&ops);
        }else{
            const style_control_list_ops ops={.context=control,.input=embedded_input,.draw=render,
                .border=edit_border,.scrollbars=embedded_paint,.native_nonclient=TRUE};
            result=style_control_list_message(window,&control->instance,&control->state,message,a,b,&ops);
        }
    }else if(class_is(window,L"COMBOBOX")){
        const style_control_draw draw={control,render};
        result=style_control_combo_message(window,&control->instance,&control->state,message,a,b,&draw);
    }else if(control->report || win2k_edit_class(window)!=WIN2K_EDIT_NONE){
        const style_control_edit_ops ops={.context=control,.input=embedded_input,.scrollbars=embedded_paint,.border=edit_border};
        result=style_control_edit_message(window,&control->instance,&control->state,NULL,message,a,b,&ops);
    }else{
        const style_control_draw draw={control,render};
        result=style_control_basic_message(window,&control->instance,&control->state,message,a,b,basic_kind(window),&draw);
    }
    return result;
}
static void finish_call(void *context,BOOL completed)
{win2k_control *control=context;(void)completed;style_instance_release(&control->instance);}

typedef struct win2k_date_call {
    win2k_control *control;
    style_control_native_overlay overlay;
    HDC dc;
    int saved;
} win2k_date_call;

static void date_button(HWND window,const style_instance *instance,HDC dc,const RECT *dirty,
    const style_control_native_button *button)
{
    win2k_palette palette=frame_palette(instance->context);
    win2k_date_button(window,instance,dc,dirty,button,&palette);
}

static LRESULT date_dispatch(void *context,HWND window,UINT message,WPARAM a,LPARAM b)
{
    win2k_date_call *call=context;
    win2k_control *control=call->control;
    const style_control_native_combo_state no_color_combo={0};
    style_control_native_overlay_begin(window,message,a,&control->instance,&no_color_combo,&control->date,&call->overlay);
    if(message==WM_PRINTCLIENT && a){
        call->dc=(HDC)a;
        call->saved=SaveDC(call->dc);
        if(call->saved)SetMetaRgn(call->dc);
    }
    LRESULT result=dispatch(control,window,message,a,b);
    DWORD error=GetLastError();
    if(call->saved){
        RestoreDC(call->dc,call->saved);
        call->saved=0;
    }
    style_control_native_overlay_finish(window,message,a,&control->instance,&no_color_combo,
        &control->date,&call->overlay,date_button);
    SetLastError(error);
    return result;
}

static void date_finish(void *context,BOOL completed)
{
    win2k_date_call *call=context;
    if(!completed){
        if(call->saved)RestoreDC(call->dc,call->saved);
        style_control_native_overlay_dispose(&call->overlay);
    }
    finish_call(call->control,completed);
}

static void date_notify(win2k_control *control,HWND parent,const NMHDR *notice)
{
    HWND picker=control->instance.window;
    if(notice->hwndFrom!=picker || control->instance.detached || GetParent(picker)!=parent)return;
    if(notice->code==DTN_CLOSEUP)close_date_popup(control);
    if(notice->code!=DTN_DROPDOWN)return;
    HWND popup=(HWND)SendMessageW(picker,DTM_GETMONTHCAL,0,0);
    if(control->instance.detached || !win32_style_window_current_thread(popup) ||
        !class_is(popup,MONTHCAL_CLASSW) || style_theme_retained(popup) || !style_theme_capture(popup))return;
    control->date_popup=popup;
    if(!style_theme_suppress(popup,control,control->native_theme)){
        control->date_popup=NULL;
        (void)style_theme_terminal(popup,control);
    }
}

static LRESULT parent_dispatch(void *context,HWND parent,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;
    if(control->bar || control->header_control || control->track_control || control->report){
        win2k_bar_draw bar={.instance=&control->instance};
        if(control->bar)bar.palette=frame_palette(control);
        style_control_notify_ops toolbar=win2k_toolbar_notify;
        toolbar.context=&bar;
        const style_control_notify_ops rebar={.context=&bar,.supported=win2k_rebar_supported,.finish_item=win2k_rebar_grip};
        const win2k_header_draw header_draw={&control->instance,&control->header_colors};
        const style_control_notify_ops header={.context=(void *)&header_draw,.supported=win2k_header_supported,.part=win2k_header_paint};
        const style_control_notify_ops track={.context=control->trackbar_colors,
            .supported=win2k_trackbar_supported,.part=win2k_trackbar_part};
        win2k_report_draw report={.instance=&control->instance};
        if(control->report){
            report.palette=list_palette(control);
            if(control->frame.present)report.palette.face=control->frame.face;
        }
        const style_control_notify_ops list={.context=&report,.supported=win2k_listview_plain,.prepare_item=win2k_listview_colors};
        const style_control_notify_ops tree={.context=&report,.supported=win2k_treeview_supported,.prepare_item=win2k_treeview_colors};
        const style_control_notify_ops *ops=control->bar==WIN2K_BAR_TOOL?&toolbar:control->bar==WIN2K_BAR_REBAR?&rebar:
            control->header_control?&header:control->track_control?&track:
            control->report==WIN2K_REPORT_LIST?&list:&tree;
        return style_control_notify_message(parent,message,a,b,(UINT_PTR)control->parent_binding.window,
            &control->instance,&control->parent_binding,parent_proc,NULL,ops);
    }
    if(message==WM_NCDESTROY)style_control_notify_unbind(&control->parent_binding,parent_proc,control);
    if(control->date_control && message==WM_NOTIFY && b){
        DWORD entry=GetLastError();
        date_notify(control,parent,(const NMHDR *)b);
        SetLastError(entry);
    }
    LRESULT result=DefSubclassProc(parent,message,a,b);DWORD error=GetLastError();
    if(!control->instance.detached && (message==WM_CTLCOLOREDIT || message==WM_CTLCOLORSTATIC) &&
       (HWND)b==control->parent_binding.window && GetParent((HWND)b)==parent)
        result=(LRESULT)win2k_edit_brush((HDC)a,(HBRUSH)result,&win2k_standard_palette);
    SetLastError(error);return result;
}
static LRESULT CALLBACK parent_proc(HWND parent,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    win2k_control *control=(win2k_control *)data;(void)id;
    style_instance_retain(&control->instance);
    const style_dispatch_ops ops={control,control,parent_dispatch,finish_call};
    return style_dispatch_message(parent,message,a,b,&ops);
}
static LRESULT CALLBACK control_proc(HWND window,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    win2k_control *control=(win2k_control *)data;(void)id;
    style_instance_retain(&control->instance);
    if(control->date_control && (message==WM_PAINT || message==WM_PRINTCLIENT)){
        win2k_date_call call={.control=control};
        const style_dispatch_ops ops={&call,&call,date_dispatch,date_finish};
        return style_dispatch_message(window,message,a,b,&ops);
    }
    const style_dispatch_ops ops={control,control,dispatch,finish_call};
    return style_dispatch_message(window,message,a,b,&ops);
}
static BOOL install_combo_list(HWND window,void *context)
{
    win2k_control *parent=context;
    if(!class_is(window,L"ComboLBox") || !list_style((DWORD)GetWindowLongPtrW(window,GWL_STYLE))){
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    return attach(window,parent->native_theme,parent->instance.window);
}
static BOOL setup(HWND window,void *context)
{
    win2k_control *control=context;
    if((control->date_control || control->bar==WIN2K_BAR_TOOL || control->bar==WIN2K_BAR_REBAR || control->header_control || control->track_control || control->report ||
        win2k_edit_class(window)==WIN2K_EDIT_STANDARD) &&
       !style_control_notify_bind(window,&control->parent_binding,parent_proc,control))return FALSE;
    if(!style_theme_capture(window) || !style_theme_suppress(window,control,control->native_theme))return FALSE;
    if(control->date_control){
        control->date=(style_control_native_combo_state){.candidate=TRUE,.parent=GetParent(window),
            .owner=(HMODULE)GetClassLongPtrW(window,GCLP_HMODULE),.native_legacy_date=TRUE};
        style_control_native_combo_settle(window,WM_PAINT,&control->instance,&control->date,TRUE);
    }
    if(control->date_control || control->month_control){
        win2k_palette palette=list_palette(control);
        win2k_date_colors(window,&control->instance,&control->calendar,control->date_control,&palette);
    }
    if(class_is(window,L"COMBOBOX")){
        const style_control_combo_ops bindings={.lookup=lookup_state};
        const style_control_combo_setup_ops children={
            .context=control,.get_info=win2k_combo_info,.install_list=install_combo_list,.remove=win2k_control_remove
        };
        return style_control_combo_setup(window,&control->instance,&control->combo,&bindings,&children);
    }
    return !control->frame_owned || SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
}
static void rollback(HWND window,void *context)
{(void)retire(window,context,FALSE);}
static BOOL attach(HWND window,style_native_theme_fn native_theme,HWND combo_parent)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(lookup(window))return TRUE;
    if(!native_theme){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    if(!style_instance_preflight(window,control_proc))return FALSE;
    style_control_protocol *protocol=NULL;
    DWORD error;
    if(!combo_parent && !win2k_control_supported(window)){
        SetLastError(ERROR_SUCCESS);
        protocol=style_control_protocol_bind(window);
        error=GetLastError();
        if(!protocol || !win2k_control_supported(window)){
            if(protocol || (error!=ERROR_NOT_ENOUGH_MEMORY && error!=ERROR_ARITHMETIC_OVERFLOW))
                error=ERROR_NOT_SUPPORTED;
            goto rejected;
        }
    }
    win2k_report_role report=class_is(window,WC_LISTVIEWW)?WIN2K_REPORT_LIST:
        class_is(window,WC_TREEVIEWW)?WIN2K_REPORT_TREE:WIN2K_REPORT_NONE;
    if(report==WIN2K_REPORT_LIST && !win2k_listview_plain(window)){error=ERROR_NOT_SUPPORTED;goto rejected;}
    BOOL progress=class_is(window,PROGRESS_CLASSW);
    BOOL legacy=progress && !SendMessageW(window,PBM_GETSTATE,0,0);
    win2k_control *control=calloc(1,sizeof(*control));
    if(!control){error=ERROR_NOT_ENOUGH_MEMORY;goto rejected;}
    style_instance_init(&control->instance,control,dispose);
    control->protocol=protocol;
    control->tab_control=class_is(window,WC_TABCONTROLW);
    control->header_control=class_is(window,WC_HEADERW);
    control->track_control=class_is(window,TRACKBAR_CLASSW);
    control->updown_control=class_is(window,UPDOWN_CLASSW);
    control->date_control=class_is(window,DATETIMEPICK_CLASSW);
    control->month_control=class_is(window,MONTHCAL_CLASSW);
    control->bar=class_is(window,TOOLBARCLASSNAMEW)?WIN2K_BAR_TOOL:class_is(window,REBARCLASSNAMEW)?WIN2K_BAR_REBAR:
        class_is(window,STATUSCLASSNAMEW)?WIN2K_BAR_STATUS:WIN2K_BAR_NONE;
    control->progress_control=progress;control->legacy_progress=legacy;
    control->report=report;
    style_control_native_init(&control->native);
    win2k_scroll_init(&control->scroll,&control->instance);
    for(unsigned i=0;i<CLASSIC_PUSH_STATE_COUNT;++i)
        control->push.state[i]=(classic_surface_colors){win2k_standard_palette.face,win2k_standard_palette.dark};
    classic_theme theme;
    if(classic_theme_default((HMODULE)&__ImageBase,&theme)){
        control->push=theme.push;
        control->choice=theme.choice;
        control->label=theme.label;
        memcpy(control->edit,theme.edit,sizeof(control->edit));
        control->list=theme.list;
        control->combo_colors=theme.combo;
        control->tab_colors=theme.tab;
        control->header_colors=theme.header;
        memcpy(control->progress_colors,theme.progress,sizeof(control->progress_colors));
        memcpy(control->trackbar_colors,theme.trackbar,sizeof(control->trackbar_colors));
        control->group=theme.group;
        control->frame=theme.frame;
        win2k_scroll_theme(&control->scroll,&theme);
    }
    control->scroll.controller.resources.list=combo_parent || class_is(window,L"LISTBOX");
    control->scroll.controller.resources.edit=win2k_edit_class(window)==WIN2K_EDIT_STANDARD &&
        (GetWindowLongPtrW(window,GWL_STYLE)&ES_MULTILINE);
    control->scroll.controller.resources.frame=report || win2k_edit_class(window)==WIN2K_EDIT_NATIVE_CLIENT;
    style_control_state_init(&control->state,NULL);control->native_theme=native_theme;
    control->combo_parent=combo_parent;
    control->frame_owned=progress || basic_kind(window)==STYLE_CONTROL_SEPARATOR;
    const style_instance_install_ops ops={control,NULL,setup,rollback};
    BOOL result=style_instance_install(&control->instance,window,control_proc,control,&ops);
    if(result)RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return result;
rejected:
    style_control_protocol_release(protocol);
    SetLastError(error);
    return FALSE;
}
BOOL win2k_control_attach(HWND window,style_native_theme_fn native_theme)
{return attach(window,native_theme,NULL);}
BOOL win2k_control_attached(HWND window)
{win2k_control *control=lookup(window);return control && !control->instance.detached;}
BOOL win2k_control_remove(HWND window)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    win2k_control *control=lookup(window);
    if(!control){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    return retire(window,control,FALSE);
}
