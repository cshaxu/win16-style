#ifndef STYLE_MSSTYLES_V3_THEME_H
#define STYLE_MSSTYLES_V3_THEME_H

#include <windows.h>
#include <stddef.h>

typedef struct msstyles_v3_text {
    const WCHAR *data;
    size_t length;
} msstyles_v3_text;

typedef struct msstyles_v3_theme {
    HMODULE resources;
    PCWSTR variant_name;
    msstyles_v3_text properties;
} msstyles_v3_theme;

/* Borrows the resource-only module and its views until the caller releases
 * the module. Unknown variants return ERROR_NOT_FOUND. */
HRESULT msstyles_v3_read(HMODULE resources,PCWSTR size,PCWSTR color,msstyles_v3_theme *theme);
/* Returns one declared scheme and its display name in the theme's resource order.
 * Returned views borrow the mapped resource module. S_FALSE marks the end. */
HRESULT msstyles_v3_color_at(const msstyles_v3_theme *,size_t index,PCWSTR *token,
    msstyles_v3_text *label);
/* Exact section/key match, ignoring identifier case. Views borrow the INI. */
BOOL msstyles_v3_property(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,
    msstyles_v3_text *value);
/* Resolves a v3 image reference to its original RT_BITMAP DIB bytes. */
BOOL msstyles_v3_bitmap(const msstyles_v3_theme *theme,msstyles_v3_text image,
    const void **data,DWORD *length);

#endif