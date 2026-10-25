#ifndef NTW16S_TEST_SYSTEM_GLYPH_MASK_H
#define NTW16S_TEST_SYSTEM_GLYPH_MASK_H
/* Independent bitmap reference: host System16 glyphs scaled to the native
 * application's M advance and cell, without using the product font builder. */
static BOOL system_mask(const DWORD *pixels,const RECT *row,HFONT font)
{
    HDC src=CreateCompatibleDC(NULL),dst=CreateCompatibleDC(NULL);if(!src || !dst)return FALSE;
    HBITMAP mono=CreateBitmap(128,64,1,1,NULL);BITMAPINFO bi={0};DWORD *expected=NULL;
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=200;bi.bmiHeader.biHeight=-64;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bm=CreateDIBSection(dst,&bi,DIB_RGB_COLORS,(void **)&expected,NULL,0);
    HFONT system=CreateFontW(16,0,0,0,FW_BOLD,0,0,0,ANSI_CHARSET,OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"System");
    if(!mono || !bm || !system)ExitProcess(2);
    HGDIOBJ os=SelectObject(src,mono),od=SelectObject(dst,bm),of=SelectObject(src,system),nf=SelectObject(dst,font);
    TEXTMETRICW st,nt;GetTextMetricsW(src,&st);GetTextMetricsW(dst,&nt);
    RECT a={0,0,128,64},b={0,0,200,64};FillRect(src,&a,GetStockObject(WHITE_BRUSH));FillRect(dst,&b,GetStockObject(WHITE_BRUSH));
    SetBkColor(src,RGB(255,255,255));SetTextColor(src,0);SetStretchBltMode(dst,BLACKONWHITE);
    const WCHAR text[]=L"E8M0M0E8";int width=0;
    for(int i=0;i<8;++i){SIZE sw,nw;GetTextExtentPoint32W(src,text+i,1,&sw);GetTextExtentPoint32W(dst,text+i,1,&nw);
        FillRect(src,&a,GetStockObject(WHITE_BRUSH));TextOutW(src,0,0,text+i,1);
        StretchBlt(dst,width,0,nw.cx,nt.tmHeight,src,0,0,sw.cx,st.tmHeight,SRCCOPY);width+=nw.cx;}
    GdiFlush();BOOL found=FALSE;
    for(int y=row->top;y+nt.tmHeight<=row->bottom && !found;++y)for(int x=row->left;x<row->left+12 && !found;++x){
        BOOL equal=TRUE;
        for(int dy=0;dy<nt.tmHeight && equal;++dy)for(int dx=0;dx<width;++dx)
            if(((pixels[(y+dy)*600+x+dx]&0xffffff)==0)!=((expected[dy*200+dx]&0xffffff)==0)){equal=FALSE;break;}
        if(equal)found=TRUE;
    }
    SelectObject(src,of);SelectObject(src,os);SelectObject(dst,nf);SelectObject(dst,od);
    DeleteObject(system);DeleteObject(mono);DeleteObject(bm);DeleteDC(src);DeleteDC(dst);return found;
}
#endif
