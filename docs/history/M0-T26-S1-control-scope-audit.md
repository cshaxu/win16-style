# M0 T26 S1: control creation scope boundary

`win16-control/control_scope.c` owns a same-thread nested scope list, the
`WH_CALLWNDPROCRET` subscription while any scope exists, root invalidation,
and delayed free when an installation callback ends its own scope. Those
lifetime mechanisms are not tied to Win16 rendering. They currently share a
structure with the Win16 palette and installation counters.

The observer also selects supported control classes, excludes descendants of
combo boxes, chooses a separate edit installer, and passes the Win16 palette.
Those are current style policies, not generic observer requirements. A shared
scope must accept a caller-supplied observation/install operation and retain
the existing nested ordering and safe callback-time end behavior; simply
moving or renaming this file would keep a Win16 dependency in the public API.

The production caller is `ntw16s-dll/control_scope.c`; the existing native
test is `tests/lib-style/win16-control/control_scope.c`. It covers nested
scopes, creation and ending a scope from an installer callback. The Win2000
`ntw2ks-dll/controls.c` currently has a separate dialog-creation observer,
but its function, scope and callback contract differ. The audit does not
claim these are interchangeable, that Win2000 uses the Win16 observer, or
that controls recover their prior theme. Implementation, failure-path tests,
and a real cross-style caller remain outstanding.
