#ifndef WIN32_STYLE_CREATION_PENDING_H
#define WIN32_STYLE_CREATION_PENDING_H
#include <windows.h>
#include <stdlib.h>

/* Per-creation-scope ownership; callers decide how a recorded window is
 * styled and release every remaining record when the scope ends. */
typedef struct win32_style_creation_pending {
    struct win32_style_creation_pending *next;
    HWND window;
    DWORD style;
    BOOL settled,installed;
} win32_style_creation_pending;

static inline win32_style_creation_pending *win32_style_creation_add(
    win32_style_creation_pending **head,HWND window,DWORD style)
{
    win32_style_creation_pending *record=calloc(1,sizeof(*record));
    if(record){record->window=window;record->style=style;record->next=*head;*head=record;}
    return record;
}
static inline win32_style_creation_pending *win32_style_creation_find(
    win32_style_creation_pending *head,HWND window)
{
    while(head && head->window!=window)head=head->next;
    return head;
}
static inline win32_style_creation_pending *win32_style_creation_take(
    win32_style_creation_pending **head,HWND window)
{
    while(*head && (*head)->window!=window)head=&(*head)->next;
    win32_style_creation_pending *record=*head;
    if(record)*head=record->next;
    return record;
}
/* Installation can reenter USER and remove this record. Only dereference it
 * after finding the same live record in its owning list again. */
static inline BOOL win32_style_creation_contains(win32_style_creation_pending *head,
    const win32_style_creation_pending *record,HWND window)
{
    for(;head;head=head->next)if(head==record && head->window==window)return TRUE;
    return FALSE;
}
#endif
