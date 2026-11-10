#include <windows.h>
#include <stdio.h>
#include <string.h>

typedef BOOL (WINAPI *draw_fn)(HDC,RECT *,UINT);
int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 1;
    HMODULE module=LoadLibraryW(argv[1]);if(!module)return 2;
    draw_fn draw=(draw_fn)(void *)GetProcAddress(module,"Ntw2ksDrawButton");if(!draw)return 3;
    if(draw(NULL,NULL,0))return 4;
    HDC dc[2]={CreateCompatibleDC(NULL),CreateCompatibleDC(NULL)};
    BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=32;bi.bmiHeader.biHeight=-32;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    void *pixels[2]={0};HBITMAP bmp[2]={0};HGDIOBJ old[2]={0};
    for(unsigned i=0;i<2;++i){bmp[i]=CreateDIBSection(dc[i],&bi,DIB_RGB_COLORS,&pixels[i],NULL,0);if(!dc[i] || !bmp[i] || !pixels[i])return 5;old[i]=SelectObject(dc[i],bmp[i]);if(!old[i])return 6;}
    const UINT states[]={0,DFCS_PUSHED,DFCS_INACTIVE,DFCS_CHECKED,DFCS_FLAT};
    for(unsigned i=0;i<ARRAYSIZE(states);++i){RECT a={1,1,31,31},b=a;memset(pixels[0],0x5a,32*32*4);memset(pixels[1],0x5a,32*32*4);
        if(!draw(dc[0],&a,states[i]) || !DrawFrameControl(dc[1],&b,DFC_BUTTON,DFCS_BUTTONPUSH|states[i]))return 7;
        GdiFlush();if(memcmp(pixels[0],pixels[1],32*32*4) || memcmp(&a,&b,sizeof(a)))return 8;}
    for(unsigned i=0;i<2;++i){SelectObject(dc[i],old[i]);DeleteObject(bmp[i]);DeleteDC(dc[i]);}
    FreeLibrary(module);printf("NTW2KS_BUTTON_OK bits=%u states=5 native_equivalent=1\n",(unsigned)(8*sizeof(void *)));return 0;
}
