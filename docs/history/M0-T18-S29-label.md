# M0 T18 S29 — Delphi windowless label investigation

An owned, isolated x86 MD5summer 1.2.0.11 instance (SHA-256
`4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9`) was
observed bare and through the currently published assets launcher. The styled
instance loaded `assets/ntw16s32.dll` and reported its root attached; both
instances closed with empty owned jobs. No file scan was initiated. A test-only
observer temporarily traced resolved `DrawTextA` and `ExtTextOutA` imports
within the real panel's synchronous paint and restored all seven temporary
import slots. Text content was not logged; the probe reports only whether a
call begins with the known fixed `Checksum` label.

The main EXE's `.rsrc` contains a Delphi `TPF0` form stream with a `TLabel`
`Label1` record immediately preceding the fixed `Checksum type:` caption;
its encoded properties are Left 19, Top 7, Width 76, Height 13. During the
observed panel paint, the matching 14-character call used `DrawTextA` with
rectangle `(0,0)-(76,13)`, window origin `(-19,-7)` and exact clip
`(0,0)-(76,13)`, consistent with that resource's geometry. This is strong
sample-specific evidence of the label path, not permission to identify
arbitrary application text by caption or coordinates.

Both bare and styled samples used `MS Sans Serif`, logical height -11, weight
400, text height 13 and measured extent 76x13 for this string. A test-only
measurement temporarily selected and then restored the stock `SYSTEM_FONT`
in the same DC: its extent was 103x16. Thus a direct unscaled font substitution
would exceed the observed 76x13 drawing rectangle; this is a concrete layout
constraint, not proof that a proportional or otherwise safe adaptation is
impossible. Another `DrawTextA` call occurred in the same panel paint scope;
a follow-up bounded length probe showed it drew an empty string, so it is not
evidence of another visible non-label text in this sample. The shared parent
DC alone nevertheless supplies no public label-object identity for separating
other possible application drawing. The sample is one x86 application at 96 DPI;
at this stage no product change, clean package or release had been made. Actual glyph
fidelity, other labels and fonts, Unicode, layout/measurement preservation and
a content-neutral ownership contract remain to be established.

A further owned run queried the panel's public MSAA `OBJID_CLIENT` and received
four children. Each returned `ROLE_SYSTEM_WINDOW`, resolved through
`WindowFromAccessibleObject`, and corresponded to one of the panel's four
actual child HWNDs: the combo and three buttons. The direct enumeration did not
expose the windowless label as a separate accessible child. All four returned
objects were released, the temporary import observers were restored, and the
owned job exited empty. This limits the available identity evidence in this
specific legacy sample; it does not prove that every VCL version or a different
accessibility provider lacks a label object, and it does not justify inferring
paint ownership from accessible coordinates.

