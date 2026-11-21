#define COBJMACROS
#include <windows.h>
#include <wincodec.h>
#include <vsstyle.h>
#include "lib-win32-style/appearance/win3x/resources/frame_buttons.h"
#include "lib-win32-style/appearance/win3x/resources/menu_glyphs.h"
#include "lib-win32-style/appearance/win3x/resources/control_glyphs.h"
#include "lib-win32-style/appearance/win3x/resources/combo_glyph.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return false;}}while(0)

static const unsigned char *resource(HMODULE module,PCWSTR type,PCWSTR name,DWORD *size)
{
    HRSRC item=FindResourceW(module,name,type);
    if(!item)return NULL;
    *size=SizeofResource(module,item);
    HGLOBAL handle=LoadResource(module,item);
    return handle?LockResource(handle):NULL;
}

static bool field(PCWSTR file,PCWSTR section,PCWSTR key,PCWSTR expected)
{
    WCHAR value[100];
    return GetPrivateProfileStringW(section,key,L"",value,ARRAYSIZE(value),file) &&
        !wcscmp(value,expected);
}

static bool variant_fields(HMODULE module,PCWSTR theme)
{
    DWORD size=0;
    const unsigned char *data=resource(module,L"VMAP",L"VMAP",&size);
    const PCWSTR keys[]={NULL,L"Size",L"ColorStyle"};
    CHECK(data);
    for(size_t i=0;i<ARRAYSIZE(keys);++i){
        uint32_t units;
        WCHAR value[64];
        CHECK(size>=sizeof(units));
        memcpy(&units,data,sizeof(units));
        size-=sizeof(units);
        data+=sizeof(units);
        CHECK(units>1 && units<=ARRAYSIZE(value));
        size_t length=units*sizeof(WCHAR),span=(length+3u)&~3u;
        CHECK(span<=size);
        memcpy(value,data,length);
        CHECK(value[units-1]==0 && wcslen(value)==units-1);
        if(keys[i])CHECK(field(theme,L"VisualStyles",keys[i],value));
        else CHECK(!_wcsicmp(value,L"Normal"));
        size-=span;
        data+=span;
    }
    return size==0;
}

static bool original_image(HMODULE module,uint32_t image_id)
{
    DWORD length=0;
    CHECK(image_id && image_id<=UINT16_MAX);
    PCWSTR name=image_id==509?L"CLASSIC_CHILD_SYSTEM":
        image_id==510?L"CLASSIC_CHILD_SYSTEM_PRESSED":MAKEINTRESOURCEW(image_id);
    const unsigned char *bytes=resource(module,L"IMAGE",name,&length);
    CHECK(bytes && length>8 && !memcmp(bytes,"\x89PNG\r\n\x1a\n",8));
    IWICImagingFactory *factory=NULL;
    IWICStream *stream=NULL;
    IWICBitmapDecoder *decoder=NULL;
    IWICBitmapFrameDecode *frame=NULL;
    IWICFormatConverter *converter=NULL;
    bool system=image_id>=507,child=image_id>=509,menu=image_id>=511,check=image_id==511;
    bool choice=image_id>=513 && image_id<=524;
    unsigned cell=choice?image_id-513:0;
    const UINT width=choice?14:menu?(check?14:7):system?18:19;
    const UINT height=choice?13:menu?(check?14:11):18,stride=width*3;
    BYTE bits[18*19*3];
    UINT x=0,y=0;
    HRESULT result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
                                    &IID_IWICImagingFactory,(void **)&factory);
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateStream(factory,&stream);
    if(SUCCEEDED(result))result=IWICStream_InitializeFromMemory(stream,(BYTE *)bytes,length);
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateDecoderFromStream(factory,(IStream *)stream,NULL,
                                        WICDecodeMetadataCacheOnDemand,&decoder);
    if(SUCCEEDED(result))result=IWICBitmapDecoder_GetFrame(decoder,0,&frame);
    if(SUCCEEDED(result))result=IWICBitmapFrameDecode_GetSize(frame,&x,&y);
    if(SUCCEEDED(result) && (x!=width || y!=height))result=E_FAIL;
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateFormatConverter(factory,&converter);
    if(SUCCEEDED(result))result=IWICFormatConverter_Initialize(converter,(IWICBitmapSource *)frame,
        &GUID_WICPixelFormat24bppBGR,WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom);
    if(SUCCEEDED(result))result=IWICFormatConverter_CopyPixels(converter,NULL,stride,sizeof(bits),bits);
    if(converter)IWICFormatConverter_Release(converter);
    if(frame)IWICBitmapFrameDecode_Release(frame);
    if(decoder)IWICBitmapDecoder_Release(decoder);
    if(stream)IWICStream_Release(stream);
    if(factory)IWICImagingFactory_Release(factory);
    CHECK(SUCCEEDED(result));
    const unsigned char gray[]={0,255,128,192};
    bool inverted=image_id==508 || image_id==510;
    const unsigned pressed[]={1,0,3,2};
    for(y=0;y<height;++y){
        const char *row=choice?control_checks[(cell/4)*13+y]+(cell%4)*14:menu?(check?menu_check[y]:menu_arrow[y]):
            system?system_menu_bitmap[y]+(child?18:0):frame_buttons[image_id-501][y];
        for(x=0;x<width;++x){
            unsigned role=(unsigned)(row[x]-'0');
            for(unsigned band=0;band<3;++band)
                CHECK(bits[y*stride+x*3+band]==gray[inverted?pressed[role]:role]);
        }
    }
    return true;
}

