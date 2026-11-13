# Shared UWP test map

| Source | Native test |
| --- | --- |
| `repaint` | `repaint.c` (owned hidden window, timer and cancellation) |
| `surface` | `surface.c` (bounds and refresh failure, reentry, detach); `surface_present.c` (mocked layered presentation, success and failure) |
| `host` | `host.c` (native subclass removal, failure, reentry and deferred disposal); `host_start.c` (installation failures and reentrant creation) |
| `input` | `input.c` (button tracking, hover, cancellation, double/right-click routing, detach during refresh) |
| `dispatch` | `dispatch.c` (owned synthetic target and real fixture hook) |
| `hook` | `hook.c` (direct identity and reentry); `hook_fixture.c` and `dispatch.c` (real thread hook and cleanup) |
| `global` | `global.c` (test-owned process, discovery, dedup, apply and remove) |
| `menu` | `menu.c` (system command state, collision, ownership and exhaustion) |
