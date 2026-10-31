#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "ntw16s-dll/global_vb_text.h"
#include "ntw16s-dll/import_scan.h"
#include "ntw16s-dll/vb_text.h"
#include "isolated_process.h"

#define CHECK(x) do{if(!(x)){printf("GLOBAL_VB_TEXT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef BOOL (WINAPI *register_fn)(void);
typedef BOOL (WINAPI *draw_fn)(HDC,unsigned);
static const ntw16s_import_query queries[]={
    {"USER32.dll","DrawTextA"},{"USER32.dll","DrawTextExA"},
    {"GDI32.dll","TextOutA"},{"GDI32.dll","ExtTextOutA"}};

/* This test links the paint-scope module but not the attachment classifier. */
BOOL ntw16s_vb_user_label_target(HWND w){(void)w;return FALSE;}
static void foreign_hook(void){}
static BOOL set_slot(void **slot,void *expected,void *replacement)
{
    MEMORY_BASIC_INFORMATION info;DWORD before,ignored;
    if(!VirtualQuery(slot,&info,sizeof(info)))return FALSE;
    DWORD type=info.Protect&0xff;
    DWORD write=(type==PAGE_EXECUTE_READ || type==PAGE_EXECUTE_READWRITE || type==PAGE_EXECUTE_WRITECOPY)?PAGE_EXECUTE_READWRITE:PAGE_READWRITE;
    if(!VirtualProtect(slot,sizeof(*slot),write,&before))return FALSE;
    void *was=InterlockedCompareExchangePointer((void *volatile *)slot,replacement,expected);
    return VirtualProtect(slot,sizeof(*slot),before,&ignored) && was==expected;
}
static uint64_t draw_hash(HDC dc,void *pixels,draw_fn draw,unsigned mode,HWND form,
                          BOOL scoped,ntw16s_vb_fonts *fonts,DWORD *last)
{
    RECT bounds={0,0,200,40};FillRect(dc,&bounds,(HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(0,0,0));
    ntw16s_vb_paint paint;if(scoped)ntw16s_vb_paint_begin(&paint,form,WM_PAINT,fonts);
    SetLastError(0x2345);BOOL ok=draw(dc,mode);*last=GetLastError();
    if(scoped)ntw16s_vb_paint_end(&paint);
    if(!ok || !GdiFlush())return 0;
    uint64_t hash=UINT64_C(1469598103934665603);const DWORD *data=pixels;
    for(size_t i=0;i<200u*40u;++i)hash=(hash^data[i])*UINT64_C(1099511628211);
    return hash;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    HMODULE runtime=LoadLibraryW(argv[1]);CHECK(runtime);
    register_fn reg=(void *)GetProcAddress(runtime,"VbGlobalRegister"),release=(void *)GetProcAddress(runtime,"VbGlobalRelease");
    draw_fn draw=(void *)GetProcAddress(runtime,"VbGlobalDraw");CHECK(reg && release && draw && reg());
    HWND form=CreateWindowW(L"ThunderRT6FormDC",L"test",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,runtime,NULL);CHECK(form);
    void **slots[4]={0};void *original[4];CHECK(!ntw16s_import_slots(runtime,queries,4,slots));
    for(unsigned i=0;i<4;++i){CHECK(slots[i] && *slots[i]);original[i]=*slots[i];}
    BOOL changed=TRUE;
    CHECK(ntw16s_global_vb_text(GetDesktopWindow(),&changed)==ERROR_NOT_SUPPORTED && !changed);
    CHECK(set_slot(slots[0],original[0],(void *)foreign_hook));
    CHECK(ntw16s_global_vb_text(form,&changed)==ERROR_BUSY && !changed);
    CHECK(*slots[0]==(void *)foreign_hook);
    for(unsigned i=1;i<4;++i)CHECK(*slots[i]==original[i]);
    CHECK(set_slot(slots[0],(void *)foreign_hook,original[0]));
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);BITMAPINFO info={0};void *pixels=NULL;
    info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=200;info.bmiHeader.biHeight=-40;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap && pixels);
    HGDIOBJ old_bitmap=SelectObject(dc,bitmap);HFONT source=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(source);
    HGDIOBJ old_font=SelectObject(dc,source);ntw16s_vb_fonts fonts={0};uint64_t native[4];DWORD native_error[4];
    for(unsigned i=0;i<4;++i){native[i]=draw_hash(dc,pixels,draw,i,form,TRUE,&fonts,&native_error[i]);CHECK(native[i]);}
    CHECK(!ntw16s_global_vb_text(form,&changed) && changed);
    for(unsigned i=0;i<4;++i)CHECK(*slots[i]!=original[i]);
    CHECK(!ntw16s_global_vb_text(form,&changed) && !changed);
    for(unsigned i=0;i<4;++i){DWORD error=0;
        CHECK(draw_hash(dc,pixels,draw,i,form,FALSE,&fonts,&error)==native[i] && error==native_error[i]);
        uint64_t styled=draw_hash(dc,pixels,draw,i,form,TRUE,&fonts,&error);
        CHECK(styled && styled!=native[i] && error==native_error[i] && GetCurrentObject(dc,OBJ_FONT)==source);
    }
    ntw16s_vb_fonts_clear(&fonts);SelectObject(dc,old_font);DeleteObject(source);
    SelectObject(dc,old_bitmap);DeleteObject(bitmap);DeleteDC(dc);
    CHECK(DestroyWindow(form) && release() && FreeLibrary(runtime));
    CHECK(!GetModuleHandleW(L"MSVBVM60.DLL"));
    runtime=LoadLibraryW(argv[1]);CHECK(runtime);reg=(void *)GetProcAddress(runtime,"VbGlobalRegister");
    release=(void *)GetProcAddress(runtime,"VbGlobalRelease");CHECK(reg && release && reg());
    form=CreateWindowW(L"ThunderRT6FormDC",L"reload",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,runtime,NULL);CHECK(form);
    CHECK(!ntw16s_global_vb_text(form,&changed) && changed);
    CHECK(DestroyWindow(form) && release() && FreeLibrary(runtime));
    printf("GLOBAL_VB_TEXT PASS bits=%u hooks=4 foreign=1 scoped=4 native=4 reload=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
