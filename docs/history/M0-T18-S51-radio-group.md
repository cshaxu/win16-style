# M0 T18 S51 — VCL radio group child buttons

The self-owned DelphiVCL4Python 1.0.7 radio-group sample has one TRadioGroup
and two separately created TGroupButton children. Neither group nor children
were attached in the previous release. In both widths, the child runtime
module forwarded WM_PAINT to the actual system BUTTON procedure; the button
reported a valid ideal size and BM_GETCHECK. A synthetic click selected the
child and cleared its sibling. This establishes a narrow native-button
protocol, not a contract for the group container or arbitrary VCL classes.

The candidate only considers same-thread, same-runtime TRadioGroup children
named TGroupButton with BS_RADIOBUTTON and without bitmap, icon, pushlike or
RTL modes. It waits for its own runtime's actual native BUTTON WM_PAINT
forward before binding the existing BUTTON protocol and painter. A native
protocol check is still required; a same-name fake paint stays native. Existing
application input and grouping remain responsible for selection. Renderer font
substitution follows the existing button policy; the application-owned source
font is restored on removal.

Real x86/x64 samples at 144 DPI using the clean candidate showed both children
attached; for each child BM_CLICK changed it 0 to 1 and its sibling 1 to 0.
The whole-window screenshots differed by 21,568 pixels, in the two button
rectangles. Those are synthetic inputs, not physical input. The self-owned C
model passed fake/native classification, selection, space key, removal, font
restoration and cleanup in both widths; the old published package failed the
new positive attachment check on both widths. A model x64 crash was traced to
an extra zero byte in the test fixture's locally generated dispatch thunk and
was fixed there; it was not a product fix. Group container chrome, changes of
parent or control role after installation, other versions, variants, and full
T18 integration remain unverified.

The independent clean dual-width GCC 16.1.0/Ninja/ccache 4.13.6 snapshot
manifest SHA-256 was `257e29ba8a57310d7e4b98be60cff113d06f8c47b20dbbdf72ac27c5fbd10158`.
It passed PE/ABI checks and 18 same-package focused cases in 12.94 seconds at
four jobs, no skips. Other tests and physical input were not run. The four
files were published to assets root and O:/winnt/system32, with matching
hashes and passing installed x86/x64 model tests:

| File | SHA-256 |
| --- | --- |
| demo.exe | `14c65732ae2b83f5bc87019ceb91d46bd44e8e34336ed0e209685e139f90a05b` |
| ntw16s32.dll | `4bb5faa652a1a232a23a7f8f3640c8f43afe0211ee97223bcc6daab9e1237aed` |
| ntw16s64.dll | `c7c8754234b1194d88955d1b557d5b2068ce80a6acaab476191f6334dcca9a2f` |
| run16s.exe | `05b449d9ce9540e6e709431c3aaf65cf8603f0845bbb75fe83eb3b931572080d` |

Original graphics and public ABI were unchanged. M0/T18 remain open.
