# M0 T26 S1: shared UWP menu transaction

`lib-style/uwp/menu.h` now owns the existing system-command restriction and
restoration and temporary action insertion/removal. The caller retains the
menu, decides whether it still belongs to the window, and owns the action
objects. IDs remain collision-checked, removal checks the item data owner,
and the Win16 UWP frame calls the shared transaction. Its frame geometry,
rendering, action dispatch and tracking state remain style-specific.

The independent native test covers command state restoration, an occupied
action ID, wrong-owner removal and exhaustion. The Win16 UWP frame test is a
separate direct-worker test of menu actions and frame lifetime. Its entry with
`--private-worker` returns before the private-desktop branch; it runs on the
ordinary desktop. A sandboxed x64 attempt returned error 5 at non-client frame
attachment; the same direct worker passed outside the sandbox. Its printed
`private-desktop` label is historical and does not describe this invocation.
This does not establish real UWP application or alternate-style coverage.

The clean x86/x64 MinGW GCC, Ninja and ccache builds passed. The eight files
were published to both destinations before testing, with matching hashes in
the candidate, assets and `O:\winnt`. Thirty-two component executions, both
ordinary-desktop UWP frame direct workers and four published-launcher menu
and Win2000 frame-geometry checks passed. The frame fixture statically links
the same snapshot's style library; it is not a real UWP application test.