static bool style_data(HMODULE module,uint32_t caption,uint32_t face,bool expect_image)
{
    DWORD size=0;
    const unsigned char *data=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1),&size);
    CHECK(data && size==2 && data[0]==4 && data[1]==0);
    data=resource(module,L"CMAP",L"CMAP",&size);
    CHECK(data && size>=2 && !(size%2) && !(data[size-1]|data[size-2]));
    unsigned indices[3]={0},classes=0,metrics=0,menu=0;
    WCHAR value[48];
    size_t units=0;
    for(size_t i=0;i<size;i+=2){
        WCHAR unit;
        memcpy(&unit,data+i,2);
        if(unit){CHECK(units+1<ARRAYSIZE(value));value[units++]=unit;}
        else if(units){
            value[units]=0;
            ++classes;
            if(!wcscmp(value,L"Button"))indices[0]=classes;
            if(!wcscmp(value,L"ScrollBar"))indices[1]=classes;
            if(!wcscmp(value,L"Window"))indices[2]=classes;
            if(!wcscmp(value,L"sysmetrics"))metrics=classes;
            if(!wcscmp(value,L"Menu"))menu=classes;
            units=0;
        }
    }
    CHECK(indices[0] && indices[1] && indices[2] && metrics && menu);
    const uint32_t values[]={face,face,caption};
    unsigned observed=0;
    const struct {unsigned part,state,image;} image_props[]={
        {15,0,501},{17,0,502},{21,0,503},{15,3,504},
        {17,3,505},{21,3,506},{13,0,507},{13,3,508}
    };
    unsigned found_images=0,found_fonts=0,found_menu=0,found_choice=0;
    data=resource(module,L"VARIANT",L"NORMAL",&size);
    CHECK(data);
    while(size){
        uint32_t item[8],color;
        CHECK(size>=sizeof(item));
        memcpy(item,data,sizeof(item));
        size-=sizeof(item);
        data+=sizeof(item);
        CHECK(item[2]<classes && !item[6]);
        if(item[5]){
            if(item[1]==210){
                CHECK(item[0]>=801 && item[0]<=803 && item[2]+1==metrics &&
                    !item[3] && !item[4] && item[7]==sizeof(LOGFONTW));
                unsigned role=item[0]-801;
                const PCWSTR win2k_fonts[]={L"Tahoma, 8, Bold",L"Tahoma, 6, Bold",L"Tahoma, 8"};
                const PCWSTR win16_fonts[]={L"MS Sans Serif, 10, Bold, Quality:NonAntialiased",NULL,
                    L"System, 12, Bold, Quality:NonAntialiased"};
                PCWSTR expected=expect_image?win16_fonts[role]:win2k_fonts[role];
                WCHAR font[80];
                CHECK(!(found_fonts&(1u<<role)) && expected &&
                    LoadStringW(module,item[5],font,ARRAYSIZE(font)) && !wcscmp(font,expected));
                found_fonts|=1u<<role;
                continue;
            }
            CHECK(expect_image && item[0]==3001 && item[1]==206 && item[7]==16);
            if(item[2]+1==indices[0]){
                bool radio=item[3]==BP_RADIOBUTTON;
                CHECK((radio || item[3]==BP_CHECKBOX) && item[4] && item[4]<=(radio?8:12));
                unsigned state=item[4]-1;
                unsigned bit=1u<<(state+(radio?12:0));
                unsigned row=state>=8?2:radio;
                unsigned column=(state>=4)+2*(state%4==2);
                CHECK(!(found_choice&bit) && item[5]==513+row*4+column);
                found_choice|=bit;
                continue;
            }
            if(item[2]+1==menu){
                CHECK((item[3]==MENU_POPUPCHECK || item[3]==MENU_POPUPSUBMENU) && !item[4]);
                unsigned role=item[3]==MENU_POPUPSUBMENU;
                CHECK(!(found_menu&(1u<<role)) && item[5]==511+role && original_image(module,item[5]));
                found_menu|=1u<<role;
                continue;
            }
            CHECK(item[2]+1==indices[2]);
            size_t j=0;
            while(j<ARRAYSIZE(image_props) && (item[3]!=image_props[j].part ||
                item[4]!=image_props[j].state || item[5]!=image_props[j].image))++j;
            CHECK(j<ARRAYSIZE(image_props) && !(found_images&(1u<<j)));
            CHECK(original_image(module,item[5]));
            found_images|=1u<<j;
            continue;
        }
        CHECK(item[7]<=size);
        size_t length=item[7],pad=(8-length%8)%8;
        CHECK(pad<=size-length);
        for(unsigned i=0;i<ARRAYSIZE(indices);++i){
            if(item[0]!=3802 || item[2]+1!=indices[i] || item[3]!=1 || item[4]!=(i==2))continue;
            CHECK(item[1]==204 && length==sizeof(color) && !(observed&(1u<<i)));
            memcpy(&color,data,sizeof(color));
            CHECK(color==values[i]);
            observed|=1u<<i;
        }
        size-=length+pad;
        data+=length+pad;
    }
    CHECK(observed==7);
    CHECK(found_fonts==(expect_image?5:7));
    CHECK(found_images==(expect_image?(1u<<ARRAYSIZE(image_props))-1:0));
    CHECK(found_menu==(expect_image?3:0));
    CHECK(found_choice==(expect_image?0xfffff:0));
    if(expect_image)CHECK(original_image(module,509) && original_image(module,510));
    if(expect_image)for(unsigned id=513;id<=524;++id)CHECK(original_image(module,id));
    return true;
}

