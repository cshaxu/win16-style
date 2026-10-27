# M0 T18 S45 — status bar last pane with sizing grip

S44's real TStatusBar sample changed only the first of two pane borders. On
ComCtl32 6.16, self-owned native x86/x64 controls with two ordinary panes
confirmed that the native border entry is called for both panes without a size
grip and only the first with `SBARS_SIZEGRIP`. Synchronous repaint and
screen/WM_PRINTCLIENT comparisons ruled out a missed repaint or a screen-only
sampling difference in this case. Public pane type and text remained normal.

Diagnostic-only DrawThemeText observation found a call from native comctl
before each pane's text. Initially adding three diagnostic hooks exceeded the
40-slot transaction and failed at DeleteDC with error 122 (process status
0xC0000142); a scratch-only larger table allowed the observation. The product
adds just one DrawThemeText hook and retains the original 40-slot capacity.

The product frames a verified native final pane before its themed text only
for a sizing-grip status control with nonempty ordinary text, no icon, valid
native owner/DC, matching geometry and unchanged epoch. The original theme
call receives the same arguments and entry last-error state. Empty, icon,
owner-drawn, simple, RTL and other unsupported cases remain native. This is
bounded behavior, not full status bar or VB/Delphi coverage. No original image
resource or public ABI changed.

Self-owned x86/x64 C module tests at 96 DPI verify last-pane pixels and clip,
empty/icon/owner-draw fallback, owner data, and removal reentry during icon
query; the old released x64 DLL fails the new positive assertion. The existing
144-DPI model also passed with a size-grip fixture including icon, custom draw,
clip, mutation, destruction and removal. The new standalone probe initially
failed attachment when it created its root with WS_VISIBLE; creating and then
showing the root corrected the test setup, and both widths passed. This failure
is not represented as a product fix.

The independent final package was tested on self-owned DelphiVCL4Python 1.0.7
x86 and x64 at 144 DPI. On the same window before/after removal, screen and
WM_PRINTCLIENT each showed 440 first-pane and 804 last-pane changed pixels,
zero outside the pane edges; ordinary text and original owner-draw data were
restored. [Evidence](../etc/evidence/delphi-s45-status-pane.json) records this
single runtime and sample. Physical input, other runtime versions and full T18
integration remain untested; earlier S40/S41 unresolved issues remain open.

Independent clean GCC/Ninja/ccache builds and ABI checks passed. The four-file
package passed 26 focused cases, zero skips, in 8.50 seconds, including startup
hook capacity/rollback, both status variants, date and color tests. Both
published locations match the same-batch hashes and installed x86/x64 pane
probes passed. Original resource identity and source provenance checks passed.

| File | SHA-256 |
| --- | --- |
| demo.exe | `56956573675d393222460a5b93c96ec50afb5470bafb430a8576b152ee757838` |
| ntw16s32.dll | `428a1167f1e6689ddd9e5c25e3816d5785a10acc8af6c872d830443d7c91b06d` |
| ntw16s64.dll | `eeaba3804b37025ab6b47633d47cd8e44cd9a27bf3595c66f71cf55777072b23` |
| run16s.exe | `b2a36fdb82dcc4bded209c77b5070c0be4baf467d109e53e3f565435b0d7fb84` |

T18 and M0 remain open.
