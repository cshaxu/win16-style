#include "lib-win32-style/application/control/scroll_resources.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SCROLL_RESOURCES line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    HWND window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,module,NULL);
    HWND other=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,module,NULL);
    CHECK(window && other);
    style_scroll_resources state={0};RECT rectangle={2,3,8,12};
    style_scroll_caret_sync(window,&state,&rectangle,FALSE);
    CHECK(state.caret && state.caret_shown && state.caret_size.cx==6 && state.caret_size.cy==9);
    GUITHREADINFO info={.cbSize=sizeof(info)};CHECK(GetGUIThreadInfo(GetCurrentThreadId(),&info) && info.hwndCaret==window);
    style_scroll_caret_sync(window,&state,&rectangle,TRUE);CHECK(state.caret && !state.caret_shown);
    rectangle.right=12;style_scroll_caret_sync(window,&state,&rectangle,FALSE);
    CHECK(state.caret && state.caret_shown && state.caret_size.cx==10);
    CHECK(CreateCaret(other,NULL,2,4));
    style_scroll_caret_destroy(window,&state);
    CHECK(!state.caret && !state.caret_shown && GetGUIThreadInfo(GetCurrentThreadId(),&info) && info.hwndCaret==other);
    CHECK(DestroyCaret());
    style_scroll_caret_sync(window,&state,NULL,FALSE);CHECK(!state.caret);
    state.timer=SetTimer(window,0,10000,NULL);CHECK(state.timer);
    style_scroll_stop_timer(window,&state.timer);CHECK(!state.timer);
    SetCapture(other);CHECK(GetCapture()==other);state.previous_capture=GetCapture();
    SetCapture(window);CHECK(GetCapture()==window);
    style_scroll_release_capture(window,&state.previous_capture,TRUE);
    CHECK(!state.previous_capture && GetCapture()==other);
    state.previous_capture=other;style_scroll_release_capture(window,&state.previous_capture,FALSE);
    CHECK(!state.previous_capture && GetCapture()==other);CHECK(ReleaseCapture());
    CHECK(DestroyWindow(window) && DestroyWindow(other));
    puts("SCROLL_RESOURCES caret resize/hide/foreign ownership, timer stop, borrowed capture restore PASS");return 0;
}
