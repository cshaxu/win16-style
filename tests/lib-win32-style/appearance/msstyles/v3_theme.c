#include "lib-win32-style/appearance/msstyles/v3_theme.h"
#include "lib-win32-style/appearance/msstyles/theme_file.h"
#include "lib-win32-style/appearance/msstyles/v3_frame.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"V3 THEME FAIL line=%d\n",__LINE__);return 1;}}while(0)

static BOOL value_is(msstyles_v3_text value,PCWSTR expected)
{return value.length==wcslen(expected) && !wmemcmp(value.data,expected,value.length);}

static BOOL check_properties(void)
{
    static const WCHAR text[]=L"\xfeff; comment\r\n[Window.Caption]\r\n  ImageCount = 2 ; item\r\n"
        L"[Window.Caption(active)]\r\nTextColor = 255 255 255\r\n[Globals] ; default\r\n";
    msstyles_v3_theme view={.properties={text,ARRAYSIZE(text)-1}};
    msstyles_v3_text value;
    return msstyles_v3_property(&view,L"window.caption",L"IMAGECOUNT",&value) && value_is(value,L"2") &&
        msstyles_v3_property(&view,L"Window.Caption(active)",L"TextColor",&value) &&
        value_is(value,L"255 255 255") &&
        !msstyles_v3_property(&view,L"Window.Caption",L"TextColor",&value) &&
        value_is(value,L"255 255 255") && !msstyles_v3_property(&view,L"Globals",L"TextColor",&value);
}

static BOOL check_caption_glyphs(const msstyles_v3_theme *view)
{
    static const enum classic_button_role roles[]={
        CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_CLOSE
    };
    style_frame_sprites sprites;
    if(!msstyles_v3_frame_images(view,&sprites))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(roles);++i){
        const style_frame_sprite_button *button=&sprites.buttons[roles[i]];
        if(button->selection!=STYLE_GLYPH_SIZE || button->face.width!=21 ||
           style_caption_glyph(button,(SIZE){17,17},96)!=&button->variants[2].image ||
           style_caption_glyph(button,(SIZE){17,17},144)!=&button->variants[2].image ||
           style_caption_glyph(button,(SIZE){22,23},144)!=&button->variants[3].image ||
           style_caption_glyph(button,(SIZE){26,26},144)!=&button->variants[4].image){
            fprintf(stderr,"glyph role=%u mode=%u face=%u variant widths=%u/%u/%u/%u/%u\n",
                (unsigned)roles[i],(unsigned)button->selection,button->face.width,
                button->variants[0].image.width,button->variants[1].image.width,
                button->variants[2].image.width,button->variants[3].image.width,
                button->variants[4].image.width);
            return FALSE;
        }
        if(button->variants[2].image.width!=13 || button->variants[3].image.width!=19 ||
           button->variants[4].image.width!=23)return FALSE;
    }
    return TRUE;
}

static BOOL check_indexed_strip(void)
{
    const struct {
        BITMAPINFOHEADER header;
        RGBQUAD palette[4];
        BYTE row[4];
    } source={
        .header={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=6,.biHeight=1,
            .biPlanes=1,.biBitCount=4,.biClrUsed=4},
        .palette={{0,0,0,0},{0,0,255,0},{0,255,0,0},{255,0,0,0}},
        .row={0x01,0x23,0x21,0}
    };
    style_dib_strip strip;
    if(!style_dib_strip_read(&source,sizeof(source),2,TRUE,&strip))return FALSE;
    BITMAPINFO format={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=3,.biHeight=1,
        .biPlanes=1,.biBitCount=32}};
    void *pixels=NULL;
    HDC dc=CreateCompatibleDC(NULL);
    HBITMAP bitmap=CreateDIBSection(dc,&format,DIB_RGB_COLORS,&pixels,NULL,0);
    HGDIOBJ before=dc && bitmap?SelectObject(dc,bitmap):NULL;
    BOOL ok=before && before!=HGDI_ERROR &&
        style_dib_strip_draw(dc,(RECT){0,0,3,1},&strip,1,96) &&
        GetPixel(dc,0,0)==RGB(0,0,255) && GetPixel(dc,1,0)==RGB(0,255,0) &&
        GetPixel(dc,2,0)==RGB(255,0,0);
    if(before && before!=HGDI_ERROR)SelectObject(dc,before);
    if(bitmap)DeleteObject(bitmap);
    if(dc)DeleteDC(dc);
    return ok;
}

