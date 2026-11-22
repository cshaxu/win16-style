#include "lib-win32-style/application/graphics/dc_geometry.h"
#include "control_vcl_paint.h"
#include "lib-win32-style/application/control/control_vcl_identity.h"
#include "lib-win32-style/application/control/control_class.h"
#include "lib-win32-style/application/control/control_chrome.h"
#include <commctrl.h>

void style_control_vcl_inplace_draw(const style_control_vcl_paint_view *v,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{HMODULE caller=NULL;RECT client;if(!v->state->candidate || v->instance->detached || !dc || !r || part!=1 || (state!=1 && state!=3) || !IsWindowEnabled(v->window) || !style_control_vcl_inplace_or_lookup(v->window,v->state->owner,v->state->parent) || (!drop && style_control_vcl_lookup_identity(v->window,v->state->owner,v->state->parent)) || WindowFromDC(dc)!=v->window || !GetClientRect(v->window,&client) || r->right!=client.right || r->top!=client.top || r->bottom!=client.bottom || r->left<client.left || r->right<=r->left || r->right-r->left>client.bottom-client.top || !style_dc_pixel_viewport(dc) || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=v->state->owner)return;
 if(drop && state!=1)return;
 int saved=SaveDC(dc);if(!saved)return;int u=max(1,MulDiv(1,(int)GetDpiForWindow(v->window),96));int left=(r->left+r->right-CLASSIC_COMBO_ARROW_WIDTH*u)/2,top=(r->top+r->bottom-CLASSIC_COMBO_ARROW_HEIGHT*u)/2;
 if(drop && (left<r->left+u || top<r->top+u || left+CLASSIC_COMBO_ARROW_WIDTH*u>r->right-u || top+CLASSIC_COMBO_ARROW_HEIGHT*u>r->bottom-u)){RestoreDC(dc,saved);return;}
 if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
 if(style_control_bit_button_face(v->window,dc,r,clip,state==3?ODS_SELECTED:0,v->palette,v->push) && !v->instance->detached){if(drop){SetDCBrushColor(dc,v->palette->edge);style_control_combo_arrow(dc,left,top,96*u,v->combo_arrow);}v->state->verified=TRUE;}RestoreDC(dc,saved);}
void style_control_vcl_nav_draw(const style_control_vcl_paint_view *v,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{RECT client;POINT wo,vo;HMODULE caller=NULL;BOOL media;
 if(!v->state->candidate || v->instance->detached || !dc || !r || part!=1 || !style_control_vcl_nav_identity(v->window,v->state->owner,v->state->parent) || WindowFromDC(dc)!=v->window || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetWindowOrgEx(dc,&wo) || !GetViewportOrgEx(dc,&vo) || wo.y || vo.x || vo.y || !GetClientRect(v->window,&client) || r->top || r->bottom!=client.bottom || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=v->state->owner)return;
 media=style_control_class_is(v->window,L"TMediaPlayer");
 if(media){if((state!=1 && state!=3 && !(state==4 && !IsWindowEnabled(v->window)) && !(state==5 && GetFocus()==v->window)) || wo.x || r->left<0 || r->left>client.right || r->right>client.right || r->right-r->left<=4)return;}
 else if((state!=1 && state!=3 && state!=4) || wo.x>0 || wo.x< -client.right || r->left || r->right<=4 || r->right>client.right+wo.x)return;
 if(style_control_bit_button_face(v->window,dc,r,clip,state==3?ODS_SELECTED:state==4?ODS_DISABLED:0,v->palette,v->push) &&
    !v->instance->detached)v->state->verified=TRUE;}
