# M0 T18 S44 — bounded VCL status-bar support

Newly created runtime-owned `TStatusBar` windows become eligible only after a
WM_PAINT forwards through `CallWindowProcW` from the class owner to the
registered native status procedure. The exact class, same-thread parent,
window extra bytes and system comctl identity are checked. The existing
native renderer frames eligible pane edges during the native paint path;
application text, icons and owner-draw callbacks retain their ownership.
Simple, no-border, popout and RTL paths remain native. This adds no product
file or public ABI and preserves the original resources byte for byte.

On real DelphiVCL4Python 1.0.7 x86 and x64 at 144 DPI, the final package
attached and retained pane text, owner-draw data `0x12345678` and simple-mode
round-trip. A same-window before/after removal capture changed 440 pixels on
the first pane edge and zero outside public pane edges in both widths. No
change was observed on the second pane in that sequence; this is not proof
that every pane or application is covered. The S43 DrawThemeBackground import
observer could not be installed; the result does not establish the complete
native call path. The earlier S43 x86 probe setup failure remains recorded.

An independent module-owned superclass fixture covered x86/x64 at 96/144 DPI:
real native forwarding, rejection of same-name fake/nonforwarding classes,
clipping, live and printed pixels, text and icon retention, owner-draw data,
simple/no-border/popout fallback, reentrant removal, mutation and destruction.
The old published x64 DLL failed this new fixture at the attachment assertion.
The fixture is a C model; its results are not represented as more Delphi
versions. [Evidence](../etc/evidence/delphi-s44-status.json) records the real
sample. No physical input or full T18 integration is claimed.

The independent clean GCC/Ninja/ccache x86/x64 build and ABI checks passed.
The same-batch package passed 24 focused cases without skips in 10.28 seconds,
including native status v5/v6, both DPIs, date and color regressions. Both
published locations match the tested hashes and the installed system32 x86/x64
144-DPI VCL status model passed. Full integration is pending T18 closure.

| File | SHA-256 |
| --- | --- |
| demo.exe | `3296cb060dd75d26c4c7b466e034c0edaabbc8b7fa450d8591483b3402296f5b` |
| ntw16s32.dll | `5e67a22952595672afdd300c6d8bdef00f9a85b79afac907abcb0546a32d6e38` |
| ntw16s64.dll | `9f416fa9ed5ec685104a5dbfb39a287edff7d8a4129da082768ba011558d2450` |
| run16s.exe | `f52a144d0f3151654bdb81d4a57ceff29ddb8d8cb469df7ff5c78c1781d0d157` |

The S40 screen discrepancy and S41 first probe exit remain unresolved.
T18 and M0 remain open.
