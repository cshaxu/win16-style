# M0 T26 S1: ordinary-desktop global window retest

The two architectures' `core-style-global-window` and
`core-global-window --private-worker` were run using self-created windows on
the ordinary desktop. The latter flag enters the worker directly; inspection
of `tests/ntw16s-dll/isolated_process.h` confirms that this branch returns
before `CreateDesktopW`. It does not install a global hook or style unrelated
applications. The test fixture and the three matching-bitness DLLs were copied
to task-owned scratch; the DLL hashes matched the published `O:\winnt` files.

All four runs passed. The common manager test covered apply, remove, repeat,
failure cleanup and rollback. The product worker covered Win16 window, button
and edit attachment and removal; its own button rendering changed and restored,
and it checked wrong-thread, destruction, reentrancy and preattached windows.
This corrects the earlier missing direct-worker check; the mistakenly invoked
private-desktop entry remains a recorded error and was not retried.

`core-global-style-bridge` was also run on the ordinary desktop using the same
published DLLs. Its x86 run passed. Its first x64 run returned 3: all reported
transitions and client-paint checks passed, but the status child bottom was
263 while the client bottom was 277. Three immediate x64 reruns passed with
both bottoms at 277. The x64 result is therefore intermittent, not an
unqualified pass. The test uses a self-created window and synthetic messages;
it does not cover physical input, full global session management, or the
separate Win2000 DWM restoration failure.
