#ifndef STYLE_MSSTYLES_V4_PNG_H
#define STYLE_MSSTYLES_V4_PNG_H

#include <windows.h>
#include <wincodec.h>

/* Reads an opaque palette-role PNG into width*height caller-owned bytes. */
HRESULT msstyles_png_roles(IWICImagingFactory *,HMODULE,PCWSTR,UINT width,UINT height,BYTE *);

#endif
