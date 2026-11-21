/* Public primitive contract, now the original Win2000 standard scheme rather
 * than DrawFrameControl using the current host's system colors. */
#include <windows.h>
#include <stdio.h>

typedef BOOL (WINAPI *draw_fn)(HDC,RECT *,UINT);
int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 1;
    HMODULE module=LoadLibraryW(argv[1]);if(!module)return 2;
    draw_fn draw=(draw_fn)(void *)GetProcAddress(module,"Ntw2ksDrawButton");if(!draw)return 3;
    if(draw(NULL,NULL,0))return 4;
    HDC dc=CreateCompatibleDC(NULL);void *pixels;
    BITMAPINFO bi={.bmiHeader={sizeof(BITMAPINFOHEADER),32,-32,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&pixels,NULL,0);if(!dc||!bitmap)return 5;
    HGDIOBJ old=SelectObject(dc,bitmap);if(!old||old==HGDI_ERROR)return 6;
    const UINT states[]={0,DFCS_PUSHED,DFCS_INACTIVE,DFCS_CHECKED,DFCS_FLAT};
    for(unsigned i=0;i<ARRAYSIZE(states);++i){
        RECT bounds={1,1,31,31};PatBlt(dc,0,0,32,32,BLACKNESS);
        if(!draw(dc,&bounds,states[i]) || bounds.left!=1 || bounds.right!=31)return 7;
        COLORREF edge=(states[i]&DFCS_FLAT)?RGB(128,128,128):
            (states[i]&(DFCS_PUSHED|DFCS_CHECKED))?RGB(64,64,64):RGB(255,255,255);
        COLORREF center=GetPixel(dc,15,15);
        COLORREF drawn_edge=GetPixel(dc,1,15);
        if(drawn_edge!=edge ||
           (center!=RGB(212,208,200) && !((states[i]&DFCS_CHECKED)&&center==RGB(255,255,255)))){
            fprintf(stderr,"button state=%x edge=%06lx expected=%06lx center=%06lx\n",
                states[i],(unsigned long)drawn_edge,(unsigned long)edge,(unsigned long)center);
            return 8;
        }
    }
    RECT adjusted={0,0,32,32};
    if(!draw(dc,&adjusted,DFCS_ADJUSTRECT) || adjusted.left!=2 || adjusted.top!=2 || adjusted.right!=30 || adjusted.bottom!=30)return 9;
    if(!SelectObject(dc,old)||!DeleteObject(bitmap)||!DeleteDC(dc)||!FreeLibrary(module))return 10;
    printf("NTW2KS_BUTTON_OK bits=%u states=5 source_scheme=1 caller_rect=1\n",(unsigned)(8*sizeof(void *)));return 0;
}