Embarcadero's [label documentation](https://docwiki.embarcadero.com/RADStudio/Sydney/en/Labels)
distinguishes nonwindowed `TLabel` from windowed `TStaticText` and documents
font, color, transparency and accelerator behavior. Its
[ParentFont](https://docwiki.embarcadero.com/Libraries/Athens/en/API%3AVcl.StdCtrls.TLabel.ParentFont)
and [AutoSize](https://docwiki.embarcadero.com/Libraries/Sydney/en/Vcl.StdCtrls.TLabel.AutoSize)
documentation also describes application-selected fonts and layout changes on
font or text updates. These are VCL object properties, not a demonstrated
public HWND message identifying the active label paint. They reinforce that a
blanket parent-DC font substitution could override application decisions or
break size and accelerator semantics. The observed empty first DrawText call
is not falsely treated as a visible non-label example.

A separate scratch observer linked the current project's unmodified `font.c`
into the test DLL and acquired its existing `win16_style_system_acquire`
variant for the actual selected MS Sans Serif font. On the same label DC that
variant measured 76x13, versus the stock System 103x16 and original 76x13;
the original selected font was restored and the process GDI object count was
123 before and after this one lease. The owned job exited empty and observers
were restored. This shows the existing metric-matched control-font mechanism
can fit this example, but it rescales System glyphs to individual original
character advances. The observation does not prove undistorted glyph fidelity,
correct shaping or multilingual behavior, or a safe way to identify this
windowless label among arbitrary application draws. It is not a product change.

The same test-only observer was extended to the Unicode GDI/USER variants and
run in a self-owned, task-local modern `cnc-ddraw config.exe` copy at 144 DPI,
using only its scratch INI. The first visible panel's MSAA client reported zero
direct children. The three Unicode imports (`ExtTextOutW`, `DrawTextW` and
`DrawTextExW`) were observed successfully; during the panel redraw there were
12 `DrawTextW` calls, including both `DT_CALCRECT` measurement and actual
drawing calls, using Segoe UI with 32-pixel text metrics. ANSI imports were not
present for the two tested ANSI entry points in that executable. These calls
were not classified as `TLabel` objects; the observation demonstrates that a
single panel draw scope can contain multiple Unicode measurement and drawing
operations. The original application configuration SHA remained unchanged,
temporary imports were restored and the owned job emptied. Other panels,
font styles, dynamic pages and physical input remain unexamined.

No safe generic label ownership contract was established, so this S leaves
application drawing native and records the exact evidence needed in TODO. This
does not establish that a solution is impossible. No product source or artwork
changed. An independent clean x86/x64 GCC 16.1/Ninja/ccache build passed the
source, byte-exact graphics, governance and ABI gates; its actual four-file
package passed 14 focused cases with zero skips in 80.27 seconds. The font
layout cases took 79.06 seconds (x86) and 63.51 seconds (x64); the other 103
standard cases and full integration were not run. The test-time issue is
separately recorded for immediate investigation without dropping glyph,
layout, DPI or resource checks.

The newly built binary hashes differed from the prior batch, so the tested
files were published to both `assets/` and `O:/winnt/system32` with all eight
destination hashes matching. The installed assets x86 and system32 x64 entry
points passed the bounded spin-model checks at 96/144 DPI. SHA-256: demo
`4fbeedd11e51f297e64f64186c7bb251fb9049b7dc241e53bff1636132f4dd49`, x86 DLL
`d21bf47d543e6088842f059aeb0ee6801aca188e88c2bbcff9556b4ea21ddc0c`, x64 DLL
`f35b82c641419a34582e32fa7d14c69275775eec0b4d56e1bff9bc61266223e5`, launcher
`e0bf369a000437f4718c25350cba5d533537e8556da4680deee9ae8c4371e36f`.
This is a rebuilt batch with no label-style implementation. Real physical
input, other font families and languages, and T18 full integration remain
unverified. P1 `c36f5ba` was pushed and all seven changed files were fetched
from GitHub and matched to their committed blobs. S29 closes as an
investigation with the label gap retained in TODO; the next T18 S addresses the
observed test-time regression before windowed `TStaticText` investigation.
T18 and M0 remain open.

## P181 recheck

A fresh owned styled MD5summer instance using the P181 assets x86 DLL was inspected with the earlier test-only label IAT observer, adapted to install its temporary UI-thread hooks from inside the owned process. The sample-specific checksum label DrawTextA call still selected MS Sans Serif height -11, text metrics 13 and extent 76x13; a temporary SYSTEM_FONT measurement was 103x16, and the private metric-matched variant measured 76x13. The observer restored the font and reported GDI count 102 before/after. This demonstrates the known label gap remains for this one application under P181, not general absence of all label styling or glyph-fidelity proof. It provides no new safe ownership witness; no product change is justified by text/geometry alone. The main window closed but the process wait timed out at 5 seconds (258); the full application test failed and its owned job reclaimed it. No scan was started. First scratch DLL build failed for missing library links and was rebuilt successfully before running. Scratch: md5-label-local-probe.c and p181-md5-label* logs.
