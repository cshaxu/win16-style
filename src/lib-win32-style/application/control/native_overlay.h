#ifndef STYLE_CONTROL_NATIVE_OVERLAY_H
#define STYLE_CONTROL_NATIVE_OVERLAY_H
#include "control_native_combo.h"
#include "../graphics/clip.h"
/* One native message owns the captured region. Print DC is borrowed; paint
 * DC is acquired/released in finish. Caller pins the instance throughout. */
typedef struct style_control_native_overlay {
    RECT dirty;
    BOOL update;
    style_clip_snapshot clip;
} style_control_native_overlay;
void style_control_native_overlay_begin(HWND,UINT,WPARAM,const style_instance *,const style_control_native_combo_state *,const style_control_native_combo_state *,style_control_native_overlay *);
void style_control_native_overlay_finish(HWND,UINT,WPARAM,const style_instance *,const style_control_native_combo_state *,const style_control_native_combo_state *,style_control_native_overlay *,style_control_native_button_painter);
/* On an escaping native exception the caller's existing unwind path disposes
 * the same object; there is no second region or separate ownership path. */
static inline void style_control_native_overlay_dispose(style_control_native_overlay *overlay)
{style_clip_snapshot_dispose(&overlay->clip);}
#endif
