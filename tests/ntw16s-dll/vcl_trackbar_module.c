#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_TRACK_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
static HWND observed;
static window_fn remove_style;
static BOOL nested,nested_done,skip_channel,retire,removed,app_post,destroy_on_item,destroyed;
static unsigned pre,item,unexpected,app_posts;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_NOTIFY && l && ((NMHDR*)l)->hwndFrom==observed && ((NMHDR*)l)->code==NM_CUSTOMDRAW){
        NMCUSTOMDRAW *d=(void*)l;
        if(d->dwDrawStage==CDDS_PREPAINT){++pre;return CDRF_NOTIFYITEMDRAW | (app_post?CDRF_NOTIFYPOSTPAINT:0);}
        if(d->dwDrawStage==CDDS_ITEMPREPAINT){
            ++item;
            if(destroy_on_item){destroy_on_item=FALSE;destroyed=DestroyWindow(observed);return CDRF_DODEFAULT;}
            if(retire){retire=FALSE;removed=remove_style(observed);}
            if(nested && !nested_done){
                nested_done=TRUE;HDC dc=CreateCompatibleDC(d->hdc);HBITMAP bmp=CreateCompatibleBitmap(d->hdc,220,42);
                HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
                if(old){SendMessageW(observed,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);SelectObject(dc,old);}
                if(bmp)DeleteObject(bmp);
                if(dc)DeleteDC(dc);
            }
            if(skip_channel && d->dwItemSpec==TBCD_CHANNEL){
                int save=SaveDC(d->hdc);if(save){SetDCBrushColor(d->hdc,RGB(40,180,90));FillRect(d->hdc,&d->rc,(HBRUSH)GetStockObject(DC_BRUSH));RestoreDC(d->hdc,save);}
                return CDRF_SKIPDEFAULT;
            }
            return app_post?CDRF_NOTIFYPOSTPAINT:CDRF_DODEFAULT;
        }
        if(app_post && (d->dwDrawStage==CDDS_POSTPAINT || d->dwDrawStage==CDDS_ITEMPOSTPAINT)){++app_posts;return CDRF_DODEFAULT;}
        ++unexpected;return CDRF_DODEFAULT;
    }
    return DefWindowProcW(w,m,p,l);
}
static BOOL paint(HWND w)
{
    SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    RECT r;if(!GetClientRect(w,&r))return FALSE;HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);
    HBITMAP bmp=CreateCompatibleBitmap(screen,r.right,r.bottom);HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
    if(old){SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);SelectObject(dc,old);}
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    return old!=NULL;
}
static BOOL capture(HWND w,DWORD *pixels,int width,int height)
{
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO bi={0};void *bits=NULL;
    bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bmp=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,&bits,NULL,0);
    HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;
    if(old){SendMessageW(w,WM_SYSCOLORCHANGE,0,0);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        memcpy(pixels,bits,(size_t)width*height*sizeof(*pixels));SelectObject(dc,old);}
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    return old!=NULL;
}
static BOOL edge(RECT r,int x,int y)
{return x>=r.left && x<r.right && y>=r.top && y<r.bottom && (x==r.left || x==r.right-1 || y==r.top || y==r.bottom-1);}
static BOOL clipped_paint(HWND w)
{
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO bi={0};DWORD *pixels=NULL;BOOL okay=FALSE;
    bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=220;bi.bmiHeader.biHeight=-42;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bmp=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,(void**)&pixels,NULL,0);
    HGDIOBJ old=dc && bmp?SelectObject(dc,bmp):NULL;RECT thumb={0};
    if(old){
        SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
        for(unsigned i=0;i<220*42;++i)pixels[i]=0x005a5a5a;
        if(thumb.left>=0 && thumb.top>=0 && thumb.right<=220 && thumb.bottom<=42 &&
           thumb.right>thumb.left && thumb.bottom>thumb.top &&
           IntersectClipRect(dc,thumb.left,thumb.top,thumb.right,thumb.bottom)!=ERROR){
            SendMessageW(w,WM_SYSCOLORCHANGE,0,0);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
            unsigned inside=0,outside=0;
            for(int y=0;y<42;++y)for(int x=0;x<220;++x)if((pixels[y*220+x]&0xffffff)!=0x5a5a5a){
                if(x>=thumb.left && x<thumb.right && y>=thumb.top && y<thumb.bottom)++inside;else ++outside;
            }
            printf("TRACK_CLIP inside=%u outside=%u\n",inside,outside);okay=inside && !outside;
        }
        SelectObject(dc,old);
    }
    if(bmp)DeleteObject(bmp);
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(w,screen);
    return okay;
}
static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
    reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
    unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg && create && unreg);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove_style);
    HWND startup=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(startup);CHECK(DestroyWindow(startup));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclModuleTrackParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Module Track",WS_OVERLAPPEDWINDOW,20,20,400,200,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&icc));
    observed=CreateWindowExW(0,TRACKBAR_CLASSW,L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_AUTOTICKS|TBS_ENABLESELRANGE,20,20,220,42,root,NULL,cls.hInstance,NULL);
    CHECK(observed);pre=item=unexpected=0;nested=TRUE;nested_done=FALSE;CHECK(paint(observed));nested=FALSE;
    printf("TRACK_NATIVE nested=%u pre=%u item=%u unexpected=%u\n",nested_done,pre,item,unexpected);
    CHECK(DestroyWindow(observed));observed=NULL;
    for(unsigned mode=0;mode<2;++mode){
        CHECK(reg(mode==0));observed=create(root,FALSE);CHECK(observed && IsWindowVisible(observed));
        printf("TRACK_FIXTURE mode=%u initial_attached=%u\n",mode,attached(observed));
        CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(!mode)CHECK(!attached(observed));
        else {
            CHECK(attached(observed));SendMessageW(observed,TBM_SETRANGE,TRUE,MAKELPARAM(0,10));
            SendMessageW(observed,TBM_SETSEL,TRUE,MAKELPARAM(2,7));SendMessageW(observed,TBM_SETPOS,TRUE,5);
            CHECK(SendMessageW(observed,TBM_GETPOS,0,0)==5 && SendMessageW(observed,TBM_GETSELSTART,0,0)==2 && SendMessageW(observed,TBM_GETSELEND,0,0)==7);
            CHECK(clipped_paint(observed));
            pre=item=unexpected=0;nested=TRUE;nested_done=FALSE;CHECK(paint(observed));nested=FALSE;
            printf("TRACK_NESTED done=%u pre=%u item=%u unexpected=%u\n",nested_done,pre,item,unexpected);
            CHECK(nested_done && pre>=2 && item>0 && !unexpected);
            pre=item=unexpected=0;skip_channel=TRUE;CHECK(paint(observed));skip_channel=FALSE;CHECK(pre && item && !unexpected);
            pre=item=unexpected=app_posts=0;app_post=TRUE;CHECK(paint(observed));app_post=FALSE;
            CHECK(pre && item && app_posts && !unexpected);
            EnableWindow(observed,FALSE);CHECK(paint(observed));EnableWindow(observed,TRUE);
            removed=FALSE;retire=TRUE;CHECK(paint(observed));CHECK(removed && !attached(observed));
            CHECK(SendMessageW(observed,TBM_GETPOS,0,0)==5 && SendMessageW(observed,TBM_GETSELEND,0,0)==7);
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);
            CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            SendMessageW(observed,TBM_SETSEL,TRUE,MAKELPARAM(2,7));SendMessageW(observed,TBM_SETPOS,TRUE,5);
            RECT client,channel,thumb;CHECK(GetClientRect(observed,&client));
            CHECK(client.right==220 && client.bottom==42);
            SendMessageW(observed,TBM_GETCHANNELRECT,0,(LPARAM)&channel);SendMessageW(observed,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
            DWORD styled[220*42],native[220*42];skip_channel=TRUE;CHECK(capture(observed,styled,220,42));
            CHECK(remove_style(observed));CHECK(capture(observed,native,220,42));skip_channel=FALSE;
            unsigned changed=0,outside=0,channel_changes=0;
            for(int y=0;y<42;++y)for(int x=0;x<220;++x)if((styled[y*220+x]&0xffffff)!=(native[y*220+x]&0xffffff)){
                ++changed;if(!edge(thumb,x,y))++outside;
                if(x>=channel.left && x<channel.right && y>=channel.top && y<channel.bottom && !edge(thumb,x,y))++channel_changes;
            }
            printf("TRACK_PIXELS changed=%u outside_thumb=%u skipped_channel=%u\n",changed,outside,channel_changes);
            CHECK(changed && !outside && !channel_changes);
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);
            CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            destroyed=FALSE;destroy_on_item=TRUE;SendMessageW(observed,WM_SYSCOLORCHANGE,0,0);
            RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);CHECK(destroyed && !IsWindow(observed));observed=NULL;
        }
        if(observed)CHECK(DestroyWindow(observed));
        observed=NULL;CHECK(unreg());
    }
    CHECK(reg(FALSE));DWORD baseline=0,peak=0;BOOL native_gdi=GetEnvironmentVariableW(L"S37_GDI_NATIVE",NULL,0)!=0;
    BOOL long_gdi=GetEnvironmentVariableW(L"S37_GDI_LONG",NULL,0)!=0;
    unsigned rounds=long_gdi?384:24,tail=long_gdi?32:8;
    for(unsigned i=0;i<rounds;++i){
        observed=native_gdi?CreateWindowExW(0,TRACKBAR_CLASSW,L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_AUTOTICKS|TBS_ENABLESELRANGE,20,20,220,42,root,NULL,cls.hInstance,NULL):create(root,FALSE);
        CHECK(observed);CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(!native_gdi){CHECK(attached(observed));if(GetEnvironmentVariableW(L"S37_GDI_REMOVE",NULL,0))CHECK(remove_style(observed));}
        CHECK(DestroyWindow(observed));observed=NULL;GdiFlush();
        DWORD count=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(count);
        if(GetEnvironmentVariableW(L"S37_GDI_TRACE",NULL,0))printf("TRACK_GDI_STEP %u %lu\n",i,count);
        if(i==rounds-tail-1)baseline=count;
        if(i>=rounds-tail && count>peak)peak=count;
    }
    printf("TRACK_GDI mode=%s warmed=%lu peak=%lu\n",long_gdi?"long":"sample",baseline,peak);
    if(long_gdi)CHECK(peak<=baseline+2);
    CHECK(unreg());
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    printf("VCL_TRACK_MODULE_OK bits=%u fake=1 selection=1 nested=1 callbacks=1 apppost=1 appskip=1 pixels=1 clip=1 disabled=1 reentrant_remove=1 destroy=1 gdi_sample=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
