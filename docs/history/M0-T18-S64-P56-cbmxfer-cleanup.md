# M0 T18 S64 P56 — CBMXfer observer cleanup

P55 recorded a styled-sample shutdown timeout after opening the self-owned
viewer. A follow-up 96-DPI control used the same fixed sample and opened and
closed the viewer without showing, adding to, or clicking the checklist. Both
the unstyled and styled runs then timed out: in each case the main form was
gone, while a hidden `CBM-Transfer Options` form and runtime main window
remained. Thus the original evidence does not establish that the style hook
caused the timeout. The unstyled run in P55 that exited successfully used a
different sequence and does not disprove this control.

A scratch-only observer next sent `WM_CLOSE` to the exact hidden Options form
on the self-owned UI thread before closing the main form. At 96 DPI, both the
unstyled and styled open-only cases destroyed that form, exited with code 0,
and reported an empty owned job. An initial styled attempt failed before
opening the viewer because the preceding run had updated its private INI; the
observer was rerun after restoring the private fixture configuration.

The same explicit cleanup was then applied to the actual checklist observation
at 144 DPI. Both the unstyled and the released `assets` run read selection
0→1, closed the hidden Options form, exited with code 0, and reported an empty
job. The styled unchecked/checked image hashes matched the P55 final-package
observations. This verifies the bounded probe sequence with an extra cleanup
action; it does not establish how an unassisted user's normal close behaves.
The earlier timeout without that cleanup remains observable.

The source examined for context is the [original author's repository](https://github.com/sjgray/cbm-transfer/tree/6122eaa2484c4477e65157e65ba7db9734ff5027/V1)
at commit `6122eaa2484c4477e65157e65ba7db9734ff5027`; the V1 executable's Git
blob matches the unmodified research binary `836dd3a92518243836e482445c378b52b6182ce8`.
The adjacent source describes an Options form and main-form shutdown behavior,
but a source/compiled-binary equivalence is not assumed solely from colocation.
The source and binaries are research-only and are not project dependencies.

No product source, graphical resource, ABI or binary changed. The four P55
files in `assets` and `O:\winnt\system32` remained identical by SHA-256. The
original 144-DPI and other VB controls' limits remain as recorded in P55.
