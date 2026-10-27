# M0 T18 S56 — ControlBar and toolbar drawing investigation

Self-owned DelphiVCL4Python 1.0.7 x86/x64 instances at 144 DPI contain a
runtime-owned TControlBar and a TToolBar child. Neither is attached by the
current product. The container redraw observed window and memory DC fills,
including a parent-coordinate rectangle with a translated viewport; DrawEdge
occurred on a memory DC. This does not identify an independently owned
container frame or justify repainting the app's content. All observer imports
were restored and the owned instances closed.

For a ShowCaptions text-only TToolBar, native TB_BUTTONCOUNT, TB_GETMAXSIZE,
TB_GETBUTTON and TB_GETBUTTONINFOW responded; its runtime forwarded
WM_PRINTCLIENT to the actual system ToolbarWindow32 procedure during redraw.
The sole text button had bitmap -2, command id 0, text S56 and state 4.
A self-owned parent observer saw the application's CDDS_PREPAINT return 0;
when it temporarily requested item drawing, both widths delivered
CDDS_ITEMPREPAINT with item id 0 and the application's return 0. This is a
candidate contract, not evidence that styling or command interaction already
works. Existing toolbar painting rejects id 0; no global relaxation or product
change was made. Images, multiple/duplicate IDs, other toolbar kinds, input,
DPI and the outer container require independent evidence.

Product source and original resources were unchanged. The independent clean
GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256 was
`7be9593b661aaa0c5be8b87b85b294bdb67e048f28b402d9e734131028d16c5b`.
Both PE/ABI checks passed; 14 focused actual-package cases passed in 6.40s at
four jobs, with no skips. Other tests and physical input were not run. Fresh
hashes changed, so the tested four files were deployed to assets root and
O:/winnt/system32; installed x86/x64 grid models passed and hashes matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `d3bcfa4ba19b0b0a063f29d6e1a563b4978bd7c83b2812bfc2482dff7210e844` |
| ntw16s32.dll | `cff207546eedf2254571aeca5fb215fc0eb9e7f87bed932f315b13787860eecc` |
| ntw16s64.dll | `25f9b7459124b9a698198004f53defd6080f7fdeed2adc5d66285359bf4e3c83` |
| run16s.exe | `77eb6a8a0326b80e26581c39843769aa71a54aa783e1ff9c1d68c4928f996d1d` |

A subsequent T18 S should evaluate the narrow real TToolBar text-only
contract. The ControlBar container itself remains in TODO. M0/T18 stay open.
