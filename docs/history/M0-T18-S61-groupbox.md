# M0 T18 S61 — VCL group-box drawing investigation

Self-owned DelphiVCL4Python 1.0.7 x86/x64 TGroupBox samples at 144 DPI were
runtime-owned custom windows, not native BUTTON superclasses, and were not
attached. During their redraw the runtime called FillRect with a parent-sized
rectangle, a translated viewport and a group-window clip; it made no observed
DrawEdge, DrawFrameControl, Rectangle or PatBlt calls in the instrumented
path. All instrumented imports were restored and the owned instances closed.
A separate self-owned diagnostic build observed the runtime call
DrawThemeBackground part 4 for rect (0,12)-(250,120) on the actual group DC in
both widths. The separate S53 radio-group experiment showed a direct black
frame at that boundary crosses the title. It is an analogous risk, not proof
that an identical prototype was run for TGroupBox. No title-safe content and
clip contract was established, so the group remains native rather than being
classified as supported. Other versions, child content and physical input were
not tested. There is no product source or graphics change.

Independent GCC 16.1.0/Ninja/ccache 4.13.6 clean dual-width snapshot manifest
SHA-256: `3aeb33cbcd1042e32ea1dff146446aa0ea8552c3ff9ef9661a69691c92665a11`.
Both PE/ABI gates passed; 12 focused actual-package cases passed in 6.13s at
four jobs, no skips. Other cases were not run. Since rebuild hashes changed,
the same four tested files were published to assets root and O:/winnt/system32;
installed x86/x64 tab models passed and both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `c9fbb4b4925d964a2731174e9b56cb3f0faa07a791e44e2e95a0074828455804` |
| ntw16s32.dll | `53cf06a94f5e1025c2809930d6f522d2dae7195aba2303970fad50c8911c76be` |
| ntw16s64.dll | `e9bd36b82a147e96be6121a1fca13dd09f60ccfd32ab4e11b7a61c7192163798` |
| run16s.exe | `a246f5b3ae7fd8e0939cea0849ce96eead99987cdbcb5ccc12e42ac6b5f5f9c8` |

The group-box styling gap remains tracked; M0/T18 remain open.
