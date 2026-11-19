#define COBJMACROS
#include "legacy_image.h"
#include "lib-win32-style/appearance/win3x/resources/frame_buttons.h"
#include "lib-win32-style/appearance/win3x/resources/menu_glyphs.h"
#include <wincodec.h>

HRESULT classic_write_legacy_png(unsigned image_id,PCWSTR path)
{
    if(image_id<CLASSIC_MINIMIZE_IMAGE_ID || image_id>CLASSIC_MENU_ARROW_IMAGE_ID)
        return E_INVALIDARG;
    const BOOL menu=image_id>=CLASSIC_MENU_CHECK_IMAGE_ID;
    const BOOL check=image_id==CLASSIC_MENU_CHECK_IMAGE_ID;
    const BOOL system=image_id>=CLASSIC_SYSTEM_IMAGE_ID;
    const BOOL child=image_id>=CLASSIC_CHILD_SYSTEM_IMAGE_ID;
    const BOOL down=image_id==CLASSIC_SYSTEM_DOWN_IMAGE_ID || image_id==CLASSIC_CHILD_SYSTEM_DOWN_IMAGE_ID;
    const unsigned width=menu?(check?14:7):system?18:19;
    const unsigned height=menu?(check?14:11):18, stride=width*3;
    const BYTE gray[]={0,255,128,192};
    const BYTE pressed_roles[]={1,0,3,2};
    BYTE pixels[18*19*3];
    for(unsigned y=0;y<height;++y){
        const char *row=menu?(check?menu_check[y]:menu_arrow[y]):system?system_menu_bitmap[y]+(child?18:0):
            frame_buttons[image_id-CLASSIC_MINIMIZE_IMAGE_ID][y];
        for(unsigned x=0;x<width;++x){
            BYTE role=(BYTE)(row[x]-'0');
            BYTE value=gray[down?pressed_roles[role]:role];
            for(unsigned band=0;band<3;++band)pixels[y*stride+x*3+band]=value;
        }
    }

    HRESULT result=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(FAILED(result))return result;
    IWICImagingFactory *factory=NULL;
    IWICStream *stream=NULL;
    IWICBitmapEncoder *encoder=NULL;
    IWICBitmapFrameEncode *frame=NULL;
    WICPixelFormatGUID format=GUID_WICPixelFormat24bppBGR;
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
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_WritePixels(frame,height,stride,height*stride,pixels);
    if(SUCCEEDED(result))result=IWICBitmapFrameEncode_Commit(frame);
    if(SUCCEEDED(result))result=IWICBitmapEncoder_Commit(encoder);
    if(frame)IWICBitmapFrameEncode_Release(frame);
    if(encoder)IWICBitmapEncoder_Release(encoder);
    if(stream)IWICStream_Release(stream);
    if(factory)IWICImagingFactory_Release(factory);
    CoUninitialize();
    return result;
}
