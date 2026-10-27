#undef calloc
#undef free
#include "ntw16s-dll/font.h"
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/vb_text.h"
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB FONT FAIL %u error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
/* This standalone font test does not link the window attachment module. */
BOOL ntw16s_vb_user_label_target(HWND window)
{
    (void)window;
    return FALSE;
}
static int font_pixels(void)
{
    HDC dc=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateBitmap(128,64,1,1,NULL);
    CHECK(dc && bitmap);HGDIOBJ oldbitmap=SelectObject(dc,bitmap);
    for(int height=12;height<=16;height+=2){
        HFONT source=CreateFontW(-height,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(source);
        HGDIOBJ old=SelectObject(dc,source);TEXTMETRICW native,adapted;SIZE before,after;
        CHECK(GetTextMetricsW(dc,&native) && GetTextExtentPoint32W(dc,L"0123456789",10,&before));
        HFONT font=win16_style_vb_font_acquire(source);CHECK(font);
        SelectObject(dc,font);CHECK(GetTextMetricsW(dc,&adapted) && GetTextExtentPoint32W(dc,L"0123456789",10,&after));
        CHECK(adapted.tmHeight==native.tmHeight && adapted.tmAscent==native.tmAscent && before.cx==after.cx);
        RECT r={0,0,128,64};FillRect(dc,&r,GetStockObject(WHITE_BRUSH));
        SetBkColor(dc,RGB(255,255,255));SetTextColor(dc,RGB(0,0,0));
        CHECK(TextOutW(dc,0,0,L"0",1));GdiFlush();
        int top=-1,bottom=-1,left=128,right=-1;
        for(int y=0;y<adapted.tmHeight;++y)for(int x=0;x<20;++x)if(GetPixel(dc,x,y)==RGB(0,0,0)){
            if(top<0)top=y;
            bottom=y;if(x<left)left=x;if(x>right)right=x;}
        CHECK(top>=0 && bottom>top && right>left);
        /* A closed zero has a cap between its side stems. Point sampling
         * at height14 used to delete that cap and turn the digit into U. */
        CHECK(GetPixel(dc,(left+right)/2,top)==RGB(0,0,0));
        CHECK(GetPixel(dc,(left+right)/2,bottom)==RGB(0,0,0));
        SelectObject(dc,old);win16_style_system_release(font);DeleteObject(source);
        printf("VB_FONT_ZERO_OK requested=%d cell=%ld advance=%ld top=%d bottom=%d\n",height,adapted.tmHeight,after.cx,top,bottom);
    }
    SelectObject(dc,oldbitmap);DeleteObject(bitmap);DeleteDC(dc);return 0;
}

static void *allocations[256];
static unsigned live_allocations,fail_next;
void *ntw16s_test_calloc(size_t count,size_t size)
{
    if(fail_next){--fail_next;return NULL;}
    void *memory=calloc(count,size);
    if(memory){unsigned i;for(i=0;i<ARRAYSIZE(allocations);++i)if(!allocations[i])break;
        if(i==ARRAYSIZE(allocations))ExitProcess(99);
        allocations[i]=memory;++live_allocations;}
    return memory;
}
void ntw16s_test_free(void *memory)
{
    if(memory)for(unsigned i=0;i<ARRAYSIZE(allocations);++i)if(allocations[i]==memory){
        allocations[i]=NULL;--live_allocations;break;}
    free(memory);
}
static int ownership(const WCHAR *runtime_path)
{
    HMODULE runtime=LoadLibraryW(runtime_path);CHECK(runtime);
    BOOL (WINAPI *registration)(void)=(void *)GetProcAddress(runtime,"VbRegister");
    void (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbRelease");CHECK(registration && release && registration());
    HWND root=CreateWindowW(L"STATIC",L"ownership",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND window=CreateWindowW(L"ThunderRT6Frame",L"Frame",WS_CHILD|BS_GROUPBOX,0,0,200,100,root,NULL,runtime,NULL);CHECK(window);
    HWND barrier=CreateWindowW(L"STATIC",L"application",WS_CHILD,0,0,20,20,window,NULL,GetModuleHandleW(NULL),NULL);CHECK(barrier);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HFONT sources[33]={0},leases[32]={0};HGDIOBJ old=GetCurrentObject(dc,OBJ_FONT);
    ntw16s_vb_fonts fonts={0},barrier_fonts={0};ntw16s_vb_paint scope,nested;
    unsigned baseline=live_allocations;
    ntw16s_vb_paint_begin(&scope,window,WM_PRINTCLIENT,&fonts);CHECK(ntw16s_vb_paint_active());
    for(unsigned i=0;i<33;++i){
        sources[i]=CreateFontW(-12-(int)i,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(sources[i]);
        CHECK(SelectObject(dc,sources[i]));
        if(!i){fail_next=1;CHECK(!ntw16s_vb_paint_font(dc));CHECK(live_allocations==baseline && !fonts.count);}
        HFONT lease=ntw16s_vb_paint_font(dc);
        if(i==32){CHECK(!lease && fonts.count==32);continue;}
        leases[i]=lease;CHECK(lease && ntw16s_vb_paint_font(dc)==lease && fonts.count==i+1);
        WORD glyphs[5],omega;WCHAR decoded[5];
        CHECK(GetGlyphIndicesW(dc,L"01234",5,glyphs,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR);
        if(!i){fail_next=1;CHECK(!ntw16s_vb_decode_glyphs(dc,(const WCHAR *)glyphs,5,decoded));}
        CHECK(ntw16s_vb_decode_glyphs(dc,(const WCHAR *)glyphs,5,decoded) && !wmemcmp(decoded,L"01234",5));
        CHECK(GetGlyphIndicesW(dc,L"\x03a9",1,&omega,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR && omega!=0xffff);
        CHECK(!ntw16s_vb_decode_glyphs(dc,(const WCHAR *)&omega,1,decoded));
        CHECK(GetCurrentObject(dc,OBJ_FONT)==sources[i]);
        ntw16s_vb_paint_begin(&nested,barrier,WM_PRINTCLIENT,&barrier_fonts);
        CHECK(!ntw16s_vb_paint_active() && !ntw16s_vb_paint_font(dc));ntw16s_vb_paint_end(&nested);
        CHECK(ntw16s_vb_paint_active());
    }
    ntw16s_vb_paint_end(&scope);CHECK(!ntw16s_vb_paint_active() && !ntw16s_vb_paint_font(dc));
    CHECK(SelectObject(dc,old));ntw16s_vb_fonts_clear(&fonts);CHECK(!fonts.count && live_allocations==baseline);
    for(unsigned i=0;i<32;++i){LOGFONTW lf;CHECK(!GetObjectW(leases[i],sizeof(lf),&lf));}
    for(unsigned i=0;i<33;++i){LOGFONTW lf;CHECK(GetObjectW(sources[i],sizeof(lf),&lf)==sizeof(lf));CHECK(DeleteObject(sources[i]));}
    CHECK(DeleteDC(dc) && DestroyWindow(root));release();CHECK(FreeLibrary(runtime));
    printf("VB_OWNERSHIP_OK leases_retired=32 maps_freed=32 sources_preserved=33 allocations=%u/%u faults=2 barrier=1 capacity=32\n",live_allocations,baseline);
    return 0;
}
static int report_ownership(void)
{
    HWND w=CreateWindowW(L"STATIC",L"report lifetime",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(w);HDC dc=CreateCompatibleDC(NULL),other=CreateCompatibleDC(NULL);CHECK(dc && other);
    HFONT source=CreateFontW(-16,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Arial");CHECK(source);
    HGDIOBJ old=SelectObject(dc,source);ntw16s_vb_fonts fonts={0},barrier={0};ntw16s_vb_paint scope,nested;BOOL cancelled=FALSE;
    unsigned baseline=live_allocations;
    ntw16s_vb_paint_begin(&scope,w,WM_PRINTCLIENT,&fonts);
    ntw16s_report_paint_enable(&scope,w,GetModuleHandleW(L"USER32.dll"),&cancelled);
    CHECK(ntw16s_report_paint_active() && ntw16s_report_paint_caller(GetModuleHandleW(NULL)));
    CHECK(!ntw16s_report_paint_caller(GetModuleHandleW(L"KERNEL32.dll")));
    CHECK(!ntw16s_vb_paint_font(dc));ntw16s_report_paint_bind(NULL,dc);CHECK(!ntw16s_vb_paint_font(dc));
    ntw16s_report_paint_bind(w,dc);fail_next=1;CHECK(!ntw16s_vb_paint_font(dc) && !fonts.count && live_allocations==baseline);
    HFONT font=ntw16s_vb_paint_font(dc);CHECK(font && fonts.count==1 && !ntw16s_vb_paint_font(other));
    HFONT standard=win16_style_vb_font_acquire(source);CHECK(standard==font);win16_style_system_release(standard);
    CHECK(GetCurrentObject(dc,OBJ_FONT)==source);
    ntw16s_vb_paint_begin(&nested,w,WM_PRINTCLIENT,&barrier);CHECK(!ntw16s_vb_paint_font(dc));ntw16s_vb_paint_end(&nested);
    CHECK(ntw16s_vb_paint_font(dc)==font);cancelled=TRUE;
    CHECK(!ntw16s_vb_paint_active() && !ntw16s_vb_paint_font(dc));
    LOGFONTW lf;CHECK(GetObjectW(font,sizeof(lf),&lf)==sizeof(lf));
    ntw16s_vb_paint_end(&scope);ntw16s_vb_fonts_clear(&fonts);CHECK(live_allocations==baseline);
    CHECK(!GetObjectW(font,sizeof(lf),&lf));CHECK(GetObjectW(source,sizeof(lf),&lf)==sizeof(lf));
    SelectObject(dc,old);CHECK(DeleteObject(source) && DeleteDC(other) && DeleteDC(dc) && DestroyWindow(w));
    puts("REPORT_TEXT_OWNERSHIP_OK exact_dc=1 barrier=1 cancelled=1 source_preserved=1 host_system=1 strokes=preserved oom=1 allocations=0");return 0;
}
static int worker(int argc,WCHAR **argv)
{CHECK(argc==2);CHECK(!font_pixels());CHECK(!ownership(argv[1]));CHECK(!report_ownership());CHECK(!live_allocations);return 0;}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
