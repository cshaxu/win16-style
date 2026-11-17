#ifndef STYLE_CLASSIC_FILE_LIVE_H
#define STYLE_CLASSIC_FILE_LIVE_H
#include "classic_dialog_proxy.h"
/* Borrow the owning proxy for the complete operation. Native calls may reenter
 * and change its active HWND; read that current value after the native call. */
HRESULT style_classic_file_live_set_filter(style_classic_dialog_proxy *,UINT);
HRESULT style_classic_file_live_get_filter(style_classic_dialog_proxy *,UINT *);
HRESULT style_classic_file_live_set_name(style_classic_dialog_proxy *,LPCWSTR);
HRESULT style_classic_file_live_get_name(style_classic_dialog_proxy *,LPWSTR *);
#endif
