# M0 T18 S53 — radio group frame investigation

Self-owned DelphiVCL4Python 1.0.7 radio groups on x86/x64 at 144 DPI use a
runtime-owned TRadioGroup containing two TGroupButton children. The installed
product leaves the group container native. The scoped runtime import observer
saw a FillRect with a parent-sized rectangle, a complex clip and translated
viewport; it saw no DrawEdge, DrawFrameControl, Rectangle or PatBlt during the
measured redraw. All five installed observer slots were restored. This does
not exclude other APIs or drawing paths.

A separate prior self-owned diagnostic build recorded Delphi runtime
DrawThemeBackground BUTTON part 4 calls in both widths, including the actual
group window DC with rectangle (0,12)-(250,120), and memory DC calls when
children draw. Therefore matching only part, class name or rectangle is not a
safe owner boundary. A scratch-only prototype restricted to the group window
DC drew a black frame after the theme call; captures show its top line passes
through the RadioGroup caption. The title region was not captured by the
observed DrawTextW or DrawThemeText calls on that window DC. That observation
does not prove the title is absent or identify its actual drawing API. The
prototype was rejected and was not merged or shipped. The x64 observer first
looked for ntw16s32.dll and falsely reported a missing product; after fixing
its own bitness check it observed the real ntw16s64.dll and root attachment.
Owned instances were closed. No physical input or other versions were tested.

Production source and original resources remained unchanged. An independent
clean GCC 16.1.0/Ninja/ccache 4.13.6 dual-width snapshot manifest SHA-256 was
`a9586ea4c1a5797646b3f1c4d22d7184f21e19ce14e07546c7c164ebe62bf1a4`.
PE and ABI checks passed; 12 actual four-file focused cases passed in 6.34s
at four jobs, no skips. Other cases and physical input were not run. Although
source did not change, fresh binary hashes changed; the tested four files were
published to assets root and O:/winnt/system32, with installed x86/x64 group
model passing and both destinations matching:

| File | SHA-256 |
| --- | --- |
| demo.exe | `b048230e1df0171e9efe68dff28805da08fc11d9dd357500e7b85f921a7c18f7` |
| ntw16s32.dll | `40b6fbeeb06043da2950efeae9f9cfff79f3c2134d64d87b3abd465a0730b6e7` |
| ntw16s64.dll | `b6ed731d2ac2b99c2ff7d8ec8709c56ea5e827f36258f82b9bdcc3909f12d292` |
| run16s.exe | `c131d8574f2bdd8e4ae71f266149ae256e5b7918f9cdf5b51e2564deb8d85233` |

The container frame and title-safe contract remain in TODO; this limitation
does not block independent T18 control work. M0/T18 remain open.
