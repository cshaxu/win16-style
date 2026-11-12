# M0 T26 S1: shared window-thread dispatch

`lib-win32/style/window_dispatch` owns the existing target-thread hook module
load, message dispatch, acknowledgement property cleanup, timeout and hook
release. Callers supply the window, thread, module, export, message, property
and action. `ntw16s-dll/uwp/target.c` retains target association, host bitness,
module validation, title capability and Win16 protocol selection. Both the
launcher and DLL now use the shared transport through that adapter. This does
not add Win2000 UWP styling or relax target policy. The transport contract uses
a caller-selected property containing error + 1; callers must validate the
window, target thread, architecture and module before calling it.

An initial clean x86 build exposed two manager fixtures missing the new
transport link. Full incremental builds then exposed another tray fixture;
the common UWP-consumer build list was corrected and both full incremental
builds passed before the replacement clean build. The failed temporary copy
was removed. Clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed with
snapshot SHA-256 `20be79ec96ba637f4dd20578a22a82e6148ce30457b4ead719b8fa425059d4bc`.
The eight files were published to assets and O:\winnt before 12 checks.
Both architectures passed a real, test-owned separate process and window with
success, returned error, missing acknowledgement and missing hook export;
fixtures shut down their child process. Both also passed published-DLL style
switching, shared module loading, UWP target discovery, published Win16 menu
and Win2000 scrollbar geometry. The shared archive has no unresolved Win16,
Win2000 or product-private symbols. Real UWP applications, cross-bitness
hooking, global administration, physical input and full visual acceptance
were not covered. Both published destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `6d0cf275f4cb0ec9723208a533dfa2005597ed9055be4536c266f7e73754eceb` |
| ntw16s32.dll | `5568e3724f4a00cb32411920a22675dcf6d04978149167ae52a8aeb226726f39` |
| ntw16s64.dll | `7365409a81af1fe5a19a30c4a711df4e92c585662200524181530ae6a4bd43bb` |
| ntw2ks32.dll | `c88783408f0818ef20378ee29731c06ba1d5b9d405acae8c95499f80e4c3bddc` |
| ntw2ks64.dll | `d236832d76694881a6fc5a15b245608bd333209e23f8ac8196898a8ab8ea4a3a` |
| ntwshk32.dll | `15bb1b52b2a03053580ea3517b281c4fe4c7fa3c73b4e97f7c7eedadca062637` |
| ntwshk64.dll | `451dbc99b6f28541d7f81f54363da74b91f11fc9720589fa60cb51ebe226d9e3` |
| run16s.exe | `ad67cac7dc2eeb8575dc6dc76dc6cab292052356e13152d00e8ee58ef3220df6` |

T26 S1 remains open, including comprehensive audit and remaining cross-style
adapters. The current static-library target is still named `style-companion`
despite now containing two mechanisms; target naming should be consolidated
as part of the ongoing library layout audit.
