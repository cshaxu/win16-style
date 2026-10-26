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

T17 S4 adds exact MSVBVM50-owned ThunderRT5CommandButton/CheckBox/OptionButton,
Frame and TextBox, plus MSVBVM60-owned ThunderRT6OptionButton. Public protocols
and grouping were observed in the original vbAccelerator intrinsic samples
by Steve McMahon; no sample code or binaries are part of the product. Runtime
font/DC painting uses the existing bounded policy; the actual VB5 Form name
is ThunderRT5Form. Choice controls share the same unsupported graphical-mode
native fallback. ComboBox/ListBox/scrollbar wrappers remain separate work.

T17 S5 admits exact runtime50/60 ComboBox, ListBox, HScrollBar and VScrollBar
wrappers observed in the same original samples. Public native data/input
protocols remain authoritative; existing glyphs and font policy are reused.
The editable combo child is verified through COMBOBOXINFO and native class
metadata. Its private System-font leases are bounded to 32 variants per owner,
held through reentrant native painting/setters and restored on removal or
reparenting. Outside synchronous painting WM_GETFONT preserves the application
source font; painting-time nested queries can observe the private painter font.
No arbitrary EDIT children, private VB object layouts or global font changes.

T17 S7 adds exact MSVBVM50-owned FileListBox, DirListBox and DriveListBox.
The ordinary file list uses the existing list renderer. Verified string-based
directory/drive owner-draw controls keep runtime images, data and callbacks;
only text on the exact DRAWITEM DC borrows the existing System font. Native
drive items/popup and application path logic remain authoritative. Directory
borders/scrollbars and drive chrome reuse existing primitives. Reparenting or
removal cancels the scope. T17 S8 also admits exact MSVBVM60-owned FileListBox
and DirListBox, verified in the original Compress-Z-It sample. Generic
owner-draw and unverified VB6 DriveListBox remain native. Self-contained C
fixtures are distinct from authentic VB5/VB6 proof.
