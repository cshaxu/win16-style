# M0 T18 S28 — VCL panel investigation

An owned x86 MD5summer 1.2.0.11 instance (SHA-256
`4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9`) was run
on an isolated desktop both bare and through the currently published assets
launcher. The styled instance actually loaded `assets/ntw16s32.dll`, SHA-256
`2dfdf58b2d668848f0da962a80d79e09ccc89e0128bf24ae36803a66ef990c8d`.
Each owned job exited with zero active processes; the startup information
dialog was automatically dismissed, and no file scan was started. Move,
minimize and restore completed, and the observed combo retained its selection.

At 96 DPI both reports identify the same application-owned `TPanel` HWND class
with no window extra bytes, `WS_CHILD` style `0x56000000`, extended style
`0x00010000`, and 276x56 dimensions. It is not attached in either run; the
styled root and the separately applicable controls are attached. Its real
class is `TPanel`, not a verified standard USER superclass. These observations
do not establish what portions of its client surface belong to child controls,
what drawing APIs are called, or whether there is a content-neutral Win16
adaptation. Separate full-window captures have different window layout and
cannot be interpreted as a panel pixel comparison without coordinate alignment.
The registered scratch contains the capture and report evidence. No product
change, new package, or delivery is claimed; S28 and T18 remain open.

A scratch-only native observer reused the existing owned-instance driver and
temporarily instrumented the application's resolved imports for `DrawEdge`,
`DrawFrameControl`, `FillRect`, `Rectangle` and `PatBlt` while synchronously
redrawing this panel. The panel has four visible direct children: a combo at
`(105,3)-(250,24)` and three buttons at `(2,26)-(90,51)`, `(94,26)-(182,51)`
and `(186,26)-(274,51)`. The tested redraw called `FillRect` once and did not
call the other four observed APIs. Both bare and styled runs restored all five
temporary import slots and their owned jobs emptied. This bounded observation
does not exclude APIs outside the five or drawing at other times.

The styled fill used a solid `0xf0f0f0` brush, a logical rectangle
`(0,0)-(284,255)`, complex clip bounding box `(4,195)-(280,251)`, and viewport
origin `(-4,-195)`. Although `WindowFromDC` identified the panel, the larger
rectangle and translated viewport show that simply treating this as a panel-
local fill would be unsound. Direct 276x56 client-DC captures of the separate
bare and styled runs were compared after excluding the four child rectangles
and the rightmost four columns, where the differently sized parent clips the
captures: all 5,637 remaining RGB pixels were identical. The excluded child
areas and four columns are not covered by this comparison. The paint API trace
and pixel comparison are evidence against blindly intercepting this one fill;
they do not prove that another panel, state, theme or version cannot be styled.

A second observation used a byte-identical task-local copy of the modern VCL
`cnc-ddraw config.exe` (SHA-256
`e21131cca40be055b9cf728ed822474f2560793635fc323127b63ba9a838d5d6`) with
its own scratch INI on an owned isolated desktop. At actual 144 DPI the current
assets DLL loaded and the root attached; the first visible `TPanel` was an
unattached, application-owned 350x702 window. Its bounded synchronous redraw
used `FillRect` with a solid `0xf3f3f3` brush, exact full-client clip and zero
viewport/window origins; none of the other four observed imports was called.
All five temporary imports were restored, the instance closed with an empty
job, and the original game's INI remained SHA-256
`47bc143a85fe84a0cc6c9ead9a0d12b8ff6c51395f84be8ca4846d6555633f85`.
Unlike the legacy sample's translated parent-area fill, this is a local fill
with a different application-selected color. Neither a matching class name nor
these two observed colors establish whether the brush represents a user-chosen
color, an application theme, or a safe replaceable system face. No code or
original artwork was copied from the application. Further evidence is needed
before applying a generalized panel fill or claiming a classic panel renderer.

Embarcadero documents both the panel's [Color](https://docwiki.embarcadero.com/Libraries/Sydney/en/Vcl.ExtCtrls.TPanel.Color)
and [ParentColor](https://docwiki.embarcadero.com/Libraries/Florence/en/Vcl.ExtCtrls.TPanel.ParentColor)
properties: the application may choose its own color or inherit one, and a
color assignment changes inheritance. The documented
[BevelOuter](https://docwiki.embarcadero.com/Libraries/Athens/en/API%3AVcl.ExtCtrls.TPanel.BevelOuter)
is likewise an application-controlled property. These object-level contracts
do not themselves provide a cross-process public HWND message for reliably
reading those values from the two observed targets. Consequently, the observed
GDI brush color or class name alone is insufficient to distinguish a default
face from deliberate application styling. The bounded evidence supports keeping
these panel surfaces native for now; it does not prove a safe solution is
impossible. A separately named TODO records the missing public contract and
the required preservation checks.

This investigation changes no product source. Its independent x86/x64 clean
GCC 16.1/Ninja/ccache build passed source, original-resource, governance and ABI
gates. The same-snapshot four-file package passed 16 focused cases in 6.85
seconds, with no skips; 101 standard cases and the separate full integration
matrix were not run. The newly built bytes differed from the prior batch, so
the tested four files were published to both `assets/` and `O:/winnt/system32`
and all eight destination hashes matched. Each installed launcher passed the
x86 and x64 spin model, covering actual 96/144 DPI across the four runs.
SHA-256: demo `33b4e6cde79a6398906f991af1d49dbabbd8bed0ef09b5fce2e613bff3f01b41`,
x86 DLL `93011871df1597d9e5a95da1775e66d56165eff1335b385a448317a5258c80b9`,
x64 DLL `b94f098150db0cc674a16efab8ccbfc7e29fcdb82f64c4452b3d9a4540dfbd66`,
launcher `25c201c3336fbb3a91ebc0f7de1997dbc239c8968b7bb5352489b52e85265fed`.
No panel styling is implied by this package. Physical input, unrelated
applications, unobserved drawing APIs and the T18 final integration remain
unverified. P1 `f30725a` was pushed, and all seven changed files were fetched
from GitHub and matched to their committed blobs. S28 closes as an investigation
with the panel gap explicitly retained in TODO; the next T18 S investigates
windowless label drawing. T18 and M0 remain open.