static bool combo_resource(HMODULE module,bool expected)
{
    DWORD length=0;
    const unsigned char *data=resource(module,MAKEINTRESOURCEW(10),L"CLASSIC_COMBO_ARROW",&length);
    if(!expected)return data==NULL;
    const unsigned char header[]={'C','A','R',1};
    CHECK(data && length==sizeof(header)+ARRAYSIZE(combo_arrow) && !memcmp(data,header,sizeof(header)));
    for(unsigned y=0;y<ARRAYSIZE(combo_arrow);++y){
        unsigned char mask=0;
        for(unsigned x=0;x<sizeof(combo_arrow[y])-1;++x)
            if(combo_arrow[y][x]=='0')mask|=(unsigned char)(1u<<x);
        CHECK(data[sizeof(header)+y]==mask);
    }
    return true;
}

static bool check_pair(PCWSTR root,PCWSTR stem,PCWSTR label,uint32_t caption,uint32_t face,bool expect_image)
{
    WCHAR descriptor[MAX_PATH],carrier[MAX_PATH],filename[48];
    CHECK(swprintf(descriptor,ARRAYSIZE(descriptor),L"%ls/%ls/%ls.theme",root,stem,stem)>0);
    CHECK(swprintf(filename,ARRAYSIZE(filename),L"%ls.msstyles",stem)>0);
    CHECK(swprintf(carrier,ARRAYSIZE(carrier),L"%ls/%ls/%ls",root,stem,filename)>0);
    CHECK(field(descriptor,L"Theme",L"DisplayName",label));
    CHECK(field(descriptor,L"Control Panel\\Desktop",L"TileWallpaper",L"0"));
    CHECK(field(descriptor,L"VisualStyles",L"Path",filename));
    CHECK(field(descriptor,L"MasterThemeSelector",L"MTSM",L"DABJDKT"));
    HMODULE module=LoadLibraryExW(carrier,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    bool valid=variant_fields(module,descriptor) && style_data(module,caption,face,expect_image) && combo_resource(module,expect_image);
    FreeLibrary(module);
    return valid;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    if(FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)))return 1;
    bool passed=check_pair(argv[1],L"Win16",L"Windows 3.x",RGB(164,200,240),RGB(192,192,192),true) &&
        check_pair(argv[1],L"Win2K",L"Windows 2000",RGB(0,0,128),RGB(212,208,200),false);
    CoUninitialize();
    if(!passed)return 1;
    puts("Classic themes: both companion descriptions and v4 resources passed");
    return 0;
}
