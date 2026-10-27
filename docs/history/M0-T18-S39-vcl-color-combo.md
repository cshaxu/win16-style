# M0 T18 S39 — VCL color-combo button

DelphiVCL4Python 1.0.7 x86/x64 TColorBox is a runtime-owned, owner-drawn
native COMBOBOX wrapper. The existing full combo painter correctly rejects
owner-draw: it would otherwise overwrite the application's color swatches.
The old released package left this sample unattached.

The new candidate admits only an exact TColorBox after observing its class
module forward WM_PAINT to the registered native COMBOBOX procedure and
checking the public item/list/button geometry, style, owner, same-thread
parent, count and selection. It lets native and application painting finish,
then paints only the documented button rectangle with the existing Win3.1
glyph. Glyph coordinates scale proportionally with DPI. Unsupported styles,
RTL, geometries, owners or DC transforms stay native. For WM_PRINTCLIENT the
native control changes the DC clip; the implementation borrows the incoming
clip for its button and restores the native return state. Its temporary region
also has an exception-unwind cleanup. The original glyph and provenance bytes,
public ABI, three components and four product files remain unchanged.

Self-owned C superclass models at 96 DPI pass both widths: fake same-name
paint is rejected, owner-drawn content remains outside the button, the styled
and native output differ only inside it (300 pixels), clip outside is zero,
returned DC clipping matches native, keyboard selection/notification count and
popup state work, and nested printing, reentrant removal, destruction and an
application exception during printing are handled. A repeated-print test
observed a late stable whole-process GDI count (x86 54->54, x64 58->58); the
exception check observed no additional GDI count. These do not prove zero
whole-process growth. The older package failed the positive attachment test
on both widths. The fixture is a native C model, not a Delphi runtime.

At 144 DPI, real self-owned DelphiVCL4Python instances of both widths attached;
normal and disabled paired comparisons changed 480 pixels only in the public
button rectangle. Open and close notifications matched the old package. The
expanded state showed 480 pixels confined to the button on offscreen printing
with matching notifications. Screen captures occasionally differed outside
the button by 1718 pixels: one of four ordinary redraws, without removal,
and one of four removal/redraws showed the same difference. Other paired
samples showed only the 480 button pixels. Focus and popup geometry matched;
the intermittent screen difference's cause remains unresolved, so not every
screen redraw state is claimed verified. No physical input or other VCL
versions were verified.

The clean GCC 16.1.0/Ninja/ccache source snapshot SHA-256 was
`f4fdc3b404a6d1dcc8c8ca09978d1e9ffed6116120c64319e2bb040635cce371`.
The actual same-batch package passed 20 focused tests, zero skips, in 20.54s;
other package cases were not selected. Both published locations passed their
x86/x64 installed-entry probes. The four tested hashes match both locations:

| File | SHA-256 |
| --- | --- |
| demo.exe | `2343243563c93339f476249b93d4a535e60bf8a94ddb925d27d3185a4cec723a` |
| ntw16s32.dll | `116782aa70160f28104173b8df79a7ff4db9a5d383803a2d52a1f01fba4d5dee` |
| ntw16s64.dll | `5160fda1acd67f71eda972ab3e5921e06119f88dd08a5ea37ecdc170e7f983f6` |
| run16s.exe | `27c06800b855686892e702b79d9459e9548e1a82fe55959bfa8cc04bbe4a515f` |

P1 commit and actual remote verification are pending. The intermittent screen
comparison and unverified framework variants stay open for follow-up; the
larger T18 audit is not complete.
