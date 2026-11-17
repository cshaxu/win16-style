#ifndef STYLE_TOOLBAR_TEXT_H
#define STYLE_TOOLBAR_TEXT_H
#include <windows.h>
#include <commctrl.h>
/* Existing bounded text-item protocol. Caller first verifies toolbar/native
 * eligibility. accept sees raw metadata before any text query and supplies
 * the presentation support policy. iString stays opaque, never dereferenced.
 * Values/text/rect belong to the complete caller-owned snapshot. */
typedef struct style_toolbar_text_item {TBBUTTON button;WCHAR text[512];RECT rect;} style_toolbar_text_item;
BOOL style_toolbar_text_read(HWND,int,style_toolbar_text_item *,BOOL (*)(HWND,const TBBUTTON *));
BOOL style_toolbar_text_supported(HWND,BOOL (*)(HWND,const TBBUTTON *));
BOOL style_toolbar_text_find(HWND,DWORD_PTR,style_toolbar_text_item *,BOOL (*)(HWND,const TBBUTTON *));
#endif
