#include "control_state.h"
#include "control_vcl_spin.h"
#include "../graphics/text_metrics.h"
#include <stdlib.h>
#include <limits.h>

void style_control_state_init(style_control_state *state,const style_control_fonts *fonts)
{*state=(style_control_state){.fonts=fonts};}
BOOL style_control_state_prepare(HWND window,style_control_state *state,int count,const int *tabs)
{
    if(count<0 || (count && !tabs) || (size_t)count>((size_t)-1)/sizeof(int)){
        SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    SetLastError(ERROR_SUCCESS);state->protocol=style_control_protocol_acquire(window);
    if(!state->protocol && GetLastError()==ERROR_ARITHMETIC_OVERFLOW)return FALSE;
    if(count){
        state->tab_positions=malloc((size_t)count*sizeof(int));if(!state->tab_positions)return FALSE;
        CopyMemory(state->tab_positions,tabs,(size_t)count*sizeof(int));state->tab_count=count;
    }
    return TRUE;
}
BOOL style_control_font_read(HWND w,style_control_state *state,SUBCLASSPROC proc,const BOOL *cancelled)
{
    DWORD_PTR existing;state->original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(cancelled && *cancelled){SetLastError(ERROR_CANCELLED);return FALSE;}
    if(GetWindowSubclass(w,proc,(UINT_PTR)proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    state->original_font=state->fonts->source(state->original_font);return TRUE;
}
void style_control_state_dispose(style_control_state *state)
{
    if(state->fonts)state->fonts->release(state->font);
    style_control_protocol_release(state->protocol);
    free(state->tab_positions);*state=(style_control_state){0};
}
BOOL style_control_font_message(HWND window,style_instance *instance,style_control_state *state,WPARAM *parameter,
    style_control_font_mode mode,BOOL native,BOOL preserve,HFONT *previous,HFONT *outbound)
{
    if(mode==STYLE_CONTROL_FONT_OBSERVE){
        HFONT source=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
        if(!instance->detached && source!=state->original_font){
            HFONT replacement=state->fonts->acquire(window,source,native,state->font_recipe);
            *previous=state->font;state->original_font=source;state->font=replacement?replacement:source;
        }
    }
    if(instance->detached)return FALSE;
    if(mode==STYLE_CONTROL_FONT_NORMALIZE || mode==STYLE_CONTROL_FONT_CACHE || mode==STYLE_CONTROL_FONT_REPLACE){
        HFONT source=state->fonts->source((HFONT)*parameter);state->original_font=source;
        *parameter=(WPARAM)source;
        if(mode!=STYLE_CONTROL_FONT_NORMALIZE){
            HFONT replacement=state->fonts->acquire(window,source,mode==STYLE_CONTROL_FONT_CACHE,state->font_recipe);
            *previous=state->font;state->font=replacement?replacement:source;
            if(mode==STYLE_CONTROL_FONT_REPLACE){
                *outbound=state->font;state->fonts->retain(*outbound);
                *parameter=(WPARAM)(preserve?source:state->font);
            }
        }
    }
    return TRUE;
}
void style_control_font_finish(HWND window,const style_instance *instance,style_control_state *state,UINT message,LPARAM parameter,BOOL painting,
    HFONT source,HFONT previous,HFONT outbound,HFONT pinned)
{
    if(message==WM_SETFONT && instance->detached && IsWindow(window))SendMessageW(window,WM_SETFONT,(WPARAM)source,parameter);
    if(painting)--state->native_paints;
    state->fonts->release(previous);state->fonts->release(outbound);state->fonts->release(pinned);
}
void style_control_message_finish(HWND w,style_instance *instance,style_control_state *state,UINT m,LPARAM b,HWND spin_button,const RECT *spin_before,BOOL combo_paint,HFONT previous,HFONT outbound,HFONT pinned,style_control_state *(*lookup)(HWND))
{
    style_control_vcl_spin_after(w,spin_button,spin_before,&instance->detached);
    HFONT source=state->original_font;
    if(m==WM_SETFONT && instance->detached && IsWindow(w)){
        style_control_state *successor=lookup(w);if(successor)source=successor->original_font;
    }
    style_control_font_finish(w,instance,state,m,b,combo_paint,source,previous,outbound,pinned);
    style_instance_release(instance);
}
LRESULT style_control_tabs_set(HWND window,style_instance *instance,style_control_state *state,WPARAM count,LPARAM values,
    LRESULT (*forward)(HWND,WPARAM,LPARAM))
{
    int *pixels=NULL,average;LRESULT result;
    if(count>INT_MAX || (count && !values) || count>((size_t)-1)/sizeof(int))return FALSE;
    if(count){
        pixels=malloc((size_t)count*sizeof(int));if(!pixels)return FALSE;
        HDC dc=GetDC(window);if(!dc){free(pixels);return FALSE;}
        HGDIOBJ old=SelectObject(dc,state->font);TEXTMETRICW metrics;
        if(!GetTextMetricsW(dc,&metrics)){SelectObject(dc,old);ReleaseDC(window,dc);free(pixels);return FALSE;}
        average=metrics.tmAveCharWidth;
        if((metrics.tmPitchAndFamily&TMPF_FIXED_PITCH) && !style_text_alphabet_average(dc,&average)){
            SelectObject(dc,old);ReleaseDC(window,dc);free(pixels);return FALSE;
        }
        SelectObject(dc,old);ReleaseDC(window,dc);
        for(int i=0;i<(int)count;++i){
            LONGLONG value=((LONGLONG)((const int *)values)[i]*average+2)/4;
            if(value<INT_MIN || value>INT_MAX){free(pixels);return FALSE;}pixels[i]=(int)value;
        }
    }
    result=forward(window,count,values);
    if(result && !instance->detached){free(state->tab_positions);state->tab_positions=pixels;state->tab_count=(int)count;}
    else free(pixels);
    return result;
}
