#define COBJMACROS
#include "lib-win32-style/application/graphics/pixel_grid.h"
#include "lib-win32-style/application/graphics/pixel_dib.h"
#include "lib-win32-style/appearance/msstyles/v4_png.h"
#include "choice_paint.h"
#include "../resources/control_glyphs.h"

BOOL win16_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    int column=((flags&DFCS_CHECKED)?1:0)+((flags&DFCS_PUSHED)?2:0);
    const DWORD colors[2]={0,0x00ffffff};
    return style_paint_indexed_dib(dc,rect,&control_checks[0][column*14],57,(SIZE){14,13},colors);
}
const char *win16_choice_art_get(HMODULE module,WORD image,win16_choice_art *art)
{
    if(!module || !image || !art)return NULL;
    if(art->image==image)return art->available?art->cell:NULL;
    HRESULT apartment=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return NULL;
    IWICImagingFactory *factory=NULL;
    HRESULT result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,(void **)&factory);
    BYTE roles[14*13];
    if(SUCCEEDED(result))result=msstyles_png_roles(factory,module,MAKEINTRESOURCEW(image),14,13,roles);
    if(factory)IWICImagingFactory_Release(factory);
    if(SUCCEEDED(apartment))CoUninitialize();
    win16_choice_art value={.image=image};
    for(unsigned i=0;SUCCEEDED(result) && i<ARRAYSIZE(roles);++i){
        if(roles[i]>1){result=WINCODEC_ERR_BADIMAGE;break;}
        value.cell[i]=(char)('0'+roles[i]);
    }
    value.available=SUCCEEDED(result);
    *art=value;
    return art->available?art->cell:NULL;
}

void win16_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,int row,int column,const win16_style_palette *palette,
    const char *image)
{
    const COLORREF colors[2]={palette->edge,palette->light};
    const style_pixel_grid grid={image?image:&control_checks[row*13][column*14],14,13,image?14:57,
        -1,colors,{glyph.left,glyph.top},{(int)dpi,(int)dpi},{96,96}};
    style_paint_pixel_grid(dc,&grid);
}
