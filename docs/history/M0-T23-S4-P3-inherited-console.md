# M0 T23 S4 P3: inherited classic console

The previously released P1 did not attach a classic console already hosting
`run16s cmd`. This change identifies the current console's server process,
verifies the system conhost image, and uses the existing thread-targeted window
attachment. It does not enumerate unrelated processes. If the Windows-internal
`NtQueryInformationProcess` information class is unavailable, the launcher
leaves the console unchanged. Its compatibility across Windows versions is not
established; the loaded DLL is not physically unloaded on removal.

The same-snapshot x86/x64 GCC/Ninja/ccache build passed. The actual package passed
29 focused cases with no skips, and the self-owned direct and inherited conhost
fixtures passed attachment, echo input/output and removal. The earlier P1 first
cmd-nested test failure remains unexplained; this run passed that matrix. The
Terminal XAML caption buttons remain duplicated. S4 and T23 remain open; this is
not a claim that all WinUI apps are covered.

Four-file SHA-256, verified against the tested package at assets and O:/winnt:

| File | SHA-256 |
| --- | --- |
| run16s.exe | 755d0a594c10e427f6ab805628a18462aa72fae657696dd69607888c8417799e |
| ntw16s32.dll | a19026a3e21be1690a708c153a009ade9dde020d91da107c146f1172e52cdaf4 |
| ntw16s64.dll | b9e86f3130793e764fff23ed8ec9d4f049711c45b8f2a438519503a48c3f09c2 |
| demo.exe | c8a6fa6995a1ffc70309b731238e3a7640adfd461fb976729ad6dd0541ce71af |

Evidence summaries remain in this document and build/t23-package/s4-inherited-package1.
After delivery and remote verification, the no-longer-needed clean build tree is
removed in accordance with Td S19; the reusable incremental cache is retained.
