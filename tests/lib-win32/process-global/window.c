#include "lib-win32/process-global/window.h"
#include <stdio.h>
typedef struct seen {HWND parent,child,grandchild;unsigned mask;BOOL cancel;} seen;
static BOOL visit(HWND w,void *context)
{
    seen *s=context;
    if(w==s->parent)s->mask|=1;
    if(w==s->child)s->mask|=2;
    if(w==s->grandchild)s->mask|=4;
    return !(s->cancel && w==s->parent);
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    HWND parent=CreateWindowW(L"STATIC",L"owned",WS_OVERLAPPED,0,0,120,100,NULL,NULL,instance,NULL);
    HWND child=CreateWindowW(L"STATIC",L"child",WS_CHILD,0,0,60,60,parent,NULL,instance,NULL);
    HWND grandchild=CreateWindowW(L"STATIC",L"grandchild",WS_CHILD,0,0,30,30,child,NULL,instance,NULL);
    if(!parent || !child || !grandchild)return 1;
    seen s={parent,child,grandchild,0,FALSE};int error=0;
    if(!win32_process_global_window_walk(GetCurrentThreadId(),GetTickCount64()+3000,visit,&s) || s.mask!=7)error=2;
    s.mask=0;if(!error && (!win32_process_global_window_walk(0,GetTickCount64()+3000,visit,&s) || s.mask!=7))error=3;
    s.mask=0;s.cancel=TRUE;if(!error && (win32_process_global_window_walk(GetCurrentThreadId(),GetTickCount64()+3000,visit,&s) || GetLastError()!=ERROR_CANCELLED || s.mask!=1))error=4;
    s.mask=0;s.cancel=FALSE;if(!error && (win32_process_global_window_walk(GetCurrentThreadId(),0,visit,&s) || GetLastError()!=ERROR_TIMEOUT || s.mask))error=5;
    if(!error && (win32_process_global_window_walk(0,0,NULL,NULL) || GetLastError()!=ERROR_INVALID_PARAMETER))error=6;
    DestroyWindow(parent);
    if(error){printf("WINDOW WALK FAIL %d\n",error);return error;}
    puts("WINDOW WALK PASS desktop=1 thread=1 children=1 cancel=1 timeout=1");return 0;
}
