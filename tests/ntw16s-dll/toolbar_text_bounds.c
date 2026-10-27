#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#include "../../src/ntw16s-dll/toolbar_paint.c"

/* The fixture calls the real COMCTL toolbar with the production reader. */
BOOL ntw16s_control_protocol_toolbar(HWND w){return w!=NULL;}
HFONT win16_style_system_acquire(HFONT source){return source;}
void win16_style_system_release(HFONT font){(void)font;}
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
    struct {WCHAR text[512];WCHAR guard[512];} buffer;
    for(unsigned i=0;i<ARRAYSIZE(buffer.guard);++i)buffer.guard[i]=0x4b4b;
    TBBUTTON actual;RECT rect;BOOL accepted=button(toolbar,0,&actual,buffer.text,ARRAYSIZE(buffer.text),&rect);
    unsigned modified=0;for(unsigned i=0;i<ARRAYSIZE(buffer.guard);++i)modified+=buffer.guard[i]!=0x4b4b;
    printf("TOOLBAR_BOUNDS_PROBE bits=%u long_text=700 accepted=%u guard_modified=%u\n",(unsigned)(sizeof(void*)*8),accepted,modified);
    CHECK(!modified && !accepted);
    CHECK(button(toolbar,1,&actual,buffer.text,ARRAYSIZE(buffer.text),&rect) && !wcscmp(buffer.text,L"Open"));
    long_text[510]=0;TBBUTTONINFOW info={.cbSize=sizeof(info),.dwMask=TBIF_TEXT,.pszText=long_text};
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(button(toolbar,0,&actual,buffer.text,ARRAYSIZE(buffer.text),&rect) && wcslen(buffer.text)==510);
    long_text[510]=L'A';long_text[511]=0;
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(!button(toolbar,0,&actual,buffer.text,ARRAYSIZE(buffer.text),&rect));
    info.pszText=L"Short caption";
    CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,101,(LPARAM)&info));SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    CHECK(button(toolbar,0,&actual,buffer.text,ARRAYSIZE(buffer.text),&rect) && !wcscmp(buffer.text,L"Short caption"));
    puts("TOOLBAR_SHORT_OK bounded_text=1 initial_string=1 length510=1 length511_fallback=1");
    CHECK(DestroyWindow(toolbar) && DestroyWindow(parent));
    puts("TOOLBAR_BOUNDS_OK long_text_native_fallback=1 guard=preserved");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
