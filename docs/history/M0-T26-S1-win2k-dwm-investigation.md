# M0 T26 S1: Win2000 DWM restore investigation

The existing `core-win2k-frame-lifecycle` test was rerun from the current
incremental build on the ordinary desktop, without switching desktops. Both
32-bit and 64-bit runs returned 3: the observed DWM non-client rendering state
was enabled before attach and disabled after removal, although attach, remove,
client geometry, reattach, thread ownership and pending-window checks passed.
The test's independent native and custom-frame baselines restored DWM.

With `--direct`, both widths failed in the same way, so the common global-window
manager is not necessary to reproduce this failure. With `--no-pump`, both
widths returned 0 and the original effective DWM state was observed after
removal. The difference includes processing the deferred frame recalculation
and other queued messages; it does not identify a specific message or prove
that deferred recalculation alone causes the failure.

A temporary x64 diagnostic called `DwmSetWindowAttribute` with
`DWMNCRP_ENABLED` again after the failed removal. It returned `S_OK`, but
`DWMWA_NCRENDERING_ENABLED` still returned false immediately, after
`SWP_FRAMECHANGED`, and after a non-client redraw. The temporary diagnostic
source was removed after the run. This observation does not establish the
underlying Windows behavior or a safe fix. Do not mark DWM restoration or
T26's cross-style lifecycle migration complete on this evidence.

Additional x64 instrumented comparisons independently bypassed the Win2000
caption painter, the custom `WM_NCCALCSIZE` adjustment, the `WM_NCPAINT`
branch and native-theme message suppression. Each individual bypass still
returned 3 with DWM disabled after removal. A combined painter and layout
bypass returned 0, but changes two behaviors at once and does not establish a
cause or a viable fix. These comparisons are not pixel acceptance. All four
temporary diagnostic gates were removed and the incremental x64 target was
rebuilt from the unchanged source.

A further x64 comparison let `WM_NCACTIVATE` reach the native window procedure
instead of taking the compact-frame early return. The direct, message-pumped
lifecycle test still returned 3: DWM was enabled before attach and disabled
after removal, while the test's native baselines restored it. This excludes
that early return alone as a sufficient explanation; it does not establish the
root cause. The experimental condition was removed and the x64 target rebuilt.

A bounded ordinary-desktop diagnostic pumped messages and flushed DWM for up to 200 ms after removal. Both 32-bit and 64-bit runs still observed disabled non-client rendering, while their native baselines reported restored state. The temporary diagnostic was removed; this rules out only a brief delayed completion in these runs and does not identify a cause.

A same-machine baseline comparison staged the original `ntw2ks32/64.dll` and `ntwshk32/64.dll` directly from Git HEAD `7c84e91d` and ran the same current self-owned lifecycle test. Both returned 3 with before=1, after=0 and native baseline restored, matching the released candidate results. The temporary baseline directory was removed. This establishes that this observed failure predates the uncommitted changes; it does not make restoration pass or rule out other regressions.

The native custom-frame comparison now also moves and resizes its window after
restoring DWM. On both x86 and x64 the native comparison remained enabled
(`after_move=1`), while the product path still returned 3 with DWM disabled.
This rules out that move/resize alone reproduces the failure; it does not
establish the cause or prove a product fix.
