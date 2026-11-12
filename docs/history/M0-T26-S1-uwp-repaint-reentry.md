# M0 T26 S1: UWP repaint callback reentrancy

The shared repaint scheduler test now cancels from inside the native repaint
callback and from inside activation, matching the remove-while-callback-active
boundary. In both cases the test observes the caller retain count as one during
cancellation, then zero after dispatch, with timer/pending/waiting state cleared.
It also retains prior coalescing, timer retry, deadline and detach checks.

The current eight-file published package was verified identical between assets
and `O:\winnt` before testing. Both incremental MinGW GCC x86/x64 test binaries
passed on a self-created hidden window on the current desktop; no desktop was
created or switched. The state in this fixture is static, so this proves callback
ordering and reference accounting, not actual heap deallocation under reentry.
No product source or binary changed in this P. Real UWP/XAML frame drawing,
Win16 adapter removal under real application callbacks and the prior UWP access
denied remain unverified. T26 remains open.
