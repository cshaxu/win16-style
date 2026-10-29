# M0 T18 S64 P3 — real VB5 and VB6 intrinsic controls

The originally sourced VB5 and VB6 intrinsic samples retained in the S4
research directory were rerun in isolated, owned 32-bit processes. Source
identity, provenance and license are recorded in [T17 S4](M0-T17-S4-vb-intrinsics.md);
the samples are not shipped or required for builds. The VB5 sample loaded its
fixed MSVBVM50.DLL; the VB6 sample loaded the system MSVBVM60.DLL. Both ran
bare and with the S64 P2 published assets DLL at 96 DPI and exited normally
with an empty owned job.

Each of the four runs passed public protocol checks for four option buttons,
four checkboxes, two combo boxes, one list box, two text boxes and two
scrollbars. These covered option sibling clearing, two-click checkbox cycles,
combo selection/dropdown cancellation, list selection, text change/restore,
and scrollbar range/position reads. In the styled runs the two scrollbar
arrows matched 225 original pixels each, both combo arrows matched 32 original
black pixels each, and the checkbox/radio checks each matched 182 original
pixels over sixteen captured states. Logs, exact paths, hashes, processes,
window counts and screenshot hashes are summarized in [VB evidence](../etc/evidence/vb-current-s64.json).

These are synthetic message and pixel checks on particular 32-bit samples;
physical keyboard, IME, notification counts, actual dragging, edit replacement
undo, additional DPI/runtime versions and the deferred OCX/DriveListBox cases
remain unverified. No binary or source changed in P3. The existing P2 clean
x86/x64 build, 14 focused package tests and eight installed hashes remain the
product baseline; both actual samples loaded its assets 32-bit DLL. P2 commit
6022aa2 and all ten remote files were verified. M0/T18/S64 remain open.

## P181 current-package recheck

The same retained original x86 VB5 and VB6 sample binaries and runtimes were rerun using the P181 clean-build observer and assets package, bare and styled. All four runs exited 0 with SAMPLE_PASS and zero active owned processes. Each recorded option=4 checkbox=4 combo=2 list=1 edit=2 scrollbar=2 and the existing synthetic protocol assertions; the styled verify mode also checked the original glyph pixel assertions and enumerated assets/ntw16s32.dll. This repeats bounded 96 DPI x86 sample evidence, not physical input, IME, all variants or 64-bit VB. The first command attempt supplied the x64 observer path, which did not exist; it failed before launching any sample. Corrected logs are under build/t18-s64-user-app-regression/p181-real-vb5-* and p181-real-vb6-*.
