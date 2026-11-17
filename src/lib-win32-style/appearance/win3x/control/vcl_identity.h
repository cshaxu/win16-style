#ifndef WIN16_CONTROL_VCL_IDENTITY_H
#define WIN16_CONTROL_VCL_IDENTITY_H
#include "lib-win32-style/application/control/runtime.h"
BOOL win16_control_vb_data_role(HWND w,HWND parent);
BOOL win16_control_vb_user_edge_role(HWND w,HWND parent);
BOOL win16_control_vb_user_edge_identity(HWND w,HWND parent);

void win16_control_vcl_classify(HWND,DWORD,BOOL,HMODULE,style_control_runtime_candidates *);
void win16_control_vcl_classify_extra(HWND,DWORD,BOOL,HMODULE,style_control_runtime_extras *);
#endif
