# M0 T18 S60 — witnessed VCL tab control

The S59 actual DelphiVCL4Python 1.0.7 runtime-owned TTabControl sample was
unattached although it forwarded WM_PAINT to the system SysTabControl32
procedure. The DLL now considers only same-thread exact TTabControl children
and waits for the runtime-owner's actual native WM_PAINT forwarding; it binds
the existing public tab protocol and renderer only after validation. The
original parent identity is guarded; a later reparent retires the attachment.
Unsupported styles follow existing native fallbacks. Labels, selection,
application callbacks and image ownership remain with the application.

The final actual package on x86/x64 DelphiVCL 1.0.7 at 144 DPI attached a
real two-tab sample and retained its public count, selected index and geometry.
Same-window client images before and after removing the style differed by
4549 pixels per width. This is pixel evidence of a change, not physical input
or a complete visual proof. The self-owned C model covered fake WM_PAINT
forwarding rejection, normal selection, an owner-draw fallback, reparent and
cleanup on both widths. The previous deployed package failed its positive
attachment check in both widths. Initial test fixture failures were a missing
commctrl include and an incorrect expectation that owner-draw must attach;
corrected tests now require native fallback. Physical keyboard/mouse, images,
other orientations, other versions and full task integration remain open.

The independent clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256
was `067c7bab90163ba5e16dc5e6422f718ada92de2698f981573395efb19df0021f`.
Dual-width PE/ABI checks passed; 16 focused actual-package cases passed in
7.31 seconds at four jobs, no skips. Other cases were not run. The tested
four files were published to assets root and O:/winnt/system32; both installed
x86/x64 model tests passed and both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `e37b5a2cd18d047b8b0f10ae7c93355c5e30edcea59845f1ad8dd7431b410abc` |
| ntw16s32.dll | `06219799660be206163f4829e5b759af02804e7e26ff6d184b587d0d4babdd46` |
| ntw16s64.dll | `cd5a54ee40f904d62c45480a7157b4eb18f2f2db460c8a96bebcaa593ae104f6` |
| run16s.exe | `44d5012b2399d8d016c54e9aa8da2b5e8dcc7baae2a8c4b94154e952fb4d9f9a` |

Original graphical resources and public ABI were unchanged. Search
`rg -n 'tab_candidate|vcl_tab_valid|bind_verified_tab' src` covers the new
witness, binding and lifetime guard. M0/T18 remain open.
