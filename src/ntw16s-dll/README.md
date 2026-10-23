# NTW16S core

One local renderer builds ntw16s32.dll / ntw16s64.dll, ABI 3.
Public entry is ntw16s.h; native_api.h and the remaining implementation helpers
are private. The two trampoline binding functions are not DLL exports.
An ordinary load supports explicit same-thread API use and stays passive.
A versioned launch payload enables early API interception and automatic window
attachment through this same renderer. OS rundll32 uses the real NONAME ordinal1
installer callback for opposite-width targets. Staged imports and actual target
initialization are separate verification events. Process propagation is the next
task; the current launcher policy only enables window styling.

Upstream project-authored core files are pinned in the repository import
manifest and individually receipted. Five glyph headers contain new geometry
from tools/generate_glyphs.py, without copied Win3.1 resource pixels. Their
dimensions satisfy the renderer interface; pixel-exact VGA fidelity is not claimed.
Generated headers are checked in; no source rewriting occurs at build time.

Static TLS and /MT require thread notifications; DllMain performs no GUI work.
Consumers retain the DLL until all HWNDs, scopes, font leases and callbacks
have ended on their owning GUI threads. Explicit removal restores borrowed
application state; WM_NCDESTROY owns final attached resource release.
