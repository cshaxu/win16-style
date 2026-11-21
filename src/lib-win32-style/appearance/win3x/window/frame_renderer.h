#ifndef WIN16_CONTROL_FRAME_RENDERER_H
#define WIN16_CONTROL_FRAME_RENDERER_H
#include "../common/palette.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "frame_paint.h"
#include "lib-win32-style/application/lifecycle/instance.h"
/* Synchronous rendering operations on the caller's complete instance and
 * renderer context. state returns a whole presentation snapshot; reread it
 * after application callbacks rather than retaining pointers into fields. */
typedef struct win16_frame_renderer_ops {
    BOOL (*layout)(HWND,void *,style_full_client_layout *);
    win16_frame_state_read state;
    void (*colors)(void *,win16_style_palette *);
    void (*scrollbars)(HWND,HDC,const win16_style_palette *);
} win16_frame_renderer_ops;
void win16_control_paint_frame(HWND,style_instance *,void *,const win16_frame_renderer_ops *,HDC);
void win16_control_frame_buffered(HWND,style_instance *,void *,const win16_frame_renderer_ops *,HDC);
void win16_control_frame_menu_state(HWND,style_instance *,void *,const win16_frame_renderer_ops *);
void win16_control_frame_button_state(HWND,style_instance *,void *,const win16_frame_renderer_ops *,int);

typedef struct win16_control_frame_button_view {
    void *context;
    style_caption_button *state;
    style_instance *instance;
    BOOL (*layout)(HWND,void *,style_full_client_layout *);
    void (*paint)(HWND,void *,int);
} win16_control_frame_button_view;
void win16_control_frame_button_press(HWND window,const win16_control_frame_button_view *view,int hit);
BOOL win16_control_frame_button_motion(HWND window,const win16_control_frame_button_view *view,UINT message,LPARAM point);
BOOL win16_control_frame_button_cancel(HWND window,const win16_control_frame_button_view *view,UINT message);
#endif
