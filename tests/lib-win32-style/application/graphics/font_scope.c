#include "lib-win32-style/application/graphics/font_scope.h"
#include "../../../ntw16s-dll/exception_probe.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FONT SCOPE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)

typedef struct fixture {HDC dc;HFONT source,font;unsigned returned;BOOL saved,restored;} fixture;
static void return_font(void *context,HFONT font)
{
    fixture *f=context;POINT origin;GetViewportOrgEx(f->dc,&origin);
    f->restored=GetCurrentObject(f->dc,OBJ_FONT)==f->source && (!f->saved || (origin.x==3 && origin.y==4));
    ++f->returned;DeleteObject(font);SetLastError(0x8888);
}
static LRESULT PROCESS_HOOK_UNWIND_FUNCTION CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m!=WM_APP+1 && m!=WM_APP+2)return DefWindowProcW(w,m,a,b);
    fixture *f=(fixture *)a;
    style_font_scope scope={.dc=f->dc,.font=f->font,.saved=f->saved?SaveDC(f->dc):0,.context=f,.release=return_font};
    style_font_scope_select(&scope,f->font);
    process_hook_unwind_scope unwind;LRESULT result=73;DWORD error;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_font_scope_end,&scope);
    if(f->saved)SetViewportOrgEx(f->dc,30,40,NULL);
    if(m==WM_APP+2)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
    SetLastError(0x4444);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
int main(void)
{
    HMODULE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=procedure,.lpszClassName=L"FontScopeFixture"};CHECK(RegisterClassW(&cls));
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,80,40,NULL,NULL,module,NULL);CHECK(window);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HFONT source=(HFONT)GetCurrentObject(dc,OBJ_FONT);CHECK(source);
    for(unsigned saved=0;saved<2;++saved)for(unsigned exceptional=0;exceptional<2;++exceptional){
        fixture f={.dc=dc,.source=source,.saved=saved};f.font=CreateFontW(-15,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Tahoma");CHECK(f.font);
        CHECK(SetViewportOrgEx(dc,3,4,NULL));
        if(exceptional)CHECK(test_unwind(window,WM_APP+2,(WPARAM)&f,0));
        else CHECK(SendMessageW(window,WM_APP+1,(WPARAM)&f,0)==73 && GetLastError()==0x4444);
        LOGFONTW log;CHECK(f.returned==1 && f.restored && !GetObjectW(f.font,sizeof(log),&log) && GetObjectW(source,sizeof(log),&log));
    }
    HFONT replacement=CreateFontW(-17,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Tahoma");CHECK(replacement);
    style_font_gate gate={0};style_font_scope outer,inner;
    CHECK(style_font_gate_begin(&gate,&outer,dc));style_font_scope_select(&outer,replacement);
    CHECK(!style_font_gate_begin(&gate,&inner,dc) && gate.depth==2);
    style_font_scope_end(&inner);style_font_gate_end(&gate);CHECK(gate.depth==1 && GetCurrentObject(dc,OBJ_FONT)==replacement);
    style_font_scope_end(&outer);style_font_gate_end(&gate);CHECK(!gate.depth && GetCurrentObject(dc,OBJ_FONT)==source);
    CHECK(DeleteObject(replacement));
    fixture failed={.dc=dc,.source=source};failed.font=CreateFontW(-19,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Tahoma");CHECK(failed.font);
    style_font_scope invalid={.font=failed.font,.context=&failed,.release=return_font};style_font_scope_select(&invalid,failed.font);
    CHECK(!invalid.previous);style_font_scope_end(&invalid);CHECK(failed.returned==1 && failed.restored);
    CHECK(DeleteDC(dc) && DestroyWindow(window) && UnregisterClassW(cls.lpszClassName,module));
    puts("FONT SCOPE PASS selection=1 saved_dc=1 restore_before_return=1 source_alive=1 native_unwind=1 nested_gate=1 failed_select=1 native_error=1");return 0;
}
