# M0 T23 S2 Task Manager: limited delivery

The owner requested ending Task Manager work at its current scope and proceeding
to the next application, without further Task Manager UAC approval prompts.
This is not a claim that all requested visual defects were fixed.

The candidate adds a narrow same-image `runas` handoff for the existing startup
path, optional window-band interception, and deferred frame activation and
repaint handling for direct XAML composition bridges. It preserves the existing
four product files and does not add an exported API. The redirect changes the
image presented by UAC to the launcher, and successful Shell handoff does not
prove that the subsequent target was created; unsupported flags and modes keep
the existing route. Real x86 elevation and actual Task Manager are separate
observations, not a complete matrix.

For a self-created Task Manager, standard `run16s taskmgr` reached its elevated
window, loaded the candidate x64 DLL with a matching path and SHA-256, and
reported the window attached. A guarded actual-screen capture showed the Win16
caption. The owner confirmed the caption persisted through hamburger and maximize
on an earlier candidate. These were incremental candidates, not the final clean
package, and do not prove general WinUI compatibility.

Known failures: a clipped duplicate icon and right-edge white strip remain below
the caption. In one real short drag, the title's blue-pixel samples fell from
8693 to 95; neither synchronous repaint after erase nor after position change
fixed it, and both experiments were removed. Changing only the client top was
sufficient to reproduce the duplicate icon without painting the Win16 caption.
The DWM margin and notification experiments did not solve it. See TODO for the
remaining work. Cross-DPI interaction, sustained dragging, full real-target
restore/cleanup and the final clean package's Task Manager display are not
verified. The private-desktop Task Manager probe was not run because its UAC
request ended in cancellation; no conclusion is drawn from that attempt.

`s2-close-clean2` built the final LF snapshot with C17, MinGW GCC 16.1, Ninja
and ccache 4.13.6 for x86 and x64; ABI, original resource, source and dependency
gates passed. `s2-close-package2` passed 29 focused actual-package cases with no
skips. The clean build also passed 14 selected core and injection tests across
both widths. Those tests do not replace the real Task Manager visual checks.
An earlier clean1 package passed the same focused test and was published, but
was superseded after correcting test-file line endings and an EOF blank line.

The CMake, product source and test delta is 720 additions and 7 deletions
(net 713), including the new test files. The existing frame state owns the
deferred timer and cancels it on removal or destruction; no parallel renderer
or separate product component was introduced.

The tested four files were published to the assets root and `O:\winnt` and
hash-checked at both destinations by `s2-close-publish2`:

| File | SHA-256 |
| --- | --- |
| run16s.exe | `3e3a2114d42350a9e5f700fe12d862fb39a3ca4ce48a7737f7d5c30488fbf1db` |
| ntw16s32.dll | `8fc0ec28b050f93e83044fa047bb9fdef1dd06f18e4facdd1d924591e2716981` |
| ntw16s64.dll | `b74a2ee056836f4a8de74f6da19931bf082998701a72e45c1a14d7098cfff377` |
| demo.exe | `ec743ac5f3c045e2180e7d5599b0d5ccce73004158add941f928b34c70210786` |

The original user request, unchanged S2 evidence, and individual experiments
remain recorded in CURRENT and the registered scratch logs pending review.
Remote verification and S2 closure are recorded separately after the delivery
commit is pushed; the next work is Explorer folder windows under T23 S3.

P1 `6afe94200f9014e94db891c2082db4fd6d68ed23` was pushed to origin/main.
The four assets, the frame dispatch and elevation source files, and this record
were read from the actual remote commit and each SHA-256 matched local content.
The owner explicitly directed limited S2 closure with the outstanding defects
retained in TODO; that decision is not a visual pass. T23 S3 Explorer is next.
