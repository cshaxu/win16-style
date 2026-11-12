# M0 T26 S1: shared UWP repaint scheduler

`lib-style/uwp/repaint` owns the existing message-only timer, 30 ms repaint
coalescing, visible XAML bridge wait/retry, deadline and cancellation. Win16
frame dispatch supplies style-specific activation plus retain/release callbacks
and owns scheduler storage; Win16 frame removal cancels it directly. The helper
requires the owner UI thread and stable owner, context and services until cancel.
Its callback retention protects the caller's state during synchronous reentry.
The 2-second wait policy and visual frame processing remain with Win16.

The standalone native probe creates its own hidden window and exercises timer
coalescing, cancellation, an expired visible-bridge wait after retries, callback
retention and rejection after detach. It is not an actual UWP or WinUI app;
real XAML lifecycle, visual flicker and previous access-denied behavior remain
unverified. No Win2000 UWP implementation or consumer is claimed. T26's wider
window, control and dialog migration remains open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`dd14d7f8e4549c95de66395c30d56760529876b8dd0bf6791ed8217b860735ab`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed 16 component tests, two published-launcher
probes, two protocol workers and a published-launcher list-report worker:
42 focused checks. The repaint test exercises an actual message-only timer and
hidden native owner, but not a real XAML application or removal during a
reentrant callback. Clean scheduler archives have no unresolved Win16, Win2000
or product-private symbols. Real UWP visual and lifecycle coverage remains open.
