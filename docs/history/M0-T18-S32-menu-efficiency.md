# M0 T18 S32 — menu regression efficiency

The existing x86 actual-menu matrix took 75.74 seconds in an isolated native
session; S31 package runs took 73.92/72.82 seconds for x86/x64. A scratch-only
instrumented copy located repeated approximately two-second delays in window
activation after a few scenarios; pixel checks took about 0–16 ms. Showing
without activation, relying on SetFocus, and deactivating before destruction
all retained the delay. This is observed timing, not a proven Windows internal
root cause.

The test now keeps one visible, owned activation anchor per DPI context. After
all original scenario assertions, it activates and verifies the anchor before
removing the style and destroying both scenario windows; it destroys the anchor
at the end of that context. Original display, cancellation, focus, capture,
hover, menu restoration, per-window border, shadow and resource checks remain.
The product and public ABI are unchanged. Test source diff before staging: 10
added and 2 removed lines, net 8. Search used `rg -n -C 3 'GetPixel|border_probe|WM_ENTERIDLE|SetTimer|TrackPopup|capture_menu|for \(' tests/ntw16s-dll/win16_style_menu_cancel.c`;
this test is the scoped menu-cancellation matrix; other tests were not silently
changed. Both baseline and scratch candidate recorded 30 case passes, 42 border
checks and 42 associated hidden-shadow checks; x86 candidate took 6.04 seconds.
The old implementation is the measured slow control, not a product defect
negative test. The anchor affects test-only activation between cases; it is not
proof that all Windows environments have the same performance or focus behavior.

The first detached build worker did not finish after its caller exited and is
not counted. The synchronous `Build-Package.ps1 -Task 18 -RunName s32-clean2
-Worker` passed clean x86/x64 GCC 16.1.0, Ninja, ccache 4.13.6, source,
resource identity, governance and ABI checks. The snapshot.json file SHA-256 is
`342600d0d8496195ab23651eaf0413809ab7d585450151bf1aa43d331552a31d`.
The actual same-batch package passed 20 selected cases, zero skips, in 34.03 s;
101 cases were not selected. Menu x86/x64 were 5.55/4.97 s and include all
15 cases in each DPI context. VB controls, VCL superclass/static and spin
models, propagation and demo cases passed. Installed assets x86 and system32
x64 menu matrices also passed. No physical input, entire integration matrix or
other desktop environments are claimed. Normal sandbox CreateDesktop returned
error 5; native tests above ran with desktop access. The previous intermittent
VB timeout and font stress limitations remain documented separately.

All four newly built files were tested as one package, then published to
assets and O:/winnt/system32; eight installed SHA-256 values match:

| File | SHA-256 |
| --- | --- |
| demo.exe (x64) | `01de5c9d5967f48eb068595023e913c745d2d8f0578bd23250b6f827bcebab52` |
| ntw16s32.dll (x86) | `9bc261bea67d52f36593154bb30e9397ecd55c68e414edbc5eca65d2448b657d` |
| ntw16s64.dll (x64) | `48b2db31ecc05957881ae8b299479034526d1a7cb2374d0af6f9c293bca7cdfd` |
| run16s.exe (x64) | `40e2eb172b81a05574a5049cd5ceb1f5dd2fb4e53d1667dd4bbf658c67ffd929` |

P1 `6e62c28` was pushed; all eight changed files, including the four binaries,
were fetched from that actual GitHub commit and matched their Git blobs. S32
is closed. M0 and T18 remain open; final T18 integration and coverage audit
continue under S33.
