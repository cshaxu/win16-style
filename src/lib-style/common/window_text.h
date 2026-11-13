#ifndef STYLE_WINDOW_TEXT_H
#define STYLE_WINDOW_TEXT_H
#include <windows.h>
#include <stdlib.h>

/* Returns a heap snapshot, including an allocated empty string; caller frees it. */
static inline WCHAR *style_window_text_copy(HWND window)
{
    int length=GetWindowTextLengthW(window);
    WCHAR *text=calloc((size_t)length+1,sizeof(*text));
    if(text)GetWindowTextW(window,text,length+1);
    return text;
}
#endif
