#ifndef STYLE_UWP_FRAME_H
#define STYLE_UWP_FRAME_H
#include "host.h"
#include "../graphics/bitmap.h"
#include "../menu/owner.h"

typedef struct style_uwp_frame_action {
    PCWSTR label;
    void (*invoke)(HWND,void *);
    void *context;
} style_uwp_frame_action;
typedef struct style_uwp_frame_action_entry {
    style_uwp_frame_action value;
    UINT command;
} style_uwp_frame_action_entry;

typedef struct style_uwp_frame_ops {
    BOOL (*geometry)(void *,RECT *outer,RECT *hole,POINT *menu_anchor);
    void (*paint)(void *,HDC);
    LRESULT (*hit)(void *,POINT);
    BOOL (*button)(void *,int);
    void (*invoke)(void *,int,UINT);
    BOOL (*menu_message)(void *,HWND,UINT,WPARAM,LPARAM,BOOL owner,LRESULT *);
    void (*dispose)(void *);
} style_uwp_frame_ops;

/* One GUI-thread-owned object, embedded in a renderer allocation. The host
 * pins it through synchronous callbacks, menu tracking and detach. The root
 * owns common state/resources; the renderer owns layout/fonts and releases
 * the enclosing allocation in dispose, after common resources are released. */
typedef struct style_uwp_frame {
    style_uwp_host host;
    style_bitmap bitmap;
    style_menu_owner menu;
    BOOL updating,active,hot,tracking,no_menu;
    int pressed;
    HWND menu_window;
    POINT menu_anchor;
    unsigned action_count;
    style_uwp_frame_action_entry *actions;
    const style_uwp_frame_ops *ops;
    void *context;
} style_uwp_frame;

/* Validate before allocating. NULL title selects an existing non-client
 * frame; its allocation/restoration remains the caller's responsibility.
 * At most 256 copied actions; labels/contexts and ops remain borrowed until
 * removal and the last callback return. No process hook is installed here. */
BOOL style_uwp_frame_validate(HWND,HWND,const style_uwp_frame_action *,unsigned);
/* Consumes a zero-initialized object on success or failure. storage is an
 * array of count entries in the enclosing allocation, not a second owner.
 * Call validate first; all ops are required and outlive the object. */
BOOL style_uwp_frame_start(style_uwp_frame *,HWND,HWND,const style_uwp_frame_ops *,void *,
    const style_uwp_frame_action *,unsigned,style_uwp_frame_action_entry *storage);
BOOL style_uwp_frame_remove(HWND);
HWND style_uwp_frame_surface(HWND);
BOOL style_uwp_frame_uses(HWND,const style_uwp_frame_ops *);
/* Menu renderer selection notification; context is the complete frame. */
void style_uwp_frame_menu_selection(HWND,void *,BOOL);
#endif
