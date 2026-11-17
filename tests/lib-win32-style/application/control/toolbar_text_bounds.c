#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include "../../../ntw16s-dll/isolated_process.h"
#include "lib-win32-style/application/control/toolbar_text.h"

/* The fixture calls the real COMCTL toolbar with the production reader. */
static BOOL accept_text_item(HWND w,const TBBUTTON *item)
{(void)w;return item->iBitmap==I_IMAGENONE && item->idCommand!=0;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TOOLBAR_BOUNDS_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;
    INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&cc));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,900,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND toolbar=CreateWindowExW(0,TOOLBARCLASSNAMEW,L"",WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT,0,0,800,80,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(toolbar);
    SendMessageW(toolbar,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);
    WCHAR long_text[701];for(unsigned i=0;i<700;++i)long_text[i]=L'A';long_text[700]=0;
    TBBUTTON buttons[2]={{.iBitmap=I_IMAGENONE,.idCommand=101,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE|BTNS_SHOWTEXT,.iString=(INT_PTR)long_text},
        {.iBitmap=I_IMAGENONE,.idCommand=102,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE,.iString=(INT_PTR)L"Open"}};
    CHECK(SendMessageW(toolbar,TB_ADDBUTTONSW,2,(LPARAM)buttons));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    struct {style_toolbar_text_item item;WCHAR guard[512];} buffer;
    for(unsigned i=0;i<ARRAYSIZE(buffer.guard);++i)buffer.guard[i]=0x4b4b;
    BOOL accepted=style_toolbar_text_read(toolbar,0,&buffer.item,accept_text_item);
    unsigned modified=0;for(unsigned i=0;i<ARRAYSIZE(buffer.guard);++i)modified+=buffer.guard[i]!=0x4b4b;
    printf("TOOLBAR_BOUNDS_PROBE bits=%u long_text=700 accepted=%u guard_modified=%u\n",(unsigned)(sizeof(void*)*8),accepted,modified);
    CHECK(!modified && !accepted);
    CHECK(style_toolbar_text_read(toolbar,1,&buffer.item,accept_text_item) && !wcscmp(buffer.item.text,L"Open"));
    long_text[510]=0;TBBUTTONINFOW info={.cbSize=sizeof(info),.dwMask=TBIF_TEXT,.pszText=long_text};
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(style_toolbar_text_read(toolbar,0,&buffer.item,accept_text_item) && wcslen(buffer.item.text)==510);
    long_text[510]=L'A';long_text[511]=0;
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(!style_toolbar_text_read(toolbar,0,&buffer.item,accept_text_item));
    info.pszText=L"Short caption";
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(style_toolbar_text_read(toolbar,0,&buffer.item,accept_text_item) && !wcscmp(buffer.item.text,L"Short caption"));
    puts("TOOLBAR_SHORT_OK bounded_text=1 initial_string=1 length510=1 length511_fallback=1");
    CHECK(DestroyWindow(toolbar) && DestroyWindow(parent));
    puts("TOOLBAR_BOUNDS_OK long_text_native_fallback=1 guard=preserved");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
