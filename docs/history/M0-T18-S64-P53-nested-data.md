# M0 T18 S64 P53 — nested VB6 Data observation

The released P52 package was examined with a second, independent x86 VB6
sample: `data1.ocx` at commit `78466fa88c59351ab9b4855937ae885e357e241d`
of `vinodkotiya/VB-Projects`. The original sample SHA-256 is
`0126b47c68a8e4e60f83444804f5dab15a5fe543b89c7886f46d1eca9e076403`.
It is a research-only input, not distributed or required by this project.
The adjacent source and binary do not have matching control counts; this
observation does not assert that the source describes this exact binary.

A private copy changes its one hard-coded database path to a private relative
path (copy SHA-256 `1c60d48e3d65c734f729a9fad096d851e98697d4297f4f2c723d0cdda753ae53`).
The scratch database is a private copy of Northwind, SHA-256
`444627c1ad773627c7bb06ecbc19eee4d8d32c1738eff957d024c6c07c5ccad8`;
its digest was unchanged after the observations. The existing private DAO3.6
bridge and activation manifest were used; no system COM registration was made.

The native OLE host observed `ThunderRT6Data` owned by
`C:\Windows\System32\MSVBVM60.DLL` beneath `ThunderRT6UserControlDC`.
The probe moved its own control into the visible client area at 96 DPI, then
processed the pending messages. In the original and styled runs respectively,
the attachment result was 0 and 1; both runs exited 0. In the 140×22 client
capture, the four navigation regions differed by 364, 342, 342 and 361 RGB
pixels; the central 70×22 region differed by zero pixels. An immediate capture
before dispatching the pending messages was identical, so attachment alone or
an early capture must not be used as proof of final drawing.

The prior diagnostic P48 package crashed in this fixture with
`0xc0000005` after its attachment expectation failed; that diagnostic package
is not the released package and its crash has not been attributed. The
released P52 observations above completed successfully. No navigation, input,
other DPI, VB5, production use of this OCX, or normal user interaction was
tested here. This is evidence for one nested VB6 rendering path, not general
OCX support. No product or test source was changed and no binaries republished.

The four released files in `assets` and `O:\winnt\system32` had matching
SHA-256 values, respectively: run16s `21a44f972d230e5d1cbaf4998852d632b15bc6c1480df0bd5f9863897f47bcc9`,
ntw16s32 `fe2b082262a237aef2f861677d35b9f6c413890078ab12f52b82468fe1269770`,
ntw16s64 `3d45cc4c18dbfbf5dfeb575fe95dbec167b0534f0098c88cd5bdc54942ee4290`,
and demo `45d11a46dcda4579963e94b47c52ffd387e079f4530989f1dfb4a861e4840223`.
