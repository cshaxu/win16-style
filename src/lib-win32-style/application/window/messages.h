#ifndef STYLE_WINDOW_MESSAGES_H
#define STYLE_WINDOW_MESSAGES_H
#include "native_frame.h"
#include "lib-win32-style/application/lifecycle/instance.h"

typedef struct style_frame_message_ops {
    void *context;
    BOOL (*client)(HWND,void *,const RECT *,RECT *);
    void (*activate)(HWND,void *,const RECT *,BOOL);
    void (*paint)(HWND,void *,HDC);
    void (*buffered)(HWND,void *,HDC);
    BOOL (*hit_test)(HWND,void *,LPARAM,LRESULT *);
    BOOL (*refresh_font)(HWND,void *);
} style_frame_message_ops;

/* The caller retains the instance across this synchronous operation and any
 * native callback. It chooses applicable messages; FALSE delegates to native.
 * Client measurement replaces the entire client rectangle before USER layout;
 * post-native adjustments can use style_frame_set_client directly instead. */
BOOL style_frame_message(style_instance *,style_native_frame *,UINT,WPARAM,LPARAM,
    const style_frame_message_ops *,LRESULT *);
#endif
