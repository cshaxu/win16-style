#define COBJMACROS
#include "classic_theme.h"
#include "lib-win32-style/application/menu/native_system_menu.h"
#include "v4_png.h"

HRESULT style_menu_art_read(HMODULE module,const WORD images[2],style_menu_art *art)
{
    if(!images[0] || !images[1])return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    HRESULT apartment=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return apartment;
    IWICImagingFactory *factory=NULL;
    HRESULT result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,(void **)&factory);
    style_menu_art value={0};
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

void classic_theme_native_menu(style_native_menu *style,const classic_theme_mapping *source)
{
    *style=(style_native_menu){.font=source->model.menu_font,.colors=source->model.menu_popup,
        .metrics=source->model.menu_metrics};
    if(source->resources)
        (void)style_menu_art_read(source->resources,source->model.menu_images,&style->art);
}
