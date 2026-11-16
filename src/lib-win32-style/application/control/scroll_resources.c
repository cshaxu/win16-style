#include "scroll_resources.h"

void style_scroll_caret_destroy(HWND window,style_scroll_resources *resources)
{
    GUITHREADINFO info={.cbSize=sizeof(info)};
    if(resources->caret && GetGUIThreadInfo(GetCurrentThreadId(),&info) && info.hwndCaret==window)DestroyCaret();
    resources->caret=resources->caret_shown=FALSE;
}
void style_scroll_caret_sync(HWND window,style_scroll_resources *resources,const RECT *rectangle,BOOL tracking)
{
    if(!rectangle || IsRectEmpty(rectangle)){style_scroll_caret_destroy(window,resources);return;}
    int width=rectangle->right-rectangle->left,height=rectangle->bottom-rectangle->top;
    if(resources->caret && (resources->caret_size.cx!=width || resources->caret_size.cy!=height))
        style_scroll_caret_destroy(window,resources);
    if(!resources->caret){
        if(!CreateCaret(window,(HBITMAP)1,width,height))return;
        resources->caret=TRUE;resources->caret_size=(SIZE){width,height};
    }
    SetCaretPos(rectangle->left,rectangle->top);
    if(tracking && resources->caret_shown){HideCaret(window);resources->caret_shown=FALSE;}
    else if(!tracking && !resources->caret_shown){ShowCaret(window);resources->caret_shown=TRUE;}
}
