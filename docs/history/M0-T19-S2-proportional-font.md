# M0 T19 S2 — proportional System bitmap trial

The user requested a test build that fits System bitmap glyphs proportionally inside existing control text cells. In `system_acquire`, control and VB glyphs now use one fit scale within each source advance and line height, center horizontally and align the ascent where the cell permits. Pixel rounding may change the ratio by one pixel; narrow cells may produce visibly smaller glyphs. Original character advances, line metrics, application font handles and layout remain the existing contracts. Menu/calendar `menu_dpi` and title paths remain unchanged. The existing COLORONCOLOR and VB BLACKONWHITE sampling choices remain; this trial does not claim that bitmap downscaling cannot merge strokes. Existing resources and notices were preserved byte for byte. CP1252 and native fallback limits remain; actual appearance awaits the user's evaluation.

A new C pixel-reference test covers native and VB variants, limiting width, preserved advances, ascent, baseline placement, pixel identity and GDI cleanup on both widths; the P208 old implementation fails on both. The existing actual-DLL font layout pixel oracle was updated to the new behavior, not removed. Source search `rg -n 'system_acquire|win16_style_system|StretchBlt' src/ntw16s-dll` located the common font generator and separate menu/calendar paths; the unrelated legacy compatibility font and image/graphics scaling were not altered.

`tools/Build-Package.ps1 -Task 19 -RunName s2-font-final` produced one clean snapshot with GCC 16.1.0, Ninja and ccache 4.13.6; source/resource identity and x86/x64 ABI passed. `tools/Test-Package.ps1` ran 20 focused cases, 0 skipped, 93.2 seconds including actual-DLL font-layout x86/x64, source-linked pixel reference x86/x64, VB font/controls, menus, owner-draw, launch/propagation and package checks. Full matrix, physical input, other DPI/languages and appearance acceptance are not claimed. Initial sandbox private desktop access returned error 5; the isolated tests passed outside that sandbox. The standalone old-fixture first compile lacked an include path; corrected old x86/x64 fixtures each failed as expected. A self-owned private-desktop MadShow probe found the old and new windows, but `GetWindowDC`/`BitBlt` failed with error 6 in both; job cleanup was performed and no screenshot or visual pass is claimed.

The tested M0 T19 package was published and hash-verified at `assets/` and `O:/winnt/`:

| File | Architecture | SHA-256 |
| --- | --- | --- |
| run16s.exe | x64 | `f44bc4c6b1030eaa89eb72639bb9a1c7b7512794bcd7bfd36d7726f4a3125fd2` |
| ntw16s32.dll | x86 | `03811590d1204bfdd88bc25d406286f65268f7bed7175369faffbb2b05186839` |
| ntw16s64.dll | x64 | `0d0cfbc7439025875a000e96a118c8ac5d75e92df782f28796f7869dfed8c026` |
| demo.exe | x64 | `b45ef512c755961fab6ac1be5bab475e44f25718d362ef9da1679f64f7a2ff31` |

This is a trial delivery, not a claim of user-approved visual quality. No extra product or public ABI was introduced.
