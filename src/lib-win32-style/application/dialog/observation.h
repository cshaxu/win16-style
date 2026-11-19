#ifndef STYLE_DIALOG_OBSERVATION_H
#define STYLE_DIALOG_OBSERVATION_H
#include "../lifecycle/creation_call.h"
/* Strictly nested on the same GUI thread. Observer code outlives its hook.
 * NULL creation context selects observation only, without creation hooks.
 * A tracked call temporarily masks the outer target, even when inactive;
 * untracked calls leave that identity unchanged. Hook failure is advisory,
 * as in the existing common-dialog contract. End releases observation and
 * creation resources, not styles already applied to surviving windows. */
typedef struct style_dialog_observation {
    style_creation_call creation;
    HHOOK hook;
    HWND window;
    struct style_dialog_observation *previous;
    BOOL tracked;
} style_dialog_observation;
BOOL style_dialog_observation_begin(style_dialog_observation *,style_creation_context *,HOOKPROC,BOOL tracked);
void style_dialog_observation_end(void *);
void style_dialog_observation_found(HWND);
HWND style_dialog_observation_current(void);
HRESULT style_dialog_observe(HRESULT (*call)(void *),void *context,void (*initialized)(HWND));
#endif
