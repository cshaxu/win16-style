#ifndef STYLE_FONT_SCOPE_H
#define STYLE_FONT_SCOPE_H
#include <windows.h>

/* Synchronous borrowed DC plus optional owned font lease. A saved DC or
 * previous selected object is restored before returning the lease. The
 * provider/context outlive this call; end exactly once, including existing
 * native unwind. No exception is caught or new recovery installed here. */
typedef struct style_font_scope {
    HDC dc;
    HFONT font;
    HGDIOBJ previous;
    int saved;
    void *context;
    void (*release)(void *,HFONT);
} style_font_scope;
static inline void style_font_scope_select(style_font_scope *scope,HFONT font)
{
    if(font){HGDIOBJ old=SelectObject(scope->dc,font);
        scope->previous=old==HGDI_ERROR?NULL:old;}
}
static inline void style_font_scope_end(void *context)
{
    style_font_scope *scope=context;
    if(scope->saved)RestoreDC(scope->dc,scope->saved);
    else if(scope->previous)SelectObject(scope->dc,scope->previous);
    if(scope->release)scope->release(scope->context,scope->font);
}
/* One GUI-thread gate per interception family. Begin always enters, even
 * when nested; only its outermost caller evaluates rendering policy.
 * Restore the font scope before leaving the gate; temporary text buffers
 * may be freed between those operations. Normal text leaves pair end on
 * normal return, preserving their original
 * exception boundary rather than inventing a new handler. */
typedef struct style_font_gate {unsigned depth;} style_font_gate;
static inline BOOL style_font_gate_begin(style_font_gate *gate,style_font_scope *scope,HDC dc)
{*scope=(style_font_scope){.dc=dc};return !gate->depth++;}
static inline void style_font_gate_end(style_font_gate *gate)
{--gate->depth;}
#endif
