#include "lib-style/common/window_text.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do { if(!(x)){printf("WINDOW_TEXT_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"First \x4e2d",WS_POPUP,0,0,10,10,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    WCHAR *first=style_window_text_copy(w);CHECK(first && !wcscmp(first,L"First \x4e2d"));
    CHECK(SetWindowTextW(w,L"Second"));WCHAR *second=style_window_text_copy(w);
    CHECK(second && !wcscmp(second,L"Second") && !wcscmp(first,L"First \x4e2d"));
    CHECK(SetWindowTextW(w,L""));WCHAR *empty=style_window_text_copy(w);CHECK(empty && !*empty);
    free(first);free(second);free(empty);CHECK(DestroyWindow(w));
    puts("WINDOW_TEXT_OK unicode=1 snapshot=1 empty=1 cleanup=1");return 0;
}
