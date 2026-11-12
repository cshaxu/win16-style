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
