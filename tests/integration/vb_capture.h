#ifndef WIN16_TEST_VB_CAPTURE_H
#define WIN16_TEST_VB_CAPTURE_H
/* The caller supplies capture_verify and its own CHECK failure policy. */
static uint64_t capture(HWND w,const WCHAR *file,BOOL verify){
    RECT r;
    CHECK(GetWindowRect(w,&r));
    int cx=r.right-r.left,cy=r.bottom-r.top;
    CHECK(cx>0 && cy>0);
    BITMAPINFO info= {0};
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=cx;
    info.bmiHeader.biHeight=-cy;
    info.bmiHeader.biPlanes=1;
    info.bmiHeader.biBitCount=32;
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);
    void *data;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&data,NULL,0);
    CHECK(screen && dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    memset(data,255,(size_t)cx*cy*4);
    CHECK(PrintWindow(w,dc,0));
    uint64_t hash=14695981039346656037ull;
    BYTE *b=data;
    for(size_t i=0; i<(size_t)cx*cy*4;
    ++i)hash=(hash^b[i])*1099511628211ull;
    capture_verify(w,r,cx,cy,data,verify);
    BITMAPFILEHEADER head= {0};
    head.bfType=0x4d42;
    head.bfOffBits=sizeof(head)+sizeof(info.bmiHeader);
    head.bfSize=head.bfOffBits+(DWORD)((size_t)cx*cy*4);
    DWORD n;
    HANDLE out=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
    CHECK(out!=INVALID_HANDLE_VALUE && WriteFile(out,&head,sizeof(head),&n,NULL) && n==sizeof(head) && WriteFile(out,&info.bmiHeader,sizeof(info.bmiHeader),&n,NULL) && n==sizeof(info.bmiHeader) && WriteFile(out,data,head.bfSize-head.bfOffBits,&n,NULL) && n==head.bfSize-head.bfOffBits && CloseHandle(out));
    SelectObject(dc,old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(w,screen);
    return hash;
}
#endif
