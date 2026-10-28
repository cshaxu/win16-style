# M0 T18 S64 P72: ordinary Delphi radio-group frame

The ordinary `TRadioGroup` container used the same observed native theme background and title route as the previously adapted `TGroupBox`, but the P71 code did not recognize the radio-group container. The existing scoped group-frame implementation now accepts the exact `TRadioGroup` class and real class as well. The same same-thread, owner, parent, paint-scope, theme-background witness, caption, DC and frame checks apply. The renderer preserves the runtime's option controls, text, font, selection and events; a group that changes parent retires. Disabled and other unsupported variants remain native. This adds neither a product file nor a public export.

A project-authored x86/x64 fixture using a `TRadioGroup` name exercises the same self-contained P71 checks: background witness, outside-paint rejection, clip preservation, reparent retirement, reentrant reparent, empty caption, disabled fallback and wrong parent owner. Both candidate widths passed; the P71 published package failed the positive attachment assertion in both widths. A model class is not a Delphi runtime.

The actual separately built DelphiVCL4Python 1.0.7 x86/x64 owned samples at 144 DPI were run with the candidate package. Each group attached, the observer restored its imports, and the owned instance closed. Two actual runtime `TGroupButton` children preserved their selection across synthetic clicks: `(1,0) -> (0,1) -> (1,0)`. Each 250x120 captured group differed from the prior published package by 616 RGB pixels; the caption rectangle and each of the two child rectangles had zero pixel differences. These message tests do not establish physical keyboard/mouse behavior; screenshots do not prove every interaction or version.

A clean same-snapshot x86/x64 GCC 16.1.0, Ninja and ccache 4.13.6 build passed the ABI and package checks. Snapshot manifest SHA-256: `bed5fc5d334d165662bf5f2e8325291a432752b86421780eee6fb98e5a2aaf69`. The actual package's 22-case focused regression passed without skips in 23.07 seconds; it included both frame variants, radio controls, VB controls, menu cancellation, startup and mandatory injection/ABI baselines. Clean builds took 17.96 seconds x86 and 21.48 seconds x64. The five original resource headers and original provenance remained byte-identical, and the source manifest passed.

All four tested files were backed up and published to `assets/` and `O:/winnt/system32`; both directories matched the package hashes. Installed x86/x64 model checks passed from both destinations:

| File | SHA-256 |
| --- | --- |
| run16s.exe | `af58dcf8448920b06723052e4cee5c3bb2d5dce93a48b4835100db3059825153` |
| ntw16s32.dll | `f85f86672362d13ab9b5d15b48f6eb053ec36ac5509f247d9e26d08409598dfa` |
| ntw16s64.dll | `342ed021f7873f16a69019f91594ab10bee218aaeeef886cbb45c87006e19563` |
| demo.exe | `262debb6376192886570fb4bfc4dcb3adb2db286effcec6d7a9bafe2e71c6849` |

Production code `attachment.c` changed +2/-2 lines; existing scoped drawing and hook paths are reused, with application-owned state and old native fallback preserved. Additional DPI, runtime versions, disabled appearance, other orientations, actual empty-caption images, physical input, full GDI lifecycle and the complete task matrix remain unverified. The group caption retains the framework's font. M0/T18/S64 remain open.
