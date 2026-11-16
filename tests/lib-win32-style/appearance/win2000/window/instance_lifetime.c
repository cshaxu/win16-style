#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
static unsigned allocations,releases;
static BOOL deny_allocation;
static void *allocate(size_t count,size_t size)
{if(deny_allocation)return NULL;void *value=calloc(count,size);if(value)++allocations;return value;}
static void release(void *value){if(value)++releases;free(value);}
#define calloc allocate
#define free release
#include "lib-win32-style/appearance/win2000/window/frame.c"
#undef calloc
#undef free

#define CHECK(x) do { if(!(x)){fprintf(stderr,"WIN2K_INSTANCE_FAIL line=%d allocations=%u releases=%u error=%lu\n",__LINE__,allocations,releases,GetLastError());return 1;} } while(0)
static BOOL reentrant_removed,reentrant_attached,retained_during_callback;
static LRESULT CALLBACK native(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_APP){
        unsigned before=releases;
        reentrant_removed=win2k_frame_remove(window);
        retained_during_callback=releases==before;
        reentrant_attached=win2k_frame_attach(window);
    }
    return DefWindowProcW(window,message,wp,lp);
}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=native,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"StyleInstanceLifetime"};
    CHECK(RegisterClassW(&cls));
    for(unsigned i=0;i<8;++i){
        HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
        CHECK(window && win2k_frame_attach(window) && win2k_frame_attached(window));
        CHECK(win2k_frame_remove(window) && !win2k_frame_attached(window) && allocations==releases);
        CHECK(win2k_frame_attach(window));
        CHECK(DestroyWindow(window) && allocations==releases);
    }
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && win2k_frame_attach(window));
    unsigned before=releases;
    SendMessageW(window,WM_APP,0,0);
    CHECK(reentrant_removed && reentrant_attached && retained_during_callback && releases==before+1);
    CHECK(win2k_frame_attached(window) && DestroyWindow(window) && allocations==releases);

    window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && win2k_frame_attach(window) && !win2k_frame_attached(window));
    unsigned pending_allocations=allocations;
    CHECK(win2k_frame_attach(window) && allocations==pending_allocations && !win2k_frame_attached(window));
    CHECK(win2k_frame_remove(window) && allocations==releases);
    CHECK(win2k_frame_attach(window));
    LONG_PTR previous=GetWindowLongPtrW(window,GWL_STYLE);
    LONG_PTR returned=SetWindowLongPtrW(window,GWL_STYLE,WS_OVERLAPPEDWINDOW);
    printf("WIN2K_STYLE_VALUES previous=%llx returned=%llx\n",(unsigned long long)(UINT_PTR)previous,(unsigned long long)(UINT_PTR)returned);
    CHECK((DWORD)returned==(DWORD)previous);
    CHECK(win2k_frame_attached(window) && DestroyWindow(window) && allocations==releases);

    window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window);
    deny_allocation=TRUE;
    CHECK(!win2k_frame_attach(window) && GetLastError()==ERROR_NOT_ENOUGH_MEMORY && allocations==releases);
    deny_allocation=FALSE;
    style_instance foreign={0};
    CHECK(SetWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc,(DWORD_PTR)&foreign));
    CHECK(!win2k_frame_attach(window) && GetLastError()==ERROR_ALREADY_EXISTS && allocations==releases);
    CHECK(RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc) && DestroyWindow(window));
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("WIN2K_INSTANCE_OK repeat=8 remove=1 terminal=1 reentrant_replace=1 pending=1 pending_repeat=1 allocation_failure=1 attach_failure=1 cleanup=1");
    return 0;
}
