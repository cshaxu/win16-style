# M0 T18 S64 P2 — VB same-thread queue test

The self-owned VB queue test installs hooks on its own thread while loading
callback functions from a DLL already in that process. On this host the old
non-NULL hMod call failed with ERROR_MOD_NOT_FOUND (126) for both x86 and x64.
For this same-process thread, the test now passes NULL hMod, keeps the callback
DLL loaded until after both hooks are removed, and releases all resources.
Microsoft documents the distinction in [SetWindowsHookExW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw).
The old P1 test failed in both widths (exit 1); the new test passed both,
including wrong-tag rejection, a notification, the post-unhook negative and
cleanup. The test itself is now part of focused package selection.

An independent self-owned cross-process EDIT control was also tested on both
architectures: target and controller had the same private desktop and the
hook function address belonged to the supplied module, but cross-process
SetWindowsHookExW with non-NULL hMod still returned 126. Its cause remains
unproven. The same-thread correction must not be represented as fixing real
cross-process observation, or as proof of full VB5/VB6 compatibility.

Clean GCC 16.1.0/Ninja/ccache 4.13.6 x86/x64 snapshot manifest SHA-256:
`b16c49a3cc2ba0c08691b4d86d5608f862cc2b417baedc9e62933235e5323d1f`.
PE/ABI, source, original-resource and governance gates passed. Fourteen
focused actual-package tests passed in 11.37 seconds at four jobs, no skips;
other tests were not run. Both installed launchers passed the x86 and x64
queue tests. The rebuild changed hashes, so the four tested files were
published to assets root and O:/winnt/system32 with matching hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `921fee9bef79d531c79478e2677010e9fba9a398658d3b30653b441a29500733` |
| ntw16s32.dll | `34df3026e281221d59ea8c6c6c91d7781f42d6fd7fcb112e813e9b465f055a21` |
| ntw16s64.dll | `5e6c8f36efbd7727d249b3fcf42102556c492aa274164adeae3468504a363b0f` |
| run16s.exe | `a6e64eb3b1eb927fc34c45df8afdff368f23eac762050b4373bc4a3c3178cada` |

Search `rg -n -F 'SetWindowsHookExW(WH_GETMESSAGE' tests/integration` identifies
this same-thread unit and the separate cross-process observer. The latter is
unchanged because it requires a module handle and its failure is not yet
understood. Test source/build wiring: 5 additions and 1 deletion, net +4;
production source change: zero. M0/T18/S64 remain open.
