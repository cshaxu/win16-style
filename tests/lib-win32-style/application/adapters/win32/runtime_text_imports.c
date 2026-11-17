#include "lib-win32-style/application/adapters/win32/runtime_text_imports.h"
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include <stdint.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TEXT IMPORTS FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)

static HFONT replacement;
static unsigned selections;
static BOOL nested,barrier;
static HFONT choose_font(HDC dc)
{
    ++selections;
    if(nested){nested=FALSE;TextOutA(dc,2,2,"nested",6);}
    return replacement;
}
static BOOL include(HWND w,UINT m){(void)w;(void)m;return TRUE;}
static style_runtime_text_role role(HWND w,void *context)
{(void)context;return (style_runtime_text_role){barrier?NULL:w,FALSE};}
static uint64_t render(HDC dc,DWORD *pixels,unsigned kind,DWORD *error)
{
    RECT r={0,0,220,50};char text[]="Shared text";BOOL ok=FALSE;
    FillRect(dc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(0,0,0));
    SetLastError(0x4321);
    switch(kind){
    case 0:ok=DrawTextA(dc,text,-1,&r,DT_SINGLELINE)>0;break;
    case 1:ok=DrawTextExA(dc,text,-1,&r,DT_SINGLELINE,NULL)>0;break;
    case 2:ok=TextOutA(dc,2,2,text,11);break;
    case 3:ok=ExtTextOutA(dc,2,2,ETO_CLIPPED,&r,text,11,NULL);break;
    }
    *error=GetLastError();if(!ok || !GdiFlush())return 0;
    uint64_t hash=UINT64_C(1469598103934665603);
    for(unsigned i=0;i<220*50;++i)hash=(hash^pixels[i])*UINT64_C(1099511628211);
    return hash;
}
int main(void)
{
    HWND window=CreateWindowW(L"STATIC",L"Independent text provider",WS_POPUP,0,0,220,50,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(window);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=220,.biHeight=-50,.biPlanes=1,.biBitCount=32}};DWORD *pixels=NULL;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void **)&pixels,NULL,0);CHECK(bitmap && pixels);
    HGDIOBJ old_bitmap=SelectObject(dc,bitmap);
    HFONT source=CreateFontW(-12,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Tahoma");
    replacement=CreateFontW(-26,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Courier New");CHECK(source && replacement);
    HGDIOBJ old_font=SelectObject(dc,source);uint64_t native[4];DWORD native_error[4];
    for(unsigned i=0;i<4;++i){native[i]=render(dc,pixels,i,&native_error[i]);CHECK(native[i]);}
    BOOL changed=TRUE;CHECK(style_runtime_text_imports(NULL,choose_font,&changed)==ERROR_INVALID_PARAMETER && !changed);
    CHECK(!style_runtime_text_imports(GetModuleHandleW(NULL),choose_font,&changed) && changed);
    CHECK(!style_runtime_text_imports(GetModuleHandleW(NULL),choose_font,&changed) && !changed);
    for(unsigned i=0;i<4;++i){DWORD error;
        CHECK(render(dc,pixels,i,&error)==native[i] && error==native_error[i] && !selections);
    }
    const style_runtime_text_ops ops={NULL,include,role};style_runtime_text_scope scope;
    barrier=TRUE;style_runtime_text_begin(&scope,window,WM_PAINT,NULL,&ops);
    for(unsigned i=0;i<4;++i){DWORD error;CHECK(render(dc,pixels,i,&error)==native[i] && !selections);}
    style_runtime_text_end(&scope);barrier=FALSE;
    style_runtime_text_begin(&scope,window,WM_PAINT,NULL,&ops);
    for(unsigned i=0;i<4;++i){DWORD error;nested=TRUE;uint64_t styled=render(dc,pixels,i,&error);
        CHECK(styled && styled!=native[i] && !nested && selections==i+1 && GetCurrentObject(dc,OBJ_FONT)==source);
    }
    style_runtime_text_end(&scope);
    SelectObject(dc,old_font);SelectObject(dc,old_bitmap);
    CHECK(DeleteObject(source) && DeleteObject(replacement) && DeleteObject(bitmap) && DeleteDC(dc) && DestroyWindow(window));
    puts("TEXT IMPORTS PASS self_owned_module=1 independent_provider=1 native4=1 barrier4=1 styled4=1 nested_gate=1 font_restored=1 idempotent=1");return 0;
}
