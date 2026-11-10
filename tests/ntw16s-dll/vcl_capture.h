#ifndef NTW16S_TEST_VCL_CAPTURE_H
#define NTW16S_TEST_VCL_CAPTURE_H
static BOOL capture(HWND w,DWORD *pixels,int width,int height,const RECT *clip,RECT *returned_clip)
{
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO bi={0};void *bits=NULL;
    bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bmp=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,&bits,NULL,0);HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
    if(old){memset(bits,0x5a,(size_t)width*height*4);if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
        SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
        if(returned_clip)GetClipBox(dc,returned_clip);
        memcpy(pixels,bits,(size_t)width*height*4);SelectObject(dc,old);}
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    return old!=NULL;
}
#endif
