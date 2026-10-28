# M0 T18 S64 P93: Delphi NumberBox edit surface

Real DelphiVCL4Python 1.0.7 TNumberBox windows remained native in both widths. The exact newly created single-line class is now a passive candidate for the existing edit renderer, admitted only after the class-owner module forwards the window's own WM_PAINT to the registered system EDIT procedure and the existing initialized native edit protocol accepts it. No new product, public export, original graphic or provenance change was made. The optional compact/inline spin glyphs remain runtime-drawn; their full classic styling is still outstanding.

Real x86/x64 samples at actual 144 DPI were tested against the final package. Default and MinValue=0/MaxValue=50 samples attached; direct synthetic character input from 42 produced text 72 and OnChange value 72 in old and new. The stated limits did not immediately clamp that input in either version; actual commit/focus-loss validation was not exercised. Final-package default captures changed 940 RGB pixels in the edit rectangle against the earlier same-width old run. With compact and inline spin options, synthetic VK_UP changed 72 to 73, with the same two OnChange text/value observations in old and new, both widths. All final target and launcher exit codes were zero with owned job process count zero. Direct synthetic mouse messages in two attempted coordinate paths did not change the old value, so no mouse spin-click success or failure is inferred. Physical input, spin-click/hover/pressed appearance, all numeric modes, expressions, range commit, other Delphi versions and DPI remain untested. [Embarcadero's control reference](https://docwiki.embarcadero.com/RADStudio/Athens/en/Using_VCL_TNumberBox_Control) documents the separate numeric modes and spin choices; the present test does not claim them all.

The self-contained dual-width native fixture reuses the existing edit protocol test, covering no-native-paint, wrong caller, bounded clip, text selection, readonly and reentrant removal; the old assets fail the positive attachment check in both widths. The clean same-snapshot C17 MinGW GCC/Ninja/ccache build passed source, original-resource, governance and x86/x64 ABI gates; build times 26.65/32.40 seconds. Snapshot manifest SHA256 `4c7c39fbbeec2b0d05eb3ba8ae5cda7ec81d26d18bf487cf90cec494967d47da`. The exact four-file actual package passed 14 focused cases in 6.53 seconds without skips, including both NumberBox fixtures, adjacent labeled/edit models and mandatory launcher/propagation checks. Both deployment directories contain the exact tested four-file hashes and the installed native model passed x86/x64 in both.

| File | SHA256 |
| --- | --- |
| run16s.exe | `ef479aa0d101c8dfc948ab64c71c7a1e92ecd159b3e5c4da055a3225bcd15b9d` |
| ntw16s32.dll | `b62038a24bc923bbed35e368e7bfbf967ce0e8e352b75e87a691709de3ff76c8` |
| ntw16s64.dll | `68ab9d045eb8da2da985c60efda6731900953b302d22e4fb2acd472b6d307cc8` |
| demo.exe | `36a0b6fb5f5c8ae50bce3850c685a834cd1f24f77d2b6c89e0efccb152f53b9e` |

See [investigation](M0-T18-S64-delphi-numberbox-investigation.md). T18 and S64 remain open.
