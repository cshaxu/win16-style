#include "lib-win32-style/application/window/frame.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FRAME_UPDATE_DC_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    HRGN update=CreateRectRgn(1,2,20,30);CHECK(update);
    HDC dc=style_frame_update_dc(w,update);CHECK(dc);
    RECT box;CHECK(GetRgnBox(update,&box)!=ERROR && box.left==1 && box.top==2 && box.right==20 && box.bottom==30);
    CHECK(ReleaseDC(w,dc));CHECK(DeleteObject(update));
    CHECK(!style_frame_update_dc(w,NULL));CHECK(DestroyWindow(w));
    puts("FRAME_UPDATE_DC_OK copied_region=1 invalid=1 cleanup=1");return 0;
}
