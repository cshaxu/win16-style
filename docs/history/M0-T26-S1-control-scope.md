# M0 T26 S1: shared control creation scope

`lib-style/common/control_scope` owns the existing thread-local caller's list
mechanism, one hook while a list is nonempty, nested root selection, root
invalidation, and delayed allocation release while an observer callback is
active. The list, hook variable, observer procedure and per-scope data are
caller-owned; the scope allocates its zero-initialized caller data and retains
it until the last callback releases the scope.
The Win16 adapter now uses that mechanism directly while keeping its palette,
control selection, combo exclusion, installation callbacks and counters.
The public component contains no Win16, product or rendering dependency.

The independent native test covers arbitrary caller data, nested selection,
invalid scope, root invalidation and release after an active callback. The
Win16 test exercises actual control creation, nested scopes, a callback that
ends its own scope, and invalid scope handling. The existing Win2000 dialog
observer has different lifetime and event semantics and has not been migrated;
no Win2000 control styling is claimed by this extraction. Actual applications,
physical input, alternate styles and the pre-existing Win2000 DWM restoration
failure remain outside these tests.

The first clean build stopped before compilation because its snapshot captured
CURRENT after a document edit but not the new linked document. No files from
that snapshot were published. After its copy was cleaned, a stable snapshot
passed the x86/x64 MinGW GCC, Ninja and ccache build. The eight candidate files
were published to assets and `O:\winnt` before the focused tests; all three
sets had matching hashes. Thirty-six focused executions passed, including both
scope tests, theme and dialog checks, Win16 DWM restore, and published launcher
menu and Win2000 frame geometry probes. The shared control archive had no
undefined product or Win16/Win2000 symbols on either architecture.

One mistakenly selected `core-global-window` entry attempted to create a
private desktop and failed with error 5. No private desktop was created and
the entry was not retried. It is not counted as passed; it was the wrong test
for the user's explicit prohibition. No computer-use or desktop switching was
used. This does not establish full global-window or actual-application coverage.
