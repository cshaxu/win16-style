#define COBJMACROS
#include "legacy_image.h"
#include "lib-win32-style/appearance/win3x/resources/frame_buttons.h"
#include <wincodec.h>

HRESULT classic_write_minimize_png(PCWSTR path)
{
    HRESULT result=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(result))return result;
    IWICImagingFactory *factory=NULL;
    IWICStream *stream=NULL;
    IWICBitmapEncoder *encoder=NULL;
    IWICBitmapFrameEncode *frame=NULL;
    WICPixelFormatGUID format=GUID_WICPixelFormat24bppBGR;
    const BYTE gray[]={0,255,128,192};
    enum {width=19,height=18,stride=width*3};
    BYTE pixels[height][stride];
    for(unsigned y=0;y<height;++y)
        for(unsigned x=0;x<width;++x)
            for(unsigned band=0;band<3;++band)
                pixels[y][x*3+band]=gray[frame_buttons[0][y][x]-'0'];

    result=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
                            &IID_IWICImagingFactory,(void **)&factory);
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateStream(factory,&stream);
    if(SUCCEEDED(result))result=IWICStream_InitializeFromFilename(stream,path,GENERIC_WRITE);
    if(SUCCEEDED(result))result=IWICImagingFactory_CreateEncoder(factory,&GUID_ContainerFormatPng,NULL,&encoder);
    if(SUCCEEDED(result))result=IWICBitmapEncoder_Initialize(encoder,(IStream *)stream,WICBitmapEncoderNoCache);
    if(SUCCEEDED(result))result=IWICBitmapEncoder_CreateNewFrame(encoder,&frame,NULL);
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_Initialize(frame,NULL);
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_SetSize(frame,width,height);
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_SetPixelFormat(frame,&format);
    if(SUCCEEDED(result) && !IsEqualGUID(&format,&GUID_WICPixelFormat24bppBGR))result=WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_WritePixels(frame,height,stride,sizeof(pixels),pixels[0]);
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_Commit(frame);
    if(SUCCEEDED(result))result=IWICBitmapEncoder_Commit(encoder);
    if(frame)IWICBitmapFrameEncode_Release(frame);
    if(encoder)IWICBitmapEncoder_Release(encoder);
    if(stream)IWICStream_Release(stream);
    if(factory)IWICImagingFactory_Release(factory);
    CoUninitialize();
    return result;
}
