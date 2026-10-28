# M0 T18 S64 P109: modern Delphi SpinEdit edit surface

A runtime-owned modern `TSpinEdit` in the real DelphiVCL4Python 1.0.7 sample was not attached by P108, unlike the previously supported legacy DoubleKiller EDIT superclass. The new candidate accepts an exact newly created TSpinEdit only after its own WM_PAINT is forwarded by its class owner to the registered system EDIT procedure and the existing initialized EDIT protocol succeeds. It applies the existing edit font and border; VCL retains value, data and input. Its observed ES_MULTILINE style does not cause the ordinary TMemo scrollbar path to be installed.

A first candidate made the real x64 text format rectangle extend to x=290 while its button began at x=280; the old format rectangle ended at x=278. The final code checks the exact same-owner, same-thread TSpinButton child and, when a font change would newly extend a previously safe format rectangle into that button, restores the prior right limit. In the actual final package, both x86 and x64 samples at 144 DPI had format right 277 and button left 280, and their editor font was Win16 System height 25. Synthetic Up changed 42 to 43 and produced one OnChange value 43 in both the old and new real samples. The final target and launcher exit codes were zero and owned jobs emptied. This does not prove all resize, reparent, unusual fonts, DPI or physical input variants. The newer runtime TSpinButton itself is not styled by this change and remains open.

The project-authored dual-width EDIT witness model rejects absent or foreign paint, preserves clipped text, selection and readonly behavior, and exercises reentrant removal. A derived model adds a same-owner button and deliberately simulates the observed font-induced format expansion; it verifies the reservation both at attachment and another WM_SETFONT. With the new reservation guard deliberately disabled in an isolated diagnostic DLL, both widths failed the new layout assertion; the old published package failed the new attachment assertion. The synthetic model does not itself implement Delphi numeric semantics. No third-party source or original graphics were changed.

A clean C17 MinGW GCC/Ninja/ccache snapshot passed source, original-resource/provenance, governance and x86/x64 ABI gates. Snapshot JSON SHA-256 `8093da9b9b936c2c1448dff50241da6db1ef72422a3d19919113cc90357bdaca`; x86/x64 builds took 25.90/31.50 seconds. The exact four-file package passed 16 focused cases in 6.59 seconds without skips, including both new models, existing spin, NumberBox and link controls, plus mandatory startup/propagation cases. Full integration and physical input were not run. Both specified destinations matched the tested hashes, and each installed package passed both-width SpinEdit models.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `c82d9a583da5f249115bad6b19b958be51512e8f4e707aa7367e7842e48a86bb` |
| ntw16s32.dll | `fed3b433367c5ea0ddb4da99aaa15f5bd0dff0df7fa0f9fe6e4bdf5f3ca55ba8` |
| ntw16s64.dll | `a9cb856dc948eb22f11c4e35a9d8f8db199fbfdd7043faf050683608d0b5895c` |
| demo.exe | `2c5ab3675c3d5462b85fd2a4877992171f8dfb6b578dfd5ba5d1c993ccb91223` |

M0/T18/S64 remain open.
