# M0 T18 S38 — VCL control audit and color-combo feasibility

The 74-family S33 ledger was refreshed with later S34 edit and S37 trackbar
evidence, then seven previously unverified real HWND families were observed
in self-owned DelphiVCL4Python 1.0.7 x86/x64 instances at 144 DPI. TBitBtn,
TRadioGroup, TColorBox, TStringGrid, TDrawGrid, TDateTimePicker and TControlBar
were runtime-owned and unattached; this is not evidence about all versions,
properties or controls without a child HWND. See the [working ledger](../etc/evidence/delphi-s38-working-ledger.json).

The TBitBtn sample uses BS_OWNERDRAW and issued WM_DRAWITEM in both widths.
An observer of the runtime module's DrawFrameControl and DrawEdge imports
confirmed neither was called on the observed redraw. Both slots and the owned
parent subclass were restored. This does not exclude other painting APIs or
other button states; the existing VB graphical-button route cannot safely be
assumed to apply.

The TColorBox and TDateTimePicker samples both forwarded WM_PAINT and
WM_NCPAINT from their class module to the corresponding registered native
procedure. The date sample returned a valid system time; its popup, inputs and
date-specific renderer remain unverified. The color combo exposed 184 items,
selected index zero, and public button rectangle (105,1)-(129,21). Its popup
opened, became visible, closed and retained the selection. The full existing
combo painter rejects its owner-draw style and must not overwrite swatches.

A scratch-only experiment drew within that public button rectangle using the
existing Win3.1 combo glyph and per-coordinate DPI scaling. At 144 DPI, both
widths in normal, disabled and programmatically expanded states each changed
480 pixels entirely inside the button rectangle. An initial integer 2x glyph
was rejected because it would not fit the button's inner height. These are
transient paint experiments, not a product change: focus, input notifications,
restoration assertions, nested painting, clipping and lifecycle remain to be
tested before implementing any owner-drawn combo adaptation. Probe slots were
restored and owned processes closed. Other unresolved rows remain open.

No production source or resource was changed by this investigation. The clean
GCC 16.1.0/Ninja/ccache build snapshot SHA-256 was
`ad45905fa4c15a60140b7e443f3107809cb37841c8a52b8b53dea8f45370f961`.
The same-batch package passed 12 focused cases with no skips in 9.15 seconds;
other package cases and physical input were not run. Independent rebuild hashes
changed, so all four tested files were published to assets and system32 and
the latter's two installed-entry probes passed. Both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `d193719f36f26a5d641b878e744e211503641f1efe1aa54a02a40b9c0f355630` |
| ntw16s32.dll | `da6ab9ad748b0db0cc70fce278d279c62127b2f25160ae287fea99490c2b4b33` |
| ntw16s64.dll | `c847eb5d651f5d72ad4a0c9a50f2d7499a4465929f2b9be464c084e68ab0dcd7` |
| run16s.exe | `bf9857ae5f16360160bc31b59b89a115fef38792ff17b4d9d44a8efedcc61aa6` |

P1 commit and actual remote verification are pending. The bounded color-combo
prototype and all other incomplete rows transfer to subsequent T18 work.
