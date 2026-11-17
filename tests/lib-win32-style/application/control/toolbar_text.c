#include "lib-win32-style/application/control/toolbar_text.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TOOLBAR_TEXT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static unsigned text_queries;
static BOOL admitted=TRUE;
static BOOL accept_item(HWND w,const TBBUTTON *item)
{(void)w;return admitted && item->iBitmap==I_IMAGENONE && item->idCommand && !(item->fsStyle&BTNS_SEP);}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR context)
{(void)id;(void)context;if(m==TB_GETBUTTONINFOW)++text_queries;return DefSubclassProc(w,m,a,b);}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND toolbar=CreateWindowW(TOOLBARCLASSNAMEW,L"",WS_CHILD,0,0,300,32,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(toolbar);
    SendMessageW(toolbar,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);CHECK(SetWindowSubclass(toolbar,observe,1,0));
    TBBUTTON buttons[2]={{.iBitmap=I_IMAGENONE,.idCommand=1,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE|BTNS_SHOWTEXT,.iString=(INT_PTR)L"One"},{.iBitmap=I_IMAGENONE,.idCommand=2,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE|BTNS_SHOWTEXT,.iString=(INT_PTR)L"Two"}};
    CHECK(SendMessageW(toolbar,TB_ADDBUTTONSW,2,(LPARAM)buttons));style_toolbar_text_item item;
    CHECK(style_toolbar_text_supported(toolbar,accept_item));CHECK(style_toolbar_text_read(toolbar,1,&item,accept_item) && item.button.idCommand==2 && !lstrcmpW(item.text,L"Two") && item.rect.right>item.rect.left);
    CHECK(style_toolbar_text_find(toolbar,1,&item,accept_item) && !lstrcmpW(item.text,L"One"));CHECK(!style_toolbar_text_find(toolbar,99,&item,accept_item));CHECK(!style_toolbar_text_read(toolbar,2,&item,accept_item));
    admitted=FALSE;text_queries=0;CHECK(!style_toolbar_text_read(toolbar,0,&item,accept_item) && !text_queries);admitted=TRUE;
    WCHAR long_text[513];for(unsigned i=0;i<512;++i)long_text[i]=L'X';long_text[512]=0;
    TBBUTTONINFOW info={.cbSize=sizeof(info),.dwMask=TBIF_TEXT,.pszText=long_text};CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,1,(LPARAM)&info));CHECK(!style_toolbar_text_read(toolbar,0,&item,accept_item));
    info.pszText=L"One";CHECK(SendMessageW(toolbar,TB_SETBUTTONINFOW,1,(LPARAM)&info));CHECK(style_toolbar_text_read(toolbar,0,&item,accept_item));
    TBBUTTON *extra=calloc(1022,sizeof(*extra));CHECK(extra);
    for(unsigned i=0;i<1022;++i)extra[i]=(TBBUTTON){.iBitmap=I_IMAGENONE,.idCommand=100+(int)i,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE|BTNS_SHOWTEXT,.iString=(INT_PTR)L"Extra"};
    CHECK(SendMessageW(toolbar,TB_ADDBUTTONSW,1022,(LPARAM)extra));free(extra);CHECK(SendMessageW(toolbar,TB_BUTTONCOUNT,0,0)==1024);CHECK(style_toolbar_text_supported(toolbar,accept_item));
    TBBUTTON last={.iBitmap=I_IMAGENONE,.idCommand=5000,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE|BTNS_SHOWTEXT,.iString=(INT_PTR)L"Last"};CHECK(SendMessageW(toolbar,TB_ADDBUTTONSW,1,(LPARAM)&last));text_queries=0;CHECK(!style_toolbar_text_supported(toolbar,accept_item) && !text_queries);
    CHECK(RemoveWindowSubclass(toolbar,observe,1));CHECK(DestroyWindow(toolbar));CHECK(DestroyWindow(parent));
    puts("TOOLBAR_TEXT PASS native_items=1 text=1 find=1 reject_before_query=1 bounded_text=1 cap1024=1 cap1025=1 no_style=1 cleanup=1");return 0;
}
