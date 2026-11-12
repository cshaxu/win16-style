# M0 T26 S1: shared theme-handle tracking

The existing theme open/close ticket tracker moved from the Win16 DLL's
injection directory to `lib-style/common/theme_track` and is linked as
`style-theme-track`. The DLL now calls that shared API. Callers supply opaque
integer kinds; Win16's theme-kind classification and drawing remain in the
Win16 adapter. The tracker remains one process-wide instance per linked module,
with the original fixed capacity, synchronization and fail-closed behavior.
The old source path and duplicate implementation were removed.

The original native test moved with the shared component and covers handle
aliasing, nested and overlapping calls, failed close, capacity and cancellation.
It uses caller-defined kind values without linking Win16. Another style has not
yet been wired to this tracker; this change does not claim that its hooks,
theme rendering or visual behavior are implemented.

Clean MinGW GCC / Ninja / ccache builds passed for both architectures. The
eight files were published to assets and `O:\winnt` before the focused tests;
all three sets matched. Forty executions passed: 36 component and cancellation
checks and four published-launcher system-menu probes for Win16 and Win2000.
The shared archive had no undefined product or specific-style symbols. These
checks do not prove real application theme drawing, global session behavior,
or that an alternate style consumes this tracker.
