# M0 T26 S1: first shared foundation

This is a partial implementation within the continuing S1 audit. The complete
Win16 window, control, dialog, UWP and WinUI reorganization remains open.

`src/lib-win32/style-common` and its three tests moved to `src/lib-style/common`
and `tests/lib-style/common`. CMake and all current includes use the new paths.
The existing global-window lifecycle, DWM, frame and menu-hit mechanisms are
used by the existing Win16 and Win2000 paths; this move alone does not merge
their remaining separate menu or rendering implementations.

The bounded print page-range parser and formatter moved from the Win16 dialog
library into the independent pure-C `style-dialog-common` target. Names no
longer carry a product prefix. The Win16 extended print dialog calls the new
interface; its contract and existing parser tests were retained. The test now
links the common target directly without a Win16 dialog dependency. Windows
2000 print dialogs are not implemented by this change.

Incremental x86/x64 GCC/Ninja builds and direct tests passed for the range
parser, Win16 dialog standalone link, style-global lifecycle, menu hit and
Win16 control standalone link. The shared archives' undefined-symbol audit
found only system and C-runtime symbols. Source and governance checks passed.

The Win2000 DWM restoration probe failed on both candidate architectures; an
x86 comparison against the pre-migration T25 published DLL failed with the
same observed state. The screen-pixel bridge probe was intermittent in both
the candidate and pre-migration published DLLs. Neither condition is recorded
as fixed or as a passing visual restoration check. The direct dialog-link test
initially lacked its required capture-directory environment and returned 41;
with its documented self-owned temporary directory it passed on both widths.

The second clean build used MinGW GCC 16.1.0, Ninja and ccache 4.13.6 for
x86/x64; the source snapshot SHA-256 is
`38f49a0c6282e3f5d9f608b4055bd49acf4cc0585dc2684dd63d4e903601c430`.
The eight-file T26 package was published to both `assets` and `O:\winnt` before
testing. All eight hashes matched the candidate and both destinations after
testing. Direct post-publication tests passed in both architectures for the
range parser, Win16 dialog link, global-window operations, menu hit, Win16
control link, Win16 DWM restore, the style bridge sample and published Win2000
scrollbar geometry. The Win2000 DWM lifecycle test returned 3 in both widths;
the earlier x86 T25 control had the same failure. Full regression, physical
input and user visual acceptance were not performed. No private desktop or
computer-use was used. S1 and T26 remain open.

| Published file | SHA-256 |
| --- | --- |
| demo.exe | `2dc9ceebfdab12527101634411da1733a49f879f6405fc8d44d702c76c0cdf93` |
| ntw16s32.dll | `9566f6981143e8be014f1ecc2bf0ad0614cef86d9d7abc89da12ff687f60a8c4` |
| ntw16s64.dll | `d2af06ac8668ce712bcc2d3b62b5ca5d9f347848372eb04fbed8676a9c6a26ef` |
| ntw2ks32.dll | `083e9d87f4f01fda82979f9abaf82bb97e114d10d35b38732c6e852ce6edce7b` |
| ntw2ks64.dll | `6ea5f64d3782c9affb160f0d5325c4d389016432211fdba7c3dc3f851e4e194e` |
| ntwshk32.dll | `b7941eb38a3042f0827e394a337af50f98535f5969cc2152c7456067e984ddd2` |
| ntwshk64.dll | `d60ced9c874c486a6c12e0835a48cb57098e4a1fab07cc9f17226253c2a18128` |
| run16s.exe | `8f83193e11cba68e74f24f16bb0a2036959c34963b6c2d514251f33da339ac95` |

The first clean T26 package compiled successfully on both widths but failed
the package version gate: the five product resource files still declared T25.
That package was not published. The resource declarations and CMake project
version were corrected to T26 before rebuilding; this is a version correction,
not evidence that the first package passed.
