# M0 T18 S54 — VCL grid nonclient styling

Self-owned DelphiVCL4Python 1.0.7 x86/x64 instances at 144 DPI exposed
TStringGrid and TDrawGrid as runtime-owned windows with client edge and a
horizontal native scrollbar. Each returned range 0..127 and a synthetic
WM_HSCROLL line-right changed position 0 to 64; line-left returned to 0.
The published prior package did not attach either grid.

The product now considers only these exact same-thread runtime class names,
original parent/owner, a client edge, at least one native scrollbar and
non-RTL layout. After the first actual WM_PAINT it checks the public native
scroll information and installs the existing nonclient border/scrollbar
painter. The grid retains its own client painting and data; loss of the
parent or required role retires this exact attachment. No public ABI or
original graphics changed.

A direct call to the generic scrollbar installer alone changed no window
pixels despite intercepting input; it was rejected as proof of styling. A
scratch experiment additionally enabling the existing native-border branch
changed nonclient pixels. Its first 160-pixel client difference was reproduced
as an unflushed redraw: after both samples were synchronously redrawn, the
client difference was zero in all four width/grid pairs. In the final clean
candidate, both actual grid types and widths attached and removed, horizontal
messages changed 0 to 64, and same-window client captures before/after removal
were identical; outer captures changed. The sample windows overlap, so the
reported whole-window counts are not an unoccluded visual-quality proof.
The self-owned model verifies missing-edge fallback, both classes, public
scroll, client pixels, explicit removal, reparent retirement and cleanup;
the old published package failed its positive attachment assertion in both
widths. Native messages are synthetic and samples have no substantive cell
content. Vertical scroll, grid editing, nonempty/custom cell paint, drag,
additional DPI/runtime versions and physical input remain unverified.

Independent clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256:
`4fa7f94719b6a1a04ca93cbb3b25840f9ecfd5c165f7ebfad88edb56633934cb`.
PE and ABI checks passed, and 16 actual-package focused cases passed in
11.19s with four jobs and no skips; other cases were not run. The tested four
files were published to assets root and O:/winnt/system32; installed x86/x64
models passed and both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `404c9b7cd0f902a9d7e2d5286b16b89a6baebe6ef5edb02e0aea4742f9235898` |
| ntw16s32.dll | `63fcd0485225161b0dd06a5517e773208f7d51506f69edfd558e101591d6159d` |
| ntw16s64.dll | `b210d23660cf2be495413cd88379aed45c5760a2bac59739dc1aa17754a3f00c` |
| run16s.exe | `801ce24c0602188bfc46529261029a38d2191bef64a54ebbf130197153e202f8` |

Search `rg -n 'grid_identity|ntw16s_grid_valid|win16_style_grid_install_created' src`
identifies admission, ongoing guard and the existing border reuse; unrelated
controls retain their former paths. Full T18 integration and uncovered grid
variants remain open; M0/T18 remain open.
