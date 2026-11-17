#ifndef STYLE_CONTROL_SPIN_H
#define STYLE_CONTROL_SPIN_H
#include "../lifecycle/instance.h"
typedef struct style_control_spin_state {BOOL candidate,modern,verified;} style_control_spin_state;
/* Existing native protocol bounds:64x64 samples and16 borrowed clone DCs.
 * No DC/bitmap reference is owned; deleting/selecting invalidates its sample.
 * Scope owns only buffers; caller pins whole instance/state and restores its
 * GUI-thread scope chain on both ordinary and exceptional returns. */
typedef struct style_control_spin_buffers {DWORD background[4096],original[4096];BYTE valid[4096];} style_control_spin_buffers;
typedef struct style_control_spin_scope {
    style_instance *instance;style_control_spin_state *state;
    HWND window,parent;HDC dc,clones[16],snapshot_dc;HGDIOBJ snapshot_bitmap;
    style_control_spin_buffers *buffers;RECT halves[2];int states[2];struct style_control_spin_scope *previous_stack;
} style_control_spin_scope;
BOOL style_control_spin_message(const style_instance *,const style_control_spin_state *,UINT);
void style_control_spin_buffers_clear(style_control_spin_scope *);
void style_control_spin_dc_created(style_control_spin_scope *,HDC,HDC);
void style_control_spin_theme_begin(style_control_spin_scope *,HDC,const RECT *);
void style_control_spin_dc_deleted(style_control_spin_scope *,HDC);
void style_control_spin_selected(style_control_spin_scope *,HDC);
void style_control_spin_object_deleted(style_control_spin_scope *,HGDIOBJ);
void style_control_spin_theme(style_control_spin_scope *,HDC,int,int,const RECT *,const RECT *,const void *);
/* Complete synchronous renderer operations; borrowedDC/rects must not escape.
 * Composition replaces the sampled face but preserves application glyphs. */
void style_control_spin_blit(style_control_spin_scope *,HDC,int,int,int,int,HDC,int,int,DWORD,BOOL (*)(HWND,HDC,const RECT *,const RECT *,UINT));
void style_control_spin_frame(style_control_spin_scope *,HDC,const RECT *,UINT,UINT,const void *,BOOL (*)(HDC,const RECT *,const RECT *));
#endif
