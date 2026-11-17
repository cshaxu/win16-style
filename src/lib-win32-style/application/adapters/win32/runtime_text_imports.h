#ifndef STYLE_RUNTIME_TEXT_IMPORTS_H
#define STYLE_RUNTIME_TEXT_IMPORTS_H
#include <windows.h>
/* Existing ANSI text-import interception, local to this linked component.
 * The caller already qualified/pins the module and keeps this component and
 * the first font provider loaded for every installed call. All calls use
 * that same immutable provider. No module is loaded or target discovered.
 * Atomic import validation/rollback uses the existing module-import backend;
 * no physical unpatch/unload capability is added. */
typedef HFONT (*style_text_font_provider)(HDC);
DWORD style_runtime_text_imports(HMODULE,style_text_font_provider,BOOL *changed);
#endif
