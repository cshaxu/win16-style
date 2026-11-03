#ifndef WIN16_CONTROL_SPIN_SCOPE_H
#define WIN16_CONTROL_SPIN_SCOPE_H
#include <windows.h>
typedef struct spin_buffers { DWORD background[4096],original[4096];BYTE valid[4096]; } spin_buffers;
typedef struct spin_draw_scope {
    const BOOL *modern_spin,*detached; BOOL *verified; HWND window,parent; HDC dc,clones[16],snapshot_dc; HGDIOBJ snapshot_bitmap;
    spin_buffers *buffers; RECT halves[2]; int states[2]; struct spin_draw_scope *previous_stack;
} spin_draw_scope;
void win16_control_spin_buffers_clear(spin_draw_scope *);
void win16_control_spin_dc_created(spin_draw_scope *,HDC,HDC);
void win16_control_spin_theme_begin(spin_draw_scope *,HDC,const RECT *);
void win16_control_spin_dc_deleted(spin_draw_scope *,HDC);
void win16_control_spin_selected(spin_draw_scope *,HDC);
void win16_control_spin_object_deleted(spin_draw_scope *,HGDIOBJ);
void win16_control_spin_theme(spin_draw_scope *,HDC,int,int,const RECT *,const RECT *,const void *);
void win16_control_spin_blit(spin_draw_scope *,HDC,int,int,int,int,HDC,int,int,DWORD);
void win16_control_spin_frame(spin_draw_scope *,HDC,const RECT *,UINT,UINT,const void *);
#endif
