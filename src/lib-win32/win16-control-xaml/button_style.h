#ifndef WIN16_XAML_BUTTON_STYLE_H
#define WIN16_XAML_BUTTON_STYLE_H

#include <windows.h>
#include <inspectable.h>
#include <xamlom.h>

/* All operations run on the owning XAML thread. Services are borrowed. */
typedef struct win16_xaml_button_style {
    IInspectable *button;
    IInspectable *installed_style;
    unsigned int property_index;
    DWORD thread_id;
} win16_xaml_button_style;

HRESULT win16_xaml_button_source(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                 IInspectable *button, unsigned int *source, unsigned int *index);
/* Only a Button whose original Style source is Default is accepted. */
HRESULT win16_xaml_button_apply(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                IInspectable *button, win16_xaml_button_style *state);
/* A changed Style returns ERROR_REVISION_MISMATCH without clearing it. */
HRESULT win16_xaml_button_remove(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                 win16_xaml_button_style *state);
/* Releases retained references only; does not change the control. */
void win16_xaml_button_discard(win16_xaml_button_style *state);

typedef struct win16_xaml_button_entry win16_xaml_button_entry;
typedef struct win16_xaml_button_set {
    win16_xaml_button_entry *first;
    DWORD thread_id;
} win16_xaml_button_set;

/* The set and all its controls belong to one XAML thread. A duplicate
 * returns S_FALSE and the existing state; no style is applied twice. */
HRESULT win16_xaml_button_set_add(win16_xaml_button_set *set, IVisualTreeService *service,
                                  IXamlDiagnostics *diagnostics, IInspectable *button,
                                  win16_xaml_button_style **state);
/* Failure leaves the entry owned by the set for a later retry or discard. */
HRESULT win16_xaml_button_set_remove(win16_xaml_button_set *set, IVisualTreeService *service,
                                     IXamlDiagnostics *diagnostics, win16_xaml_button_style *state);
/* Caller must stop discovery first. Stops on the first failure and retains
 * that entry and all remaining entries. */
HRESULT win16_xaml_button_set_remove_all(win16_xaml_button_set *set, IVisualTreeService *service,
                                         IXamlDiagnostics *diagnostics);
/* Releases bookkeeping without altering styles; must run on the owning thread. */
HRESULT win16_xaml_button_set_discard(win16_xaml_button_set *set);

#endif
