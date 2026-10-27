# M0 T18 S37 — Runtime-owned VCL trackbar border

The observed DelphiVCL4Python 1.0.7 TTrackBar is owned by its runtime DLL,
forwards WM_PAINT to the system trackbar procedure, and uses TBS_ENABLESELRANGE.
The old package rejected that owner and the selection style. Its parent asks
for item custom draw, so overriding the whole control or treating its PREPAINT
as a default response would discard application behavior.

The candidate admits only an exact child TTrackBar after witnessing its own
class owner forward WM_PAINT to the registered native trackbar procedure and
after the existing public range, position, geometry and thread checks pass.
The runtime path preserves native painting, selection and ticks. If the app
requests item notifications and returns default for the channel or thumb,
the painter privately requests item postpaint and draws only a one-pixel edge.
Application-requested postpaint, custom drawing and skip-default results are
preserved. Non-default geometry, unsupported style, RTL, transformed DC and
unverified owner remain native. Existing v5/v6 trackbar rendering is separate.
No public ABI, original graphic resource or extra product file was added.

Self-owned x86/x64 C superclass tests check same-name fake rejection, public
selection, nested painting, application-requested postpaint, app-drawn skipped
channel, disabled state, clipping, reentrant removal and destruction. The
old package fails both widths on the missing positive attachment. The app-drawn
channel comparison changes 56 pixels only on the thumb perimeter; clipping
changes 209 pixels inside and zero outside. The model is not a Delphi runtime.

With the published files and real x86/x64 DelphiVCL4Python windows at 144 DPI,
the controls attach and change 294 pixels entirely on the public channel or
thumb perimeters versus the old package. The selection 2..7, 11 ticks and
position 0->5->0 were observed and restored; title top 26 pixels were equal.
Each self-owned process closed and its observer restored its import slots.
This supports only the tested runtime and bounded border, not all VCL controls,
all custom-draw combinations, physical input, or full Win16 trackbar fidelity.

A short GDI count initially rose for both the candidate and system controls.
In 384-round x86/x64 self-owned comparisons, both paths reached 57 objects at
round 352 and remained at 57 for the last 32 rounds. Routine tests report a
short sample without asserting a stable whole-process count; the longer mode
checks a late plateau. This does not establish zero process-wide resource
growth or identify the system's initial count increase.

The clean GCC 16.1.0/Ninja/ccache snapshot SHA-256 was
`df08751795146b01e5e7f002e1953b04d11edf7e876d5b460771d10e3fbfadc0`.
The actual same-batch package passed 24 selected tests with no skips in 95.16s,
including native v5/v6 trackbars, VCL models, font, menu, launch and propagation;
other package cases and physical input were not run. Both installed locations
passed the x86/x64 entry probes. The four tested hashes match both locations:

| File | SHA-256 |
| --- | --- |
| demo.exe | `51258d51ab547a0548c1ab81ef682af45c8229dbc439904d51e8097982efa43b` |
| ntw16s32.dll | `6eb5129b4e6f82fdbd943fadc9953c3b799516646d2e3ad6603b1b8224cb257d` |
| ntw16s64.dll | `5d791db707a52858dec6ea5deb7b65c66516ea760cb2d7e1632074ec04e84388` |
| run16s.exe | `28a465b122e8c733ecaa1c718b18d5f87efa638155940fd58eedf3850c7d1192` |

P1 `5a78183` was pushed; all 17 changed files were retrieved from the actual GitHub commit and matched byte for byte. The larger T18 74-row audit remains open;
the short whole-process GDI variation and untested real-app combinations are
not converted to success claims.
