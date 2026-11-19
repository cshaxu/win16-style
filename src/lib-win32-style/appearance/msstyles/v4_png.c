#define COBJMACROS
#include "v4_png.h"
#include <string.h>

HRESULT msstyles_png_roles(IWICImagingFactory *factory,HMODULE module,PCWSTR name,
    UINT width,UINT height,BYTE *roles)
{
    if(!width || !height || width>19 || height>18)return E_INVALIDARG;
    const BYTE signature[]={137,'P','N','G',13,10,26,10};
    HRSRC item=FindResourceW(module,name,L"IMAGE");
    HGLOBAL handle=item?LoadResource(module,item):NULL;
    const BYTE *source=handle?LockResource(handle):NULL;
    DWORD size=source?SizeofResource(module,item):0;
    if(!source || size<sizeof(signature) || memcmp(source,signature,sizeof(signature)))
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    IWICStream *stream=NULL;
    IWICBitmapDecoder *decoder=NULL;
    IWICBitmapFrameDecode *frame=NULL;
    IWICFormatConverter *converter=NULL;
    UINT actual_width,actual_height;
    BYTE pixels[19*18*4];
    HRESULT result=IWICImagingFactory_CreateStream(factory,&stream);
    if(SUCCEEDED(result))result=IWICStream_InitializeFromMemory(stream,(BYTE *)source,size);
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateDecoder(factory,&GUID_ContainerFormatPng,&GUID_VendorMicrosoft,&decoder);
    if(SUCCEEDED(result))result=IWICBitmapDecoder_Initialize(decoder,(IStream *)stream,WICDecodeMetadataCacheOnDemand);
    if(SUCCEEDED(result))result=IWICBitmapDecoder_GetFrame(decoder,0,&frame);
    if(SUCCEEDED(result))result=IWICBitmapFrameDecode_GetSize(frame,&actual_width,&actual_height);
    if(SUCCEEDED(result) && (actual_width!=width || actual_height!=height))
        result=WINCODEC_ERR_BADIMAGE;
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateFormatConverter(factory,&converter);
    if(SUCCEEDED(result))result=IWICFormatConverter_Initialize(converter,(IWICBitmapSource *)frame,
        &GUID_WICPixelFormat32bppBGRA,WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom);
    if(SUCCEEDED(result))result=IWICFormatConverter_CopyPixels(converter,NULL,width*4,sizeof(pixels),pixels);
    if(converter)IWICFormatConverter_Release(converter);
    if(frame)IWICBitmapFrameDecode_Release(frame);
    if(decoder)IWICBitmapDecoder_Release(decoder);
    if(stream)IWICStream_Release(stream);
    if(FAILED(result))return result;

    const BYTE gray[]={0,255,128,192};
    for(UINT i=0;i<width*height;++i){
        const BYTE *pixel=pixels+i*4;
        const BYTE *role=memchr(gray,pixel[0],sizeof(gray));
        if(!role || pixel[0]!=pixel[1] || pixel[1]!=pixel[2] || pixel[3]!=255)
            return WINCODEC_ERR_BADIMAGE;
        roles[i]=(BYTE)(role-gray);
    }
    return S_OK;
}
