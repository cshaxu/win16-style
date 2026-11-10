#ifndef NTW16S_TEST_VCL_FRAME_COMPARE_H
#define NTW16S_TEST_VCL_FRAME_COMPARE_H
static int compare(HWND w,windowfn remove){
    RECT outer,client;POINT origin={0};CHECK(GetWindowRect(w,&outer)&&GetClientRect(w,&client)&&ClientToScreen(w,&origin));
    int width=outer.right-outer.left,height=outer.bottom-outer.top;CHECK(width>0&&height>0&&width<1024&&height<1024);
    HDC window=GetWindowDC(w),dc=CreateCompatibleDC(window);CHECK(window&&dc);
    BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    DWORD *bits=NULL;HBITMAP bitmap=CreateDIBSection(window,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(bitmap&&bits);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);
    DWORD *before=malloc((size_t)width*height*sizeof(*before));CHECK(before);
    CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*sizeof(*before));
    CHECK(remove(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,window,0,0,SRCCOPY));GdiFlush();
    unsigned edge=0,content=0;int x0=origin.x-outer.left,y0=origin.y-outer.top;for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(before[y*width+x]!=bits[y*width+x]){if(x>=x0&&x<x0+client.right&&y>=y0&&y<y0+client.bottom)++content;else ++edge;}
    printf("POPUP_PIXELS edge=%u client=%u\n",edge,content);CHECK(edge>0&&content==0);free(before);SelectObject(dc,old);CHECK(DeleteObject(bitmap)&&DeleteDC(dc)&&ReleaseDC(w,window));return 0;
}
#endif
