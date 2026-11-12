# M0 T26 S1: shared presentation and print traversal

`lib-style/common/present` now provides buffered client presentation with
caller-supplied background, grouping and paint callbacks, plus the existing
client, direct-child and owned-window print traversal. The Win16 control
adapter and frame message path call the shared functions; the old Win16 common
implementation was removed. `style-present` depends only on USER/GDI. Win2000
has not yet adopted this rendering strategy and no change to its output is
claimed. The shared print code preserves its attachment flag contract, print
ordering and same-thread owned-window traversal.

Clean x86/x64 MinGW GCC 16.1, Ninja and ccache builds passed. Source snapshot
SHA-256 `adee901bbbf5313f814c59dc3765e8c6f96f768b78ed5d27c7c285d62d1d95b0`.
The eight artifacts were published to assets and O:\winnt before 14 focused
post-publication checks. Both architectures passed standalone buffered and
detached drawing, failure fallback, group SaveDC failure, print order and
reentrant detach, Win16 control and dialog linking, the published Win16 menu,
and Win2000 scrollbar geometry. The shared archives have no unresolved Win16,
Win2000 or product-private names. The existing instrumented control-presentation
fixture compiled for both widths but was not run because it creates a private
desktop, which the user prohibited. Actual user visual acceptance, normal
runtime control-paint and the full matrix remain unverified. Both published
destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `78a4330c82397a12f3b19271207c3573ac8046443fb619bb302f2d4951513d3d` |
| ntw16s32.dll | `8a02f31f9bbb7d162fa1fac99f88ee1568390b4e3887ed3ca5aec753943c6848` |
| ntw16s64.dll | `6d95eb78e72426e3d31a36697d046154e3ef80e09db1e0eb80632912e88acc55` |
| ntw2ks32.dll | `473a1974b9e851e7e474efb945631a18d490df37559919a04ba9c9a6b612c408` |
| ntw2ks64.dll | `70506dd4c8f356bf5a70264ed57fe7461b6aa90f596265e696c7a1fa50469353` |
| ntwshk32.dll | `7f09c4d3809e063f5a3791a674b64de565ab0c5507ca977f3c59727ff664f5ac` |
| ntwshk64.dll | `b167e110562cf1c40177e10e95282bc8da4e317d16bc21135ec139607b2e49f5` |
| run16s.exe | `ec9016a5c00dc2b3e26dc171b0782ac261fd3eb31fbeeaf13a0d1be3a0224ea3` |

T26 S1 remains open; theme-state lifecycle, full cross-style callers and the
previously documented DWM recovery and visual issues still need work.
