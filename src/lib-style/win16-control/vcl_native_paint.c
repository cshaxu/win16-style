#include "lib-style/common/dc_geometry.h"
#include "vcl_native_paint.h"
#include "basic_paint.h"
#include "special_paint.h"
#include "../win16-common/palette.h"
#include <commctrl.h>

void win16_control_vcl_inplace_draw(const win16_control_vcl_paint_view *v,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{HMODULE caller=NULL;RECT client;if(!*v->candidate || *v->detached || !dc || !r || part!=1 || (state!=1 && state!=3) || !IsWindowEnabled(v->window) || !v->identity(v->window,*v->owner,*v->parent) || (!drop && v->lookup(v->window,*v->owner,*v->parent)) || WindowFromDC(dc)!=v->window || !GetClientRect(v->window,&client) || r->right!=client.right || r->top!=client.top || r->bottom!=client.bottom || r->left<client.left || r->right<=r->left || r->right-r->left>client.bottom-client.top || !style_dc_pixel_viewport(dc) || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=*v->owner)return;
 if(drop && state!=1)return;
 int saved=SaveDC(dc);if(!saved)return;int u=max(1,MulDiv(1,(int)GetDpiForWindow(v->window),96));int left=(r->left+r->right-7*u)/2,top=(r->top+r->bottom-9*u)/2;
 if(drop && (left<r->left+u || top<r->top+u || left+7*u>r->right-u || top+9*u>r->bottom-u)){RestoreDC(dc,saved);return;}
 if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
 if(win16_style_bitbtn_face(v->window,dc,r,clip,state==3?ODS_SELECTED:0) && !*v->detached){if(drop){win16_style_palette palette;win16_style_default_palette(&palette);SetDCBrushColor(dc,palette.edge);win16_control_combo_arrow(dc,left,top,96*u);}*v->verified=TRUE;}RestoreDC(dc,saved);}
void win16_control_vcl_nav_draw(const win16_control_vcl_paint_view *v,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{RECT client;POINT wo,vo;HMODULE caller=NULL;BOOL media;
 if(!*v->candidate || *v->detached || !dc || !r || part!=1 || !v->identity(v->window,*v->owner,*v->parent) || WindowFromDC(dc)!=v->window || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetWindowOrgEx(dc,&wo) || !GetViewportOrgEx(dc,&vo) || wo.y || vo.x || vo.y || !GetClientRect(v->window,&client) || r->top || r->bottom!=client.bottom || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=*v->owner)return;
 media=v->class_is(v->window,L"TMediaPlayer");
 if(media){if((state!=1 && state!=3 && !(state==4 && !IsWindowEnabled(v->window)) && !(state==5 && GetFocus()==v->window)) || wo.x || r->left<0 || r->left>client.right || r->right>client.right || r->right-r->left<=4)return;}
 else if((state!=1 && state!=3 && state!=4) || wo.x>0 || wo.x< -client.right || r->left || r->right<=4 || r->right>client.right+wo.x)return;
 if(win16_style_bitbtn_face(v->window,dc,r,clip,state==3?ODS_SELECTED:0) && !*v->detached)*v->verified=TRUE;}
