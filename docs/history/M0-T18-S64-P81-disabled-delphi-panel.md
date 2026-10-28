# M0 T18 S64 P81: disabled Delphi panel outer bevel

The existing bounded TPanel outer-bevel painter rejected disabled panels. Actual DelphiVCL4Python 1.0.7 samples still draw the same complementary system-color outer Polyline pair when disabled. The adapter now retains the enabled state observed at the first line and requires it to remain unchanged through the second line and before the replay; parent, exact geometry, DC, caller and paint-scope checks remain. Only the witnessed outer shadow line is replayed in black. Framework/app painting, background, text, child and input ownership remain unchanged; no public ABI or original graphics changed.

Real x86/x64 144-DPI disabled Delphi panels were compared against the previous released package. In the actual tested candidate, raised and lowered samples changed 369 and 368 RGB pixels respectively, all at the outer edge; a no-bevel sample changed zero. For a sample with a child button, parent-DC captures changed 369 outer pixels and zero in the child rectangle; child focus was false and BM_GETSTATE zero in both captures. Parent-DC pixels do not independently prove the child-window image. Target processes exited after WM_CLOSE; the abbreviated driver did not check exit codes or job active counts. No physical input, historical disabled color comparison or other DPI was established.

The self-contained x86/x64 C models passed disabled-to-enabled transitions, a mid-pair enable change with no erroneous attachment, reentrant paint, wrong caller, incomplete/custom pair, parent changes, clipping and cleanup. The old published DLL failed the new disabled positive check in both widths; a deliberately removed state guard was caught by the x86 mid-pair negative. Three focused paints reported GDI 5 to 5 on each width, not a whole-process leak proof. The first scratch compilation lacked the fixture include path and was retried; only the corrected runs count.

Clean same-snapshot C17 MinGW GCC 16.1/Ninja/ccache 4.13.6 x86/x64 builds took 24.79/30.30 seconds; source, governance, ABI and byte-exact original resource gates passed. Snapshot manifest SHA256 `67b312b0ad8e73553888459a8bed7833de8b8cb72b53156ca15c36256d49513c`. The actual four-file package passed 18 focused cases with zero skips in 13.51 seconds, including dual-width ordinary and DPI-aware panel, group/radio frames, menu cancellation and required package baseline. Full integration and physical input remain untested. The exact tested package was published to assets root and O:/winnt/system32, eight hashes matched, and installed panel model passed from both destinations in both widths.

| File | SHA256 |
| --- | --- |
| run16s.exe | `dde103cd87f135feb99b9c3ae741048594819ea872d28eff31db1829994a77bf` |
| ntw16s32.dll | `0dc7e799d7eb63332ef9dfa27ffe66d4033ff0bdc4c82ac6343fb2312c3f8f4b` |
| ntw16s64.dll | `abe433cd1d8690f97ab4bf8781efb68cfbe18129e13ff772a40d60d21f8ea1c5` |
| demo.exe | `83b2cb98d984ff69fd95b3bb761c579f6226fc183d0d5f7d9e57f5a87156259e` |

Production `attachment.c` +5/-5, net zero. Search `rg -n 'TPanel|panel_polyline|Polyline' src/ntw16s-dll/attachment.c src/ntw16s-dll/injection/window_titles.c` confirms the existing original-first hook and bounded TPanel receiver remain in use; no independent painting path was replaced. The separately investigated second inner ring remains unimplemented because the scratch prototype can alter same-module application drawings; see [investigation](M0-T18-S64-delphi-panel-inner-investigation.md). T18/S64 remain open.
