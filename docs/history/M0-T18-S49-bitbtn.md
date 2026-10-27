# M0 T18 S49 — scoped VCL bit button face

The owned DelphiVCL4Python 1.0.7 TBitBtn on both architectures used
BS_OWNERDRAW. The framework called DrawThemeBackground on the exact DC during
its parent WM_DRAWITEM, then painted its glyph and text. A selected item had
ODS_SELECTED; theme state alone did not reliably distinguish disabled from
normal. The new bounded path intercepts only a same-thread exact TBitBtn,
matching parent, runtime module, DC and client rectangle during
ODA_DRAWENTIRE. Native theme drawing runs first; the existing classic face is
then drawn before the application continues with its own glyph/text. Original
return/last-error and owner-draw callback stay owned by the application.
No resource or public ABI changed.

The real x86/x64 Delphi witnesses at 144 DPI with a self-owned bitmap showed
6,068 changed background pixels and 128 unchanged red glyph pixels per width;
BM_CLICK generated one notification and focus succeeded. This is synthetic
input and one runtime, not proof for all glyph/state variants. The clean
candidate's real witnesses also reported attached and closed normally.

The self-owned dual-width model checks an inactive impostor, target attachment,
full and clipped printing, 100 unchanged glyph pixels, one click, retirement
inside the callback and cleanup; the previous deployed package fails this
new test at target attachment on both widths. Exceptional callback unwinding,
foreign-caller and nested-draw negatives, additional icon and glyph layouts,
physical input and full T18 application integration remain to be verified;
S50 will cover the first three before this path is considered fully audited.

Independent clean snapshot SHA-256 of snapshot.json:
`53a4ee10244337ef93b3b5922bac26d7c53d690fdcc7d39c531cb7cd4e745592`.
Build-Package used GCC 16.1.0, Ninja and ccache 4.13.6, compiled x86 and x64,
passed PE and 31-export ABI checks. The actual same-batch four-file package
passed 20 focused cases at four jobs in 27.99 seconds, no skips; all other
cases were omitted. Installed system32 entry passed the model on both widths.
Both destinations were checked against the package:

| File | SHA-256 |
| --- | --- |
| demo.exe | `052a047ba4d2e4f0c9c11e5fb8961644eec677d180f6a2d7fbee87118e988b4f` |
| ntw16s32.dll | `54d5865b99381b090f98b697ee820481cb03cc5fade20cfe8eb5480e3402efce` |
| ntw16s64.dll | `1ef731fccdc14a898e8feee143c70701fdb37ee2e0cc5984292c5c0d4cfa32ae` |
| run16s.exe | `7ec941452f153c56eadf8a13816d9331f824aab7b37648020656fd3e89ee1a6a` |

Search `rg -n 'ntw16s_bit_theme_frame|win16_style_bitbtn_face|bit_draw_dispatch' src`
locates the sole hook entry, scope/guard and renderer; unrelated theme drawing
uses its original path. The stat before adding history and the two new fixture
files was 134 insertions and 16 deletions across text files; final Git numstat
is authoritative. No existing user changes were removed. T18 and M0 remain
open; publication is not whole-task acceptance.
