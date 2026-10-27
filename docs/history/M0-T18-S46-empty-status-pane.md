# M0 T18 S46 — empty final status pane

S45 retained a size-grip final pane natively when its ordinary text was empty.
A phase-marked self-owned native probe on ComCtl32 6.16 showed that both widths
still call the verified native DrawThemeText entry for an empty final pane;
its rectangle maps to that public pane. The first variant probe accidentally
ran all modes despite accepting a mode argument; the corrected single-mode
probe and phase markers supplied the result. Icon-bearing panes also called
this entry, but their drawing order and preservation require separate study.

The existing guarded pre-text frame now accepts an empty ordinary, icon-free
pane. All other identity, native-caller, geometry, clipping, epoch, same-thread
and size-grip checks remain. Icon and owner-drawn panes stay native. No public
ABI, original glyph or other product component changed.

The x86/x64 self-owned module model at 96 DPI found 590 changed pixels only
on the final-pane edge for ordinary and empty modes, 79 inside a restricted
clip and zero outside. Icon/owner-draw modes remained native and retained
owner data; reentrant removal during icon query was safe for both ordinary
and empty modes. The S45 released x64 DLL failed the new empty positive case.
The independent final package on DelphiVCL4Python 1.0.7 x86/x64 at 144 DPI
showed 440 first-pane and 804 empty-last-pane changed pixels, zero outside the
pane edges, in both screen and WM_PRINTCLIENT comparisons. The empty text and
owner-draw data were restored. [Evidence](../etc/evidence/delphi-s46-empty-pane.json)
is one runtime and one style, not universal support. Other versions, physical
input and full T18 integration remain untested; S40 and S41 caveats remain.

Independent clean GCC/Ninja/ccache builds and ABI checks passed; the actual
four-file package passed 26 focused cases in 8.39 seconds with no skips.
Published assets and system32 files match these tested hashes, and the
installed dual-width pane tests passed. Original resource identity and source
provenance checks passed.

| File | SHA-256 |
| --- | --- |
| demo.exe | `869d60a8dae85591fa2be553ba12a50243db14bdb27473bae40e321e60946b65` |
| ntw16s32.dll | `8ada779c76e2e190762b4eced780339d1fbd8541f8673ed8d721935a8882134c` |
| ntw16s64.dll | `c850bec45c5eeb6369aa3c162ebfc90df91c30ecb111c92725c16569c2536007` |
| run16s.exe | `fd56f3b38f55be14353618c20f0ea93c5f442d35f9fbd9c84bc76caf14b165ce` |

T18/M0 remain open. Icon and owner-drawn last-pane styling remain unimplemented.
