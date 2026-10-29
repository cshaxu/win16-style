# M0 T18 S64 P181: bounded common-control verification

No product source changed. Real owned MadShow and DoubleKiller runs confirmed bounded control observations as recorded in [application regression](M0-T18-S64-user-app-regression.md); neither complete application run passed because process exit timed out even in bare controls. The temporary in-process observers were scratch-only and removed before close. Physical input, natural process shutdown and the full VB/Delphi matrix remain unverified; T18 and S64 remain open.

A fresh snapshot built both widths using GCC 16.1.0, Ninja and ccache 4.13.6. Source snapshot manifest SHA-256: b2f3c4e066bed881c8a4f6843ab784c8848a09c94c8e2bd32d9e941491d7eda3. Initial sandboxed x86 ABI configuration exceeded the 600-second architecture budget, without a publishable package; a separate unsandboxed clean build succeeded. Actual-package focused testing passed 16 cases with zero skips, including dual-width VB5/VB6 checklist and graphical choice models. This is model evidence, not a complete real-application test.

Rebuilding changed all four binary hashes. The tested batch was published to the project assets root and O:/winnt root, with backups and post-copy SHA-256 verification. No product source or resources changed. Candidate hashes:

- run16s.exe: 2aa8fb136c273d622372a58c26a5405c149ca01a19673dec64667f701e52a2d7
- ntw16s32.dll: 202b4f5c5894eec919ab3312b35d4aa00527d75b5b5f2085860a992e31faf350
- ntw16s64.dll: f79a1e1655cfae4bb984a0fc1cc3e2dfe2f7d30dd7efc7dbef5dfbf4413ada26
- demo.exe: 36748641518610b747b04198443c6103a6b8539a37d4b1b7d64d8164eaba2961

Build: `tools/Build-Package.ps1 -Task 18 -RunName s64-p181-common-verify-retry`. Focused test: `tools/Test-Package.ps1 -Task 18 -RunName s64-p181-common-verify-test` with eight VB model cases and mandatory baseline. Publish receipt: `build/t18-package/s64-p181-common-verify-publish/result.json`. Runtime paths in the preceding real-app checks refer to the earlier P180 binaries, not this rebuilt package.
