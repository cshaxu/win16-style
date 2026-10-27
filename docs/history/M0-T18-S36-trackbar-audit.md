# M0 T18 S36 — VCL trackbar audit and feasibility

The frozen 74-row working ledger remains provisional; this S investigated one
concrete real sample, not the entire inventory. DelphiVCL4Python 1.0.7 x86 and
x64 self-owned instances at 144 DPI use a runtime-DLL-owned TTrackBar, which
was not attached by the published package. The actual WM_PAINT and WM_NCPAINT
calls forwarded to the system-registered trackbar procedure from the class
owner. Extra window bytes matched native (4/8); public range was 0..10,
thumb/channel rectangles were nonempty, and position changed 0->5->0.

The default style was 0x61, including TBS_ENABLESELRANGE, which the existing
trackbar painter rejects. Selection was initially empty (-1,-1), changed to
2..7, and returned to empty with TBM_CLEARSEL; 11 ticks were observed. The
first x64 experiment tried TBM_SETSEL(-1,-1), which clamped to 0..0; its owned
instance exited, and subsequent x86/x64 experiments used CLEARSEL correctly.

On both styled widths and an unstyled x64 control, the actual VCL parent
returned CDRF_NOTIFYITEMDRAW at PREPAINT and CDRF_DODEFAULT on the observed
channel/thumb item stages. The existing renderer only intercepts after default
PREPAINT, so owner admission alone is not enough. A scratch-only observer
requested private ITEMPOSTPAINT only when the application's item return was
exactly default, then drew a one-pixel black frame on native channel/thumb
rectangles. Original app notifications were still forwarded; the extra private
post stage was not sent to the app. This is a feasibility probe, not product
code, a claim of fully Win16 styling or a general permission to override
application-drawn content.

At selected range 2..7, x86/x64 captures differed from the matching native
run by 294 RGB pixels, all on the observed outer perimeters. The selected
interior and all other client pixels matched. The same result held for owned
disabled and synthetic mouse-down scenarios; application callback sequences
matched exactly (19 disabled and 22 pressed), capture returned to NULL, and
enabled state and value were restored. An additional synthetic app handler
painted the channel and returned CDRF_SKIPDEFAULT; both widths preserved that
channel, with 54 changes only on the remaining thumb perimeter. A synthetic
handler removed the observer reentrantly during ITEMPREPAINT; both widths
returned its original result, emitted no later overlay and restored all owned
hooks and import slots. These experiments do not establish physical mouse,
keyboard, other DPI, all custom-draw flags, destruction/reentrancy combinations,
all VCL versions or the product's lifetime handling. All test instances were
owned, ran on isolated desktops, and exited. No product code or resources were
changed. Relevant raw reports and pixel captures remain in registered
`build/t18-s36-vcl-audit`.

The audit has discovered a concrete follow-up candidate but the other frozen
rows are still unfinished. A subsequent S must first implement and verify
runtime-owned trackbar identity and a bounded postpaint policy with independent
C tests for application draw results, selection, clipping, DPI, state, reentry,
cleanup and a genuine old-build negative; it must not infer compatibility for
other controls from this sample. The 74-row audit must then resume. P1 `fd608d0` was pushed; all seven changed files including four assets
were retrieved from the actual GitHub commit and matched. S36 transfers the
verified trackbar candidate to S37; the remaining 74-row audit is not complete.


The S36 independent clean x86/x64 GCC 16.1.0 / Ninja / ccache 4.13.6 build
passed source, resource, governance and ABI gates. Snapshot file SHA-256:
`76d933b19c31ccb97e4e0e14034dcf89fceff66a7a53ab7d8f0364020dc86559`.
The actual same-batch package passed 18 focused cases, no skips, in 35.92s;
105 standard cases were not selected. Both installed-entry trackbar probes
passed. All eight deployed files matched the tested package:

| File | SHA-256 |
| --- | --- |
| demo.exe (x64) | `ed45f9cf2e050b12c6adc8e0c96e1de3cef16d4cc589c69c560e2b76eec8ec7e` |
| ntw16s32.dll (x86) | `284e9fc36b5bfe11743305bd39f9ff8877ede6486cd41cd2a11881fc1c8536e5` |
| ntw16s64.dll (x64) | `fe27bd210527738edf10399badebd45f81b069ed36670688d2fae99f8ca488c0` |
| run16s.exe (x64) | `26c9ae993968c6509db73d44237d30c9ededbe42f89a17610a868e6f55ca4d19` |

The synthetic mouse-down captured the mouse, but the sampled native item state
remained zero; this is not proof of pressed-state appearance. DPI other than
144 and physical input remain untested. Source search included `rg -n -C 3
'TTrackBar|trackbar' src/ntw16s-dll/control_protocol.c
src/ntw16s-dll/trackbar_paint.c` and the existing `trackbar_notify` implementation.
Only documentation and audit evidence changed; the new binary hashes result from
an independent same-batch rebuild, not a product source modification.
