# NTW16S core

One local renderer builds ntw16s32.dll / ntw16s64.dll, ABI 3.
Public entry is ntw16s.h; native_api.h and the remaining implementation helpers
are private. The two trampoline binding functions are not DLL exports.
An ordinary load supports explicit same-thread API use and stays passive.
A versioned launch payload enables early API interception and automatic window
attachment through this same renderer. OS rundll32 uses the real NONAME ordinal1
installer callback for opposite-width targets. Staged imports and actual target
initialization are separate verification events. The default launcher policy
enables styling and native CreateProcess A/W descendant propagation. Each
creation layer retains the original caller suspension or releases only its own
added suspension. Installation failure rolls back only its uncommitted child.

Upstream core files are pinned in the import manifest and individually receipted.
The five graphical resource headers and resources.md are byte-exact from the
same pinned ntvdm64 commit. Original resource notices and provenance are retained;
these are not project-authored or relicensed artwork. Both DLLs embed the original
provenance as RCDATA5. tools/verify_resource_identity.py rejects any substitution;
package verification checks all 18 compiled atlas tables in each DLL.
No resource generation or source rewriting occurs at build time. Source artwork
identity is separate from host font, palette and DPI behavior.

C17 MinGW-w64 GCC builds the complete product; C++ Detours is retired.
The private MinHook/HDE C backend preserves its original BSD notices and is
limited to the initial staged-loader transaction, with no waiting or thread
freezing. Private SDK PE staging restores original import metadata.
Static runtime/TLS cleanup retains thread notifications; DllMain performs no GUI work.
Consumers retain the DLL until all HWNDs, scopes, font leases and callbacks
have ended on their owning GUI threads. Explicit removal restores borrowed
application state; WM_NCDESTROY owns final attached resource release.

T17 S3 adds exact MSVBVM60-owned ThunderRT6CheckBox for ordinary windowed
checkboxes. The runtime owns Value, click and keyboard behavior. Existing
System glyph/font painting is reused. Graphical/push-like/icon/bitmap and
owner-drawn variants remain native; a live transition retires this painter.
Returning to a plain style requires a fresh attachment/window to resume it.
VB5 and OptionButton are not inferred from this result. Real-application
checks and independent C protocol fixtures are reported separately.
