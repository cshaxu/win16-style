# M0 T26 S1: shared UWP surface presentation

`lib-style/uwp/surface` now also owns frame alpha composition, hole region
creation/ownership transfer, layered-window submission and relative z-order
placement. The Win16 UWP frame invokes this shared routine after its own paint;
style-specific appearance, geometry and attachment remain in the Win16 adapter.
The caller supplies the owner, surface, frame bounds, validated client hole,
bitmap and detachment flag. This is not a Win2000 UWP renderer.

The clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed; source snapshot
SHA-256 `fa6bc55f86cc232a24b0d0c4dc8f6452ad52ff4780be1c9d269d54a8b9be97a6`.
Eight files were published to assets and O:\winnt before post-publication tests.
Sixteen focused checks passed: both widths of shared surface geometry/bitmap,
instrumented alpha/region ownership and failure paths, Win16 UWP layout,
native title selection, target discovery, self-owned cross-process dispatch,
published Win16 menu and Win2000 scrollbar geometry. The instrumented test
substitutes system calls; it is not a live layered-window success. The shared
archive contains no unresolved Win16, Win2000 or product-private symbols.

The full Win16 UWP frame worker still failed in both widths at its first attach
with error 5, matching the previous baseline documented in the preceding P.
A temporary x64 diagnostic showed geometry, DIB, SetWindowRgn and
UpdateLayeredWindow succeeding, followed by SetWindowPos failing with error 5
when its insert-after window belonged to another process. An additional
self-owned-window z-order probe allowed the first attach to succeed; after
maximizing, the insert-after window again belonged to another process and
SetWindowPos failed. This narrows the failure but does not establish why
Windows rejected that z-order operation. No fallback or global z-order behavior
was changed. Full layered-frame lifecycle, actual UWP/WinUI applications,
physical input and visual acceptance remain unverified. Both published
destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `cbaa011bab5817ac6f65f4f3e1d93638dccfba43a057efc3f6f0939c681761fa` |
| ntw16s32.dll | `452a6ad6926ee43896d9e5597e8f122482448ac217e33bd40babd156f37a0930` |
| ntw16s64.dll | `f9ab4d36d9a8d6fa844fd0655194b8cc61f7b718af291f0a693c8804d2e4e0a2` |
| ntw2ks32.dll | `18677357e81b9733283939870c937ef5154bab74e96ea7c2be0c6750a18dfaf2` |
| ntw2ks64.dll | `813307b51a3381b049ac19989b60b342cdf10f9c7269ab5d570f1041fe2c7700` |
| ntwshk32.dll | `19d8ea1ebf19290069b5e20ab33cf2af92afbf70c22bedf9e3c1974e15a8e453` |
| ntwshk64.dll | `a4b98e2044181a896e9ec33c54a617029d2b27762f6b72bc4395f16f46b135e5` |
| run16s.exe | `27356a547b39ad70553c2ef9c060b5b9661aa26a70d497769f0651ee7d420e0c` |

T26 S1 remains open, including the full inventory and cross-style adoption.
