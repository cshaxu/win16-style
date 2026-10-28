# M0 T18 S64 P120: ordinary borderless status pane text

Self-owned DelphiVCL4Python 1.0.7 x86/x64 at 144 DPI used public SB_SETTEXTW with SBT_NOBORDERS for an ordinary pane. P119 styled the first pane but left the borderless pane in the original font; screenshots showed the mismatch. Both targets and launchers exited normally and the owned jobs emptied. The product now keeps witnessed text rectangles separate from border rectangles, allowing the existing bounded System-font lease for ordinary ASCII borderless pane text without adding a pane frame. Owner-draw and non-ASCII paths remain native.

The owned x86/x64 native v6 fixture detected the old package at the borderless text assertion; the new package changed text while the border pixels remained identical. Unicode, owner-draw, icon, application font, mutation and print exception checks passed. A pre-existing border-focused full-frame comparison initially failed because it assumed borderless text would remain native; that test now uses the System stock font while preserving its entire pixel comparison, and the separate font test checks the changed text and unchanged perimeter.

The clean C17 MinGW GCC/Ninja/ccache build passed both architectures, ABI and original-resource checks. The same four-file package passed 28 focused cases in 13.33 seconds, zero skips. The final package ran the real samples in both widths, and four installed native tests passed after publication to assets and O:/winnt/system32; all eight file hashes matched. Other DPI/runtime versions, physical input and the full T18 matrix remain unverified. M0/T18/S64 remain open.

| File | SHA-256 |
| --- | --- |
| demo.exe | `bc8921abe34a57cb52589e4469a1988257ec6f20f9241499591d3cd396ef8a14` |
| ntw16s32.dll | `42a6aef0b6989f83d9cf23bb552ff330158d713af00355e60713f967b31d5bd5` |
| ntw16s64.dll | `b445e815714d57cbae09166bb61a2ca498af3e165ba85c17cc9a366c0067ff02` |
| run16s.exe | `6227953af58b2e359ed5a2fe00e302c7786433ce183cd06b26c05d3ec0259d90` |
