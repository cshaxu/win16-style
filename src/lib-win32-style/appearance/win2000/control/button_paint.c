/* Win2000 USER button behavior: btnctl.c xxxDrawButton/xxxBNDrawText and
 * rtl/draw.c DrawPushButton. Source identities and original image samples
 * are recorded by T27; this does not use Win3.x bitmap glyphs. */
#include "button_paint.h"
#include "../common/edge.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include <wchar.h>

static BOOL checked_face(HDC dc,const RECT *r,const win2k_control_palette *p)
{
    if(p->highlight!=RGB(255,255,255))return win2k_control_fill(dc,r,p->highlight);
    /* ntuser/kernel/init.c LW_BrushInit's original monochrome gray pattern. */
    static const WORD pattern[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
    HBITMAP bitmap=CreateBitmap(8,8,1,1,pattern);if(!bitmap)return FALSE;
    HBRUSH brush=CreatePatternBrush(bitmap);BOOL result=FALSE;
    if(brush){
        COLORREF back=SetBkColor(dc,p->highlight),text=SetTextColor(dc,p->face);
        result=FillRect(dc,r,brush)!=0;
        SetBkColor(dc,back);SetTextColor(dc,text);DeleteObject(brush);
    }
    DeleteObject(bitmap);return result;
}
BOOL win2k_push_button_frame(HDC dc,RECT *bounds,UINT state,BOOL default_button,const win2k_control_palette *p)
{
    if(!dc || !bounds || !p)return FALSE;
    RECT r=*bounds;
    if(default_button){
        HBRUSH border=CreateSolidBrush(p->text);if(!border)return FALSE;
        BOOL drawn=FrameRect(dc,&r,border)!=0;DeleteObject(border);if(!drawn)return FALSE;
        InflateRect(&r,-1,-1);
    }
    UINT flags=BF_RECT|BF_ADJUST|BF_SOFT;
    if(state&DFCS_FLAT)flags|=BF_FLAT;
    if(state&DFCS_MONO)flags|=BF_MONO;
    if(default_button && (state&DFCS_PUSHED))flags|=BF_FLAT;
    if(!win2k_control_edge(dc,&r,(state&(DFCS_PUSHED|DFCS_CHECKED))?EDGE_SUNKEN:EDGE_RAISED,flags,p))return FALSE;
    if(state&DFCS_CHECKED){if(!checked_face(dc,&r,p))return FALSE;}
    else if(!win2k_control_fill(dc,&r,p->face))return FALSE;
    if(state&DFCS_ADJUSTRECT)*bounds=r;
    return TRUE;
}
static style_control_button_role role(HWND window,void *context,DWORD style)
{(void)window;(void)context;return (style_control_button_role){style&BS_TYPEMASK,FALSE};}
static HFONT font(HWND window,void *context)
{
    (void)context;HFONT value=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
    return value?value:(HFONT)GetStockObject(SYSTEM_FONT);
}
typedef struct button_text {const WCHAR *value;UINT format;} button_text;
static BOOL CALLBACK multiline(HDC dc,LPARAM value,WPARAM length,int width,int height)
{
    const button_text *text=(const button_text *)value;RECT r={0,0,width,height};(void)length;
    return DrawTextW(dc,text->value,-1,&r,text->format)!=0;
}
static BOOL draw_content(HDC dc,LPARAM value,WPARAM length,int x,int y,int width,int height,
    UINT type,DRAWSTATEPROC draw,BOOL disabled,const win2k_control_palette *p)
{
    if(!disabled)return DrawStateW(dc,NULL,draw,value,length,x,y,width,height,type);
    HBRUSH light=CreateSolidBrush(p->highlight),gray=CreateSolidBrush(p->gray);BOOL result=FALSE;
    if(light && gray)result=DrawStateW(dc,light,draw,value,length,x+1,y+1,width,height,type|DSS_MONO) &&
        DrawStateW(dc,gray,draw,value,length,x,y,width,height,type|DSS_MONO);
    if(light)DeleteObject(light);
    if(gray)DeleteObject(gray);
    return result;
}
BOOL win2k_button_paint(HWND window,style_instance *instance,HDC dc,const win2k_control_palette *p)
{
    const style_control_button_reader reader={NULL,role,font};style_control_button_snapshot snapshot;
    if(!style_control_button_snapshot_read(window,instance,&reader,&snapshot))return FALSE;
    /* A later COMCTL32 image-list button is not USER's original text path.
     * Preserve its native image+text painter instead of silently losing images. */
    if(!(snapshot.style&(BS_ICON|BS_BITMAP))){
        BUTTON_IMAGELIST images={0};BOOL listed=(BOOL)SendMessageW(window,BCM_GETIMAGELIST,0,(LPARAM)&images);
        if(instance->detached || (listed && images.himl && images.himl!=BCCL_NOGLYPH)){
            style_control_button_snapshot_clear(&snapshot);return FALSE;
        }
    }
    RECT bounds;BOOL result=FALSE;int saved=SaveDC(dc);if(!saved)goto done;
    UINT cues=(UINT)SendMessageW(window,WM_QUERYUISTATE,0,0);
    if(instance->detached || !IsWindow(window) || !GetClientRect(window,&bounds))goto restore;
    if(!SelectObject(dc,snapshot.font) || SetBkMode(dc,TRANSPARENT)==0)goto restore;
    SetTextColor(dc,p->text);SetBkColor(dc,p->face);
    UINT state=(snapshot.state&BST_PUSHED)?DFCS_PUSHED:0;
    if(snapshot.style&BS_FLAT)state|=DFCS_FLAT|DFCS_MONO;
    if(!win2k_push_button_frame(dc,&bounds,state|DFCS_ADJUSTRECT,snapshot.role.type==BS_DEFPUSHBUTTON,p))goto restore;
    IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
    int width=0,height=0;UINT type;LPARAM value;WPARAM length=0;DRAWSTATEPROC draw=NULL;
    button_text text={snapshot.text,DT_WORDBREAK|DT_EDITCONTROL};style_control_button_image image;
    if(cues&UISF_HIDEACCEL)text.format|=DT_HIDEPREFIX;
    UINT horizontal=snapshot.style&BS_CENTER,vertical=snapshot.style&BS_VCENTER;
    if(horizontal==BS_RIGHT)text.format|=DT_RIGHT;
    else if(horizontal!=BS_LEFT)text.format|=DT_CENTER;
    if(vertical==BS_BOTTOM)text.format|=DT_BOTTOM;
    else if(vertical!=BS_TOP)text.format|=DT_VCENTER;
    if(snapshot.style&(BS_ICON|BS_BITMAP)){
        UINT kind=(snapshot.style&BS_BITMAP)?IMAGE_BITMAP:IMAGE_ICON;
        if(!style_control_button_image_read(window,instance,kind,&image) || instance->detached)goto restore;
        /* Image-list buttons are a later API, not this original USER path. */
        if(image.listed)goto restore;
        if(!image.image){result=TRUE;goto restore;}
        width=image.size.cx;height=image.size.cy;value=(LPARAM)image.image;
        type=kind==IMAGE_ICON?DST_ICON:DST_BITMAP;
    }else{
        if(!*snapshot.text){result=TRUE;goto restore;}
        BOOL multi=(snapshot.style&BS_MULTILINE)!=0;
        RECT measured={0,0,max(0,bounds.right-bounds.left-(multi?4:0)),max(0,bounds.bottom-bounds.top-(multi?2:0))};
        UINT format=(snapshot.style&BS_MULTILINE)?text.format:DT_SINGLELINE;
        if(!DrawTextW(dc,snapshot.text,-1,&measured,format|DT_CALCRECT))goto restore;
        width=measured.right-measured.left;height=measured.bottom-measured.top+1;
        value=(LPARAM)snapshot.text;length=wcslen(snapshot.text);type=DST_PREFIXTEXT;
        if(cues&UISF_HIDEACCEL)type|=DSS_HIDEPREFIX;
        if(snapshot.style&BS_MULTILINE){type=DST_COMPLEX;value=(LPARAM)&text;draw=multiline;}
    }
    int x=horizontal==BS_LEFT?bounds.left+2:horizontal==BS_RIGHT?bounds.right-width-2:(bounds.left+bounds.right-width)/2;
    int y=vertical==BS_TOP?bounds.top+1:vertical==BS_BOTTOM?bounds.bottom-height-1:(bounds.top+bounds.bottom-height)/2;
    if(state&DFCS_PUSHED){++x;++y;}
    result=draw_content(dc,value,length,x,y,width,height,type,draw,(snapshot.style&WS_DISABLED)!=0,p);
    if(result && (snapshot.state&BST_FOCUS) && !(cues&UISF_HIDEFOCUS)){
        InflateRect(&bounds,-1,-1);result=DrawFocusRect(dc,&bounds);
    }
restore:
    if(!RestoreDC(dc,saved))result=FALSE;
done:
    style_control_button_snapshot_clear(&snapshot);return result;
}
