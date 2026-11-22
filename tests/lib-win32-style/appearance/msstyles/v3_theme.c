#include "lib-win32-style/appearance/msstyles/v3_theme.h"
#include "lib-win32-style/appearance/msstyles/theme_file.h"
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

int wmain(int argc,WCHAR **argv)
{
    CHECK(check_properties());
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