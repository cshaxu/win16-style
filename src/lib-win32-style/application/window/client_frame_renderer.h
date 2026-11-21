#ifndef STYLE_CLIENT_FRAME_RENDERER_H
#define STYLE_CLIENT_FRAME_RENDERER_H
#include "palette.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "client_frame_paint.h"
#include "lib-win32-style/application/lifecycle/instance.h"
/* Synchronous rendering operations on the caller's complete instance and
 * renderer context. state returns a whole presentation snapshot; reread it
 * after application callbacks rather than retaining pointers into fields. */
typedef struct style_client_frame_renderer_ops {
    BOOL (*layout)(HWND,void *,style_full_client_layout *);
    style_client_frame_state_read state;
    void (*colors)(void *,style_window_palette *);
    void (*scrollbars)(HWND,HDC,const style_window_palette *);
} style_client_frame_renderer_ops;
void style_client_frame_paint(HWND,style_instance *,void *,const style_client_frame_renderer_ops *,HDC);
void style_client_frame_buffered(HWND,style_instance *,void *,const style_client_frame_renderer_ops *,HDC);
void style_client_frame_menu_state(HWND,style_instance *,void *,const style_client_frame_renderer_ops *);
void style_client_frame_button_state(HWND,style_instance *,void *,const style_client_frame_renderer_ops *,int);

typedef struct style_client_frame_button_view {
    void *context;
    style_caption_button *state;
    style_instance *instance;
    BOOL (*layout)(HWND,void *,style_full_client_layout *);
    void (*paint)(HWND,void *,int);
} style_client_frame_button_view;
void style_client_frame_button_press(HWND window,const style_client_frame_button_view *view,int hit);
BOOL style_client_frame_button_motion(HWND window,const style_client_frame_button_view *view,UINT message,LPARAM point);
BOOL style_client_frame_button_cancel(HWND window,const style_client_frame_button_view *view,UINT message);
#endif
