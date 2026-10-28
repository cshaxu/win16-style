# M0 T18 S64 P108: delayed Delphi link text

The P107 adapter permanently recorded a failed protocol attempt when a real `TLinkLabel` first painted with empty text. In the self-owned DelphiVCL4Python 1.0.7 x86 and x64 samples at 144 DPI, `LM_GETIDEALHEIGHT` returned -1 and attachment was false; after setting a link and repainting, the height became 25 but P107 still did not attach. A first candidate that rearmed only on WM_SETTEXT passed the C model but still failed the real sample, so it was not shipped and its cause was not assumed. The final implementation clears the failed attempt and previous witness; a later paint must establish a fresh same-owner native SysLink forwarding witness before retrying the initialized protocol. Ordinary installed controls retain the existing path.

The final same-batch actual package changed real x86/x64 samples from `(attached=0, height=-1)` to `(attached=1, height=25)` after setting the link. Target and launcher exits were zero and owned jobs emptied. Self-contained x86/x64 tests cover the delayed empty-to-link transition and verify that a subsequent foreign-caller paint does not reuse the old witness; a later legitimate paint attaches. The P107 published package failed the delayed positive assertion on both widths. Existing tests also cover native link focus, mouse/Return notifications, disabled link item, visited state, original font restoration and reentrant removal. Repeated empty repaints now retry bounded protocol queries when a fresh witness is present; performance under pathological repaint rates, other VCL versions, physical input, languages and the full integration matrix are not established.

A clean C17 MinGW GCC/Ninja/ccache same-snapshot build passed source, original-resource/provenance, governance and both ABI gates; snapshot JSON SHA-256 `8c86a180e0c8d3640aad65ad1b84fbb3ecaf7afdac7f22b4ab6ca603aba19b88`; x86/x64 build times 23.95/29.04 seconds. The exact four-file package passed 16 focused cases in 6.79 seconds, zero skips; this is not a full integration run. Both specified destinations matched the tested hashes and the installed link model passed both widths in each destination.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `257fcfd19bf1183df61c307de1b36cbddabe00bcc5e95b0754e6ba5a15983f5a` |
| ntw16s32.dll | `45f1601b29c42cfb5d283d3e85c4e9a78a2233a6d58328891d742da73887dd03` |
| ntw16s64.dll | `0ebac1e3e54e2e904f163fa0040b1d30b5d3689ca5b43bf9d5f5486b344799eb` |
| demo.exe | `4419eeb8ed3c7388cf599987b8f60922e59058095c294ae083fb8c3dd2ed860a` |

M0/T18/S64 remain open; other ordinary control gaps remain.
