#ifndef STYLE_CONTROL_VCL_IDENTITY_H
#define STYLE_CONTROL_VCL_IDENTITY_H
#include <windows.h>
/* Recognition of the existing supported VCL subset, independent of painting. */
BOOL style_control_vcl_group_button_identity(HWND w);
BOOL style_control_vcl_grid_identity(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_toolbar_identity(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_inplace_identity(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_lookup_identity(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_inplace_or_lookup(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_lookup_popup_identity(HWND w,HWND source,HWND source_parent,HMODULE owner);
BOOL style_control_vcl_nav_identity(HWND w,HMODULE owner,HWND parent);
BOOL style_control_vcl_bit_identity(HWND w,HMODULE owner);
BOOL style_control_vcl_checklist_identity(HWND w);
BOOL style_control_vcl_mdi_identity(HWND w,HWND parent,HWND host,HMODULE owner,BOOL (*host_attached)(HWND));
BOOL style_control_vcl_cool_identity(HWND w,HWND parent,HMODULE owner,BOOL (*current)(HWND,void *),void *context);
#endif
