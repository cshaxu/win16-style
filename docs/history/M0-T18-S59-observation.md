# M0 T18 S59 — remaining family observation and interim count

In self-owned DelphiVCL4Python 1.0.7 x86/x64 instances at 144 DPI, Splitter,
Bevel and PaintBox objects were created but their wrapper exposes no Handle;
child-window enumeration showed no corresponding HWND. This does not prove
all versions or all of their drawing semantics. A real TTabControl created two
tabs, forwarded WM_PAINT to the registered system SysTabControl32 procedure,
and returned valid count, selection and adjustment geometry in both widths.
The TTabControl itself was unattached, while its native up/down child was
attached. Thus the existing main-exe/native protocol does not cover this
runtime-owned case; S60 will examine a witnessed limited adaptation.

The [interim 74-family count](../etc/evidence/delphi-s59-count.json) records
24 families with at least one bounded implemented path, 2 explicitly preserved
native and 48 without an evidenced implementation under the stated accounting
rule. These are not numbers of full-family completion or of remaining code
changes: some bounded paths have known gaps, and pending includes nonvisual
and unclassified families. TTabControl is counted as bounded because an older
native path exists, even though the real runtime sample remains unattached.
This accounting is not the final T18 audit.

The independent clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256
was `09c781eb80155bd1bbf6d1b5f4ed0b225eb72ef3c53a2e2cff3e0706569e9eaa`.
PE/ABI gates passed and 14 focused actual-package cases passed in 7.70s at
four jobs, no skips. Full integration and physical input were not run. Four
rebuilt files were published to assets root and O:/winnt/system32; installed
x86/x64 toolbar models passed and all eight hashes matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `4e996290d55fe10d856f597b17dd78b34adfc1896a9d3f79bd450f5e7114f532` |
| ntw16s32.dll | `6bba336d074097232473f59830fdf6090c221f418750a63633b00a300cf8e17c` |
| ntw16s64.dll | `17f62c17af9c26dba8d4c136cbd705fdc6ec26a2e2051398d4a5aec6ff99f5d7` |
| run16s.exe | `f15de5495723e41c0fc4c9cde2856b1c0b2055d7c15a9702d07ae3a4632f937f` |

No production source or original graphics changed. The ledger was created
after the build snapshot; it is audit documentation, not a product input.
T18/M0 remain open.
