# M0 T18 S64 P119: ordinary Delphi status-pane text

Real DelphiVCL4Python 1.0.7 x86/x64 at 144 DPI drew two ordinary status panes via system COMCTL32 DrawThemeText and UxTheme DrawTextExW. The previous package attached the status window and styled borders but retained Segoe UI text. The bounded implementation borrows the existing System font cache for witnessed native ordinary ASCII pane text only, with caller, DC, geometry, text and epoch checks; non-ASCII, owner-drawn and unsupported variants retain native drawing. The application font, icons, and original graphical resources are preserved.

The owned native version-6 fixture compared old and candidate pixels: old P118 failed both ordinary-pane assertions in both widths; candidate changed ordinary text while Chinese, owner-draw including application DrawThemeText, no-border and small icon pixels remained native. Query-time text mutation fell back. An artificial DrawTextExW exception exposed an existing x64 WM_PRINTCLIENT status-scope lifetime failure in both old and new packages. A bounded unwind cleanup now restores the state; both widths catch the exception, restore the test import, and paint afterward. The installed tests passed from both destinations. This does not establish all WM_PAINT or exception paths.

The clean C17 MinGW GCC/Ninja/ccache x86/x64 build passed ABI and byte-exact original-resource checks. The same four-file package passed 28 focused cases in 15.33 seconds with zero skips. Real Delphi x86/x64 candidate samples at 144 DPI produced matching status screenshots, target and launcher exited zero, and owned jobs emptied. The four tested files were published to assets and O:/winnt/system32, with all eight hashes verified. Other DPI, Delphi versions, physical input, additional exception sites, and the full T18 matrix remain unverified. M0/T18/S64 remain open.

| File | SHA-256 |
| --- | --- |
| demo.exe | `234f304cf6baa2295abcd318063d77f9549adb3b1c3001334e3445b4debe9d5c` |
| ntw16s32.dll | `a0b34bf8105b4269f1b3ae1940abac229961a2f6224f824c7affa53f33b3bc69` |
| ntw16s64.dll | `c258f05fcdc04b5bcd5d838e2c36f49306fd5da68c2bd3a4f29feb378ea3a4c0` |
| run16s.exe | `582b40a661e56c03866bf303f072a633a5b9e74f87026d5a1cc656331e71abce` |
