#ifndef STYLE_CONTROL_UPDOWN_H
#define STYLE_CONTROL_UPDOWN_H
#include <windows.h>
#include <uxtheme.h>
/* One linked module owns a process-wide API-ready flag and a GUI-thread local
 * registry. Caller pins each registered object, its native module and ops.
 * Registry borrows windows/DCs; it neither deletes GDI handles nor installs
 * hooks. Retire removes native associations, not a DLL or style attachment. */
typedef struct style_control_updown_ops {
    void *context;
    BOOL (*supported)(DWORD,DWORD);
    BOOL (*render)(HWND,void *,HDC,const RECT *,const RECT *,unsigned,BOOL,BOOL);
} style_control_updown_ops;
typedef struct style_control_updown {
    struct style_control_updown *next;HWND window;HMODULE native;
    void *owner;BOOL (*acquire)(void *);void (*release)(void *);
    style_control_updown_ops ops;
} style_control_updown;
void style_control_updown_api_enable(void);
BOOL style_control_updown_api_enabled(void);
void style_control_updown_register(style_control_updown *,HWND,HMODULE,void *,BOOL (*)(void *),void (*)(void *),const style_control_updown_ops *);
void style_control_updown_retire(style_control_updown *);
/* Resolver supplies synchronous native-paint metadata. FALSE with a non-NULL
 * HWND still blocks drawing on another registered window's clone; metadata
 * association is allowed beneath that barrier, matching native nested paint.
 * The existing32-slot bound overflows to native. Calls acquire/release owner
 * on normal renderer return; no additional exception recovery is invented. */
void style_control_updown_dc_created(HDC,HDC,const void *,BOOL (*)(HDC,HWND *));
void style_control_updown_dc_deleted(HDC);
BOOL style_control_updown_draw_frame(HDC,RECT *,UINT,UINT,const void *,BOOL (*)(HDC,HWND *));
BOOL style_control_updown_draw_theme(HTHEME,HDC,int,int,const RECT *,const RECT *,const void *,BOOL (*)(HDC,HWND *));
#endif
