# M0 T18 S50 — bit button scope tests

S49's self-owned C fixture now raises a controlled exception within the parent
WM_DRAWITEM callback, with architecture-specific exception catchers from the
existing date test. The exception is observed, the button stays attached,
a subsequent clipped print and normal removal succeed, and all test DCs and
bitmaps are released. The parent executable also calls DrawThemeBackground
inside the exact owner-draw callback: the target does not become verified when
the caller is not its class module. A nested parent callback for two button
windows uses a shared memory DC; each becomes verified and both are destroyed.
The first attempted nested probe used the outer window's own DC; the inner
window was rejected because WindowFromDC identifies the outer window, as the
ownership guard requires. The tests do not prove every nested rendering
variant or all exception and GDI lifetime paths.

The new tests passed x86/x64 with the S49 package, then with an independent
clean snapshot. GCC 16.1.0, Ninja and ccache 4.13.6 built both widths, and the
ABI gate passed. Snapshot manifest SHA-256:
`1b9bb402d236118ebd540f83a148f4ea6c73b0204b1f37f317be0fd77b5a09aa`.
The same-batch package passed 12 focused cases with no skips in 6.46s at four
jobs. Other cases and physical input were not tested. Despite no product source
change, fresh rebuild hashes differed, so the four tested files were deployed
to assets root and O:/winnt/system32; installed-entry tests passed on both
widths and both destinations matched the following hashes.

| File | SHA-256 |
| --- | --- |
| demo.exe | `2c1b47541960868fe2271f2e0b701c12fa22586aa86a2cd479d900973f83aeb1` |
| ntw16s32.dll | `3c8d52fb55272f74675db8123bee5bcdd17a7ea7e7aeef37afda43edd160d38c` |
| ntw16s64.dll | `588f62ac929a4a994c7d2bd43bd22a64659d0af7b7824bbc7cae38d8e746ae14` |
| run16s.exe | `c88c3181e09f18a1a44809dbb0f6fe61cd46b506b8644538361d907ec1bf853a` |

Application callback, input and glyph ownership remain with the application;
product source, resources and ABI were unchanged. Full real-application
integration and remaining control families stay in T18, which remains open.
