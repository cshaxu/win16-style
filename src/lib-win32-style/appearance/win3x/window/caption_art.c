#define COBJMACROS
#include "caption_art.h"
#include "lib-win32-style/appearance/msstyles/v4_png.h"

HRESULT win16_caption_art_read(HMODULE module,const classic_caption_images *images,win16_caption_art *art)
{
    for(unsigned role=0;role<CLASSIC_IMAGE_ROLE_COUNT;++role)
        if(!images->id[role][0] || !images->id[role][1])
            return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);

    HRESULT apartment=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return apartment;
    IWICImagingFactory *factory=NULL;
    HRESULT result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,(void **)&factory);
    win16_caption_art value={0};
    for(unsigned role=0;SUCCEEDED(result) && role<=CLASSIC_IMAGE_ROLE_COUNT;++role){
        const UINT width=role>=CLASSIC_IMAGE_SYSTEM?WIN16_SYSTEM_ART_WIDTH:WIN16_CAPTION_ART_WIDTH;
        const BOOL child=role==CLASSIC_IMAGE_ROLE_COUNT;
        for(unsigned state=0;SUCCEEDED(result) && state<2;++state){
            PCWSTR name=child?(state?L"CLASSIC_CHILD_SYSTEM_PRESSED":L"CLASSIC_CHILD_SYSTEM"):
                MAKEINTRESOURCEW(images->id[role][state]);
            result=msstyles_png_roles(factory,module,name,width,WIN16_CAPTION_ART_HEIGHT,value.roles[role][state]);
        }
    }
    if(factory)IWICImagingFactory_Release(factory);
    if(SUCCEEDED(apartment))CoUninitialize();
    if(SUCCEEDED(result)){
        value.available=TRUE;
        *art=value;
    }
    return result;
}

HRESULT win16_menu_art_read(HMODULE module,const WORD images[2],win16_menu_art *art)
{
    if(!images[0] || !images[1])return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    HRESULT apartment=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return apartment;
    IWICImagingFactory *factory=NULL;
    HRESULT result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,(void **)&factory);
    win16_menu_art value={0};
    CHAR *const rows[]={&value.check[0][0],&value.arrow[0][0]};
    const UINT widths[]={14,7},heights[]={14,11};
    for(unsigned i=0;SUCCEEDED(result) && i<ARRAYSIZE(rows);++i){
        BYTE roles[14*14];
        result=msstyles_png_roles(factory,module,MAKEINTRESOURCEW(images[i]),widths[i],heights[i],roles);
        for(UINT y=0;SUCCEEDED(result) && y<heights[i];++y)
            for(UINT x=0;x<widths[i];++x){
                BYTE role=roles[y*widths[i]+x];
                if(role>1){result=WINCODEC_ERR_BADIMAGE;break;}
                rows[i][y*(widths[i]+1)+x]=(CHAR)('0'+role);
            }
    }
    if(factory)IWICImagingFactory_Release(factory);
    if(SUCCEEDED(apartment))CoUninitialize();
    if(SUCCEEDED(result)){
        value.available=TRUE;
        *art=value;
    }
    return result;
}
