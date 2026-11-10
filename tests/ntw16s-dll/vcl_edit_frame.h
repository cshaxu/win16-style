#ifndef NTW16S_TEST_VCL_EDIT_FRAME_H
#define NTW16S_TEST_VCL_EDIT_FRAME_H
static int frame_pixels(HWND w,window_fn remove)
{
 RECT wr,cr;POINT pt={0};CHECK(GetWindowRect(w,&wr)&&GetClientRect(w,&cr)&&ClientToScreen(w,&pt));
 int width=wr.right-wr.left,height=wr.bottom-wr.top,x0=pt.x-wr.left,y0=pt.y-wr.top;
 HDC src=GetWindowDC(w),dc=CreateCompatibleDC(src);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;void *bits=NULL;
 HBITMAP bmp=CreateDIBSection(src,&bi,DIB_RGB_COLORS,&bits,NULL,0);CHECK(src&&dc&&bmp&&bits);HGDIOBJ old=SelectObject(dc,bmp);CHECK(old);DWORD *before=malloc((size_t)width*height*4);CHECK(before);
 CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,src,0,0,SRCCOPY));GdiFlush();memcpy(before,bits,(size_t)width*height*4);
 CHECK(remove(w));CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));CHECK(BitBlt(dc,0,0,width,height,src,0,0,SRCCOPY));GdiFlush();
 unsigned edge=0,client=0,outer=0;DWORD *after=bits;for(int y=0;y<height;y++)for(int x=0;x<width;x++)if(before[y*width+x]!=after[y*width+x]){if(x>=x0&&x<x0+cr.right&&y>=y0&&y<y0+cr.bottom)client++;else {edge++;if(x<2||x>=width-2||y<2||y>=height-2)outer++;}}
 free(before);SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);ReleaseDC(w,src);printf("MEMO_FRAME_PIXELS edge=%u client=%u outer=%u\n",edge,client,outer);return outer>0;
}
#endif
