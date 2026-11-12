# Shared UWP test map

| Source | Native test |
| --- | --- |
| `repaint` | `repaint.c` (owned hidden window, timer and cancellation) |
| `dispatch` | `dispatch.c` (owned synthetic target and real fixture hook) |
| `hook` | `hook_fixture.c` and `dispatch.c` (identity, message, nested reentry and cleanup) |
| `global` | `global.c` (test-owned process, discovery, dedup, apply and remove) |
| `menu` | `menu.c` (system command state, collision, ownership and exhaustion) |
