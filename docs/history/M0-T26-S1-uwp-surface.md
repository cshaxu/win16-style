# M0 T26 S1: shared UWP surface primitives

`lib-style/uwp/surface` now owns the prior Win16 UWP frame's host/title
rectangle calculation, maximized work-area clipping, and reusable 32-bit DIB
allocation, reuse, resize and disposal. `win16-window-uwp/frame.c` calls these
primitives directly; Win16-specific geometry, input, rendering and menus stay
in the Win16 adapter. The shared source has no dependency on a style or
product. This does not implement Win2000, XP, Vista or Win7 UWP rendering.

The clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed; source snapshot
SHA-256 `3e98e733d09d56b7b2148b61b42b9db9ca0d6e25c4c4744f6655167152770cbd`.
Eight files were published to assets and O:\winnt before tests. Both widths
passed shared bitmap reuse, resize and invalid-size behavior, normal title
bounds, hidden-title handling, Win16 frame geometry, native title discovery,
UWP target discovery, self-owned cross-process dispatch, the published Win16
menu and Win2000 scrollbar geometry: 14 focused checks. The standalone surface
test was subsequently extended with maximized work-area clipping and passed
both widths after incremental test-only rebuild; product sources and published
binaries were unchanged. The shared archive has no unresolved Win16, Win2000
or product-private symbols.

The existing full Win16 UWP frame worker was invoked directly on the current
desktop, bypassing its forbidden private-desktop launcher. It failed at the
first nonclient attach with error 5 in both widths. A control rebuild using the
previous committed frame implementation failed at the identical line and
error; skipping that branch also made the previous implementation fail at the
normal title attach with error 5. Thus the failure is not evidence of a new
regression, but neither is it a pass: its cause and complete layered-frame
lifecycle remain unverified. Real UWP/WinUI applications, physical input and
visual acceptance remain unverified. Both published destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `ab3c879b930ba557c5372412cfa7b6ac05f2c5344642f01584bf40d9a8e7acb7` |
| ntw16s32.dll | `bfbdaa01b0392e8d6b630250b1fdecc104a5b7c4e9d30eacd422d65042e4d68d` |
| ntw16s64.dll | `e634ba788bbf2ab92cb82461f19c7e68c88170d9400093ca35d320047661d764` |
| ntw2ks32.dll | `42c22c4ff4af01d8b77c5ed69e6a9d891487999be20f02921580c657258f4fb1` |
| ntw2ks64.dll | `f2e0944f3db6ff363dae9a58f5b5de00109c482c072b24160d240f35bbaa80d6` |
| ntwshk32.dll | `ffcb9db66b5f59c671c8436f16eb8cb69f0d60d4e51aa63af3bccfe40ed7ac65` |
| ntwshk64.dll | `075b9a5ffed52a6e64e2719879e8790b00c53ffd6e21de1862ab7ca3c3aa7d10` |
| run16s.exe | `6431e5e520dd92aa845d66c1c1a5531ced645fee384674724f373efd56e9d2f1` |

T26 S1 remains open, including full inventory, actual cross-style adapters and
the previously reported Win2000 DWM restoration problem.
