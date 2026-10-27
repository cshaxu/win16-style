# M0 T18 S64 P6 — graphical choice image pixels

The existing self-owned VB graphical-choice model compared whole-control
hashes and image handles, which could pass if the face changed but the image
was not drawn. It now separately counts the existing 16x16 test sentinel:
all 256 original-color pixels must be present in both bitmap and icon
controls, unchecked and checked. Existing state, disabled, ownership,
retirement and grouping assertions remain. These test-only pixels are not
project graphic resources; no product graphic was replaced.

Both x86 and x64 fixed models passed under the current DLL. With a scratch-only
fault DLL that suppresses image drawing, both failed at the bitmap-pixel
assertion. With a second scratch-only DLL suppressing only icons, the bitmap
check passed and both failed at the icon assertion. Neither fault DLL was
published. The disabled path is still checked for a whole-image state change,
not for exact disabled-image color. The fixtures simulate runtime-owned public
protocols; they are not real VB5/VB6 runtimes and do not prove actual Picture,
DownPicture, DisabledPicture, owner-draw or other complex VB variants.

Clean GCC 16.1.0/Ninja/ccache 4.13.6 x86/x64 snapshot manifest SHA-256:
`306a757e4320a4f8a4f178da050b628631ca886dca99446e1ac39ee72d4dee13`.
PE/ABI, source, original-resource and governance gates passed. Fourteen
focused actual-package tests passed in 11.73 seconds at four jobs, no skips;
other cases were not run. Both installed launchers passed both bit widths.
The rebuild changed binary hashes, so the four tested files were published to
assets root and O:/winnt/system32 with matching hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `33f1c8a57e4d709816fcaa3651bd7a4323e6bc23ebb6b6db19debf5e63f19acb` |
| ntw16s32.dll | `85f6e863f8a0e5584d4ab889a0b67cbdc4db64207a4b1b519e92bc0bb6ba6a31` |
| ntw16s64.dll | `86b3f72ccfd7101b409ea471abe6ac7aaa2c077311a339cadf9f86e1f0baff8b` |
| run16s.exe | `6b27051fb93eae02e59a5696f189867bb2d6758bee0867de061254ce5b2bf3d2` |

Test/build wiring scope: 8 additions and 3 deletions, net +5; product source
delta is zero. Prior user-authorized OCX/DriveListBox deferrals and the S64
VCL work remain open; this P does not declare the overall VB goal complete.
