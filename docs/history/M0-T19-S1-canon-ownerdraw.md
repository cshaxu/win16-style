# M0 T19 S1 — Canon MF Toolbox owner-drawn icons

## Scope and evidence

The user reported eight missing images in `C:/Program Files (x86)/Canon/MF Toolbox Ver4.9/MFTBOX.exe`. The independently created isolated x86 instance used an identical copy (SHA-256 `011fbdd4347074ef90d207d582b693ae4de8e9a52665ccf73fdcf28991e93542`); the existing user instance and scanner were not operated. Window enumeration identified the eight visible image areas as native `Button` windows with `BS_OWNERDRAW`; bitmap `Static` windows had no image handle and are not the demonstrated cause. The target loaded the tested `ntw16s32.dll` from the candidate package.

A native C transition fixture starts with a normal attached button and changes it to `BS_OWNERDRAW`. P207 reproduced the failure on both architectures: attached before=1 after=1, `WM_DRAWITEM`=0 and 0 of 4800 expected pixels. The fix lets native processing finish and retires the existing painter for an exact native `Button` class that has become owner-drawn. The final test has attached before=1 after=0, `WM_DRAWITEM`=1 and 4800/4800 pixels on both architectures. In the Canon instance, 48x48 `GetWindowDC`/`BitBlt` captures of each of the eight areas matched unstyled pixel bytes exactly with the final candidate; old P207 differed in every area and showed only black and white. This is strong causal evidence, but the actual Canon startup style-transition event was not independently traced. `PrintWindow` returned zero in this private desktop and was not used as visual evidence. No scanner command was issued; real hardware and physical input remain untested.

The first broad class predicate matched VB superclass roles and the `vb-owner-option-32` actual-package test failed. That candidate was not published. The predicate was narrowed to the literal window class and the final VB tests passed. The first clean package also failed its version gate due to T18 version resources; the final snapshot updates all three version resources to T19. Both failures are retained here rather than counted as passes.

## Build, test, and deployment

`tools/Build-Package.ps1 -Task 19 -RunName s1-canon-final4` used one clean source snapshot, Ninja, ccache 4.13.6 and MinGW GCC 16.1.0 for i686 and x86_64; source/third-party/resource identity and both ABI gates passed. Package PE versions are M0 T19, DLL public ABI remains unchanged, and the 18 original resource tables per DLL passed identity verification.

`tools/Test-Package.ps1` ran 18 focused actual-package cases in 39.9 seconds, jobs=1, with zero skips: mandatory packaging/ABI/demo/startup/propagation cases, menu cancellation, VB graphical choices and controls, and both owner-draw transitions. This is focused coverage; the full matrix, physical mouse/keyboard input, actual scanning, other Canon versions and other applications are not claimed. The actual Canon comparison was rerun with the exact tested package after the focused suite, without triggering a scan.

The tested four files were published to both project `assets/` and `O:/winnt/` and verified against the package:

| File | PE | SHA-256 |
| --- | --- | --- |
| run16s.exe | x64 | `b13fe7106aee0dd7b7d5bef84a0cc4941eb899d2a32ee5ee73905d945f4f09de` |
| ntw16s32.dll | x86 | `9e4768faa615e0af9d621ec3c73a3e48906e63b398db895e8cc92c7fa96a3036` |
| ntw16s64.dll | x64 | `1490fe8f01ad730bfc492474e25b4e78baf8bd1270643a0e723632c6738f6b40` |
| demo.exe | x64 | `877ac356eca2cd34ba7de2b02f899f9690bff79e432a1994c5922a769aff2b04` |

The original graphics and provenance resources remain byte-identical; no new component or public ABI was introduced. Canon app copy and task probes under `build/t19-s1-canon` are temporary owned scratch, not project assets.
