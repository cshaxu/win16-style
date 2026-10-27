# M0 T18 S55 — populated grid and vertical scrolling

A self-owned DelphiVCL4Python 1.0.7 TStringGrid was populated with two
independent strings, 25 rows and 8 columns, and paired with a similarly sized
TDrawGrid. Both actual windows had horizontal and vertical scrollbars. At
144 DPI, x86/x64 product-attached grids responded to synthetic WM_HSCROLL
line-right with 0 -> 25 and WM_VSCROLL line-down with 0 -> 7. The observer
returned the scroll position, captured the client, removed style and captured
again; both grid types in both widths had zero changed client pixels. After
that sequence, TStringGrid.GetCell returned both original strings. The sample
capture displays truncated text because the native column is narrow; it does
not prove every glyph or cell editing behavior. The probe did not simulate
physical drag or a custom drawing callback.

The self-owned C grid model now covers both native scrollbar axes with finite
line movement, range/position, client preservation, removal and reparent in
both widths. Its painted marker supplies a client-pixel oracle but is not a
real Delphi custom cell. Product source, original resources and ABI did not
change. Other runtime versions, DPI, actual keyboard editing, custom cell
paint and physical interaction remain open.

The independent clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256
was `abfcea7becfcb90431a481b091823541b0fde2798514b9bb2bfabd73f42dbc37`.
Both PE/ABI checks passed; 12 actual-package focused cases passed in 6.38s
at four jobs, no skips. Other cases were not run. Rebuilt hashes differed,
so all four tested files were published to assets root and O:/winnt/system32;
the installed x86/x64 model passed and both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `6ec8fad2fd761cb83ef2e392d1edbe0c6c279be62d3bcfe3e1ab04d687b31d8f` |
| ntw16s32.dll | `e8463e0cd41ee53d47acad918328e08904da162b8be520b3420f79506658e3c7` |
| ntw16s64.dll | `c01fa4b2f6aeccee8f455d725f6c32bff415bd78cbb55820ad66c5d9c05e2bd9` |
| run16s.exe | `fcd64d2a18f484f25052a87524c483d202139a865427d3c0b44e9fd538b91dbc` |

T18 and M0 remain open; test-only validation does not close the T integration.
