#ifndef NTW16S_TEST_CONTROL_CAPTURE_H
#define NTW16S_TEST_CONTROL_CAPTURE_H
typedef struct capture_surface {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *bits;} capture_surface;
static capture_surface surface;
static void capture_end(void)
{
    if(!surface.dc)return;
    if(!SelectObject(surface.dc,surface.old) || !DeleteObject(surface.bitmap) || !DeleteDC(surface.dc)){
        fprintf(stderr,CAPTURE_LABEL "_CAPTURE_CLEANUP_FAIL error=%lu\n",GetLastError());ExitProcess(1);
    }
    ZeroMemory(&surface,sizeof(surface));GdiFlush();
}
static DWORD image(HWND w,DWORD *pixels,UINT count)
{
    if(!surface.dc){
        BITMAPINFO info={0};HDC dc=GetDC(w);surface.dc=CreateCompatibleDC(dc);ReleaseDC(w,dc);
        info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=200;info.bmiHeader.biHeight=-100;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
        surface.bitmap=CreateDIBSection(surface.dc,&info,DIB_RGB_COLORS,(void **)&surface.bits,NULL,0);
        if(!surface.dc || !surface.bitmap)ExitProcess(1);
        surface.old=SelectObject(surface.dc,surface.bitmap);
    }
    int saved=SaveDC(surface.dc);if(!saved)ExitProcess(1);
    RECT clear={0,0,200,100};FillRect(surface.dc,&clear,GetStockObject(WHITE_BRUSH));
    WCHAR cls[64];GetClassNameW(w,cls,64);
    if(capture_full(cls))SendMessageW(w,WM_PRINT,(WPARAM)surface.dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    else SendMessageW(w,WM_PRINTCLIENT,(WPARAM)surface.dc,PRF_CLIENT);
    GdiFlush();DWORD hash=2166136261u;
    for(UINT i=0;i<200*100;++i)hash=(hash^surface.bits[i])*16777619u;
    if(pixels && count==200*100)CopyMemory(pixels,surface.bits,count*sizeof(DWORD));
    if(!RestoreDC(surface.dc,saved))ExitProcess(1);
    return hash;
}

#endif