int wmain(int argc,WCHAR **argv)
{
    CHECK(check_properties());
    CHECK(check_indexed_strip());
    if(argc==1){puts("V3 PROPERTY PASS exact section, whitespace and comments");return 0;}
    CHECK(argc==4);
    FILE *description=_wfopen(argv[1],L"rb");
    CHECK(description);
    msstyles_theme_descriptor parsed={0};
    BOOL read=msstyles_theme_file_read(description,&parsed);
    CHECK(!fclose(description) && read && !strcmp(parsed.file,"luna.msstyles") &&
        !*parsed.display_name && !parsed.hosts.declared);
    HMODULE module=LoadLibraryExW(argv[2],NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    WCHAR chosen_size[MSSTYLES_THEME_TOKEN_CAPACITY],chosen_color[MSSTYLES_THEME_TOKEN_CAPACITY];
    CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,parsed.size,-1,chosen_size,ARRAYSIZE(chosen_size)));
    CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,parsed.color,-1,chosen_color,ARRAYSIZE(chosen_color)));
    msstyles_v3_theme chosen={0};
    CHECK(msstyles_v3_read(module,chosen_size,chosen_color,&chosen)==S_OK);
    wprintf(L"V3 DESCRIPTOR %ls %ls %ls\n",chosen_size,chosen_color,chosen.variant_name);
    static const PCWSTR colors[]={L"NormalColor",L"Metallic",L"HomeStead"};
    static const PCWSTR color_names[]={L"Default (blue)",L"Silver",L"Olive Green"};
    for(size_t i=0;i<ARRAYSIZE(colors);++i){
        PCWSTR token;
        msstyles_v3_text label;
        CHECK(msstyles_v3_color_at(&chosen,i,&token,&label)==S_OK);
        CHECK(!wcscmp(token,colors[i]) && value_is(label,color_names[i]));
    }
    PCWSTR token;
    msstyles_v3_text label;
    CHECK(msstyles_v3_color_at(&chosen,ARRAYSIZE(colors),&token,&label)==S_FALSE);
    puts("V3 COLOR NAMES PASS Default (blue), Silver, Olive Green");
    static const PCWSTR sizes[]={L"NormalSize",L"LargeFonts",L"ExtraLargeFonts"};
    static const PCWSTR names[][3]={
        {L"NORMALBLUE_INI",L"LARGEFONTSBLUE_INI",L"EXTRALARGEBLUE_INI"},
        {L"NORMALMETALLIC_INI",L"LARGEFONTSMETALLIC_INI",L"EXTRALARGEMETALLIC_INI"},
        {L"NORMALHOMESTEAD_INI",L"LARGEFONTSHOMESTEAD_INI",L"EXTRALARGEHOMESTEAD_INI"}
    };
    for(size_t c=0;c<ARRAYSIZE(colors);++c){
        for(size_t s=0;s<ARRAYSIZE(sizes);++s){
            msstyles_v3_theme view={0};
            CHECK(msstyles_v3_read(module,sizes[s],colors[c],&view)==S_OK);
            CHECK(!wcscmp(view.variant_name,names[c][s]));
            CHECK(check_caption_glyphs(&view));
            msstyles_v3_text prop;
            const void *dib=NULL;
            DWORD length=0;
            CHECK(msstyles_v3_property(&view,L"Window.Caption",L"ImageFile",&prop));
            CHECK(msstyles_v3_bitmap(&view,prop,&dib,&length) && length>sizeof(BITMAPINFOHEADER));
            BITMAPINFOHEADER header;
            memcpy(&header,dib,sizeof(header));
            CHECK(header.biSize>=sizeof(header) && header.biWidth>0 && header.biHeight>0);
            CHECK(msstyles_v3_property(&view,L"Window.Caption",L"ImageCount",&prop) && value_is(prop,L"2"));
            CHECK(msstyles_v3_property(&view,L"Button.PushButton",L"ImageFile",&prop));
            CHECK(msstyles_v3_bitmap(&view,prop,&dib,&length) && length>sizeof(BITMAPINFOHEADER));
            wprintf(L"V3 VARIANT %ls %ls %ls caption=%ldx%ld\n",sizes[s],colors[c],
                view.variant_name,header.biWidth,header.biHeight);
        }
    }
    msstyles_v3_theme view={0};
    CHECK(msstyles_v3_read(module,sizes[0],L"UnknownColor",&view)==HRESULT_FROM_WIN32(ERROR_NOT_FOUND) && !view.resources);
    CHECK(msstyles_v3_read(module,L"UnknownSize",colors[0],&view)==HRESULT_FROM_WIN32(ERROR_NOT_FOUND) && !view.resources);
    CHECK(FreeLibrary(module));
    module=LoadLibraryExW(argv[3],NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module && msstyles_v3_read(module,sizes[0],colors[0],&view)==HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));
    CHECK(FreeLibrary(module) && !view.resources);
    printf("V3 LUNA PASS bits=%u variants=9 images=18 invalid_selection=2 rejects_v4=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
