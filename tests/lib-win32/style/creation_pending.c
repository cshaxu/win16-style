#include "lib-win32/style/creation_pending.h"
#include <stdint.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CREATION PENDING FAIL line=%d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    HWND a=(HWND)(uintptr_t)1,b=(HWND)(uintptr_t)2;
    win32_style_creation_pending *outer=NULL,*inner=NULL;
    CHECK(!win32_style_creation_find(outer,a) && !win32_style_creation_take(&outer,a));
    win32_style_creation_pending *first=win32_style_creation_add(&outer,a,WS_VISIBLE);
    win32_style_creation_pending *second=win32_style_creation_add(&outer,b,WS_CHILD);
    win32_style_creation_pending *nested=win32_style_creation_add(&inner,a,WS_POPUP);
    CHECK(first && second && nested && !first->settled && !first->installed);
    CHECK(win32_style_creation_find(outer,a)==first && win32_style_creation_find(inner,a)==nested);
    CHECK(win32_style_creation_contains(outer,first,a) && !win32_style_creation_contains(outer,nested,a) &&
        !win32_style_creation_contains(outer,first,b));
    first->settled=TRUE;first->installed=TRUE;
    CHECK(win32_style_creation_take(&outer,a)==first && first->settled && first->installed && first->style==WS_VISIBLE);
    CHECK(outer==second && !win32_style_creation_find(outer,a) && !win32_style_creation_contains(outer,first,a));free(first);
    CHECK(win32_style_creation_take(&outer,b)==second && !outer && second->style==WS_CHILD);free(second);
    CHECK(win32_style_creation_take(&inner,a)==nested && !inner && nested->style==WS_POPUP);free(nested);
    puts("CREATION PENDING PASS nested=1 order=1 identity=1 status=1 remove=1");return 0;
}
