#ifndef STYLE_MESSAGE_OBSERVER_H
#define STYLE_MESSAGE_OBSERVER_H
#include <windows.h>

/* Each successful begin owns a distinct current-thread hook, including nested
 * begins. The caller must end it on normal and exceptional exits; a failed
 * begin returns NULL and end accepts NULL. This observes messages only: the
 * caller owns attachment, rollback, the message loop and error preservation. */
static inline HHOOK style_message_observer_begin(HOOKPROC callback)
{return SetWindowsHookExW(WH_CALLWNDPROCRET,callback,NULL,GetCurrentThreadId());}
static inline void style_message_observer_end(HHOOK hook)
{if(hook)UnhookWindowsHookEx(hook);}
static inline const CWPRETSTRUCT *style_message_dialog_initialized(int code,LPARAM parameter)
{
    if(code<0 || !parameter)return NULL;
    const CWPRETSTRUCT *message=(const CWPRETSTRUCT *)parameter;
    return message->message==WM_INITDIALOG?message:NULL;
}
#endif
