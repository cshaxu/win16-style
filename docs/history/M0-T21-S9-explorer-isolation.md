# M0 T21 S9: Explorer isolation investigation

Global mode currently excludes Explorer by executable name under the Windows directory, so File Explorer is not styled. The user's global objective makes this an important coverage limitation. Removing the exclusion would subject a shared shell process to the global hook and late attachment; neither loading a DLL nor the existing synthetic fixtures prove Explorer window identity, behavior, safe restoration, or separation from the user's existing shell.

A native read-only observer was run under the existing job-backed private-desktop driver. It launched only its own Windows explorer.exe process, observed only the private desktop, and cleaned up only its own process or windows. Three invocations were tried: a `/separate` directory command (PID 37400), a direct directory command (PID 36740), and `/separate` alone (PID 38048). Each new process was in the test job, exited with code 1, and produced no visible Explorer window owned by it; no other Explorer window appeared on the private desktop. No global hook was installed for this observation, and no existing user Explorer was closed, injected or otherwise operated on. This does not establish that the requests were transferred to an existing default-desktop shell; the cause of exit code 1 is not established.

Because no independently owned Explorer GUI instance was observed, this S did not remove the exclusion or claim Explorer compatibility. A VM or another genuinely isolated Windows session with a separate shell could establish a safe target and allow existing/new window, UI and shutdown tests. This is an investigation limit, not evidence that styling Explorer is impossible. T21 remains open with this coverage gap; no process-level exclusion architecture beyond the already published design was introduced.

Microsoft documents a shell setting that separates desktop/taskbar and folder-window Explorer processes in its [shell debugging guide](https://learn.microsoft.com/en-us/windows/win32/shell/debugging-with-the-shell). That is general guidance, not evidence of this machine's actual window ownership; this investigation did not change that user-wide setting.

Final clean build, actual-package checks, deployment and remote verification are recorded below.

`s9-explorer-clean` passed the clean x86/x64 GCC/Ninja/ccache build and source, original graphics and ABI gates. Its actual four-file package passed 13 focused cases with no skips (`s9-explorer-check`). This package test did not include an Explorer attachment test and must not be interpreted as one. The tested batch was published to assets root and O:/winnt root:

- run16s.exe x64: `bfdfd4164985eb5ba95c3f37f075c9a98805d407f2ca87e560582992a3cdc0e7`
- ntw16s32.dll x86: `27304c6b2ece801cec4414461f9dfd7508975754b0b597907355a9c94503658d`
- ntw16s64.dll x64: `86a682c6f14c8cceb2e239dfa28bb0bbb1fde8f4bfa22d5596b305c56e855e75`
- demo.exe x64: `0d89f5b025707c312a92b3a345f35b561916469af56c36a64c81aae0a9ea12aa`

No product source changed. The new clean-build binary hashes differed from S8, so both deployment locations were updated per the delivery rule. Remote verification follows the delivery commit.
The staged diff before this accounting line covered six files, 44 insertions and three deletions, including four rebuilt binaries. There are no production source or test-suite changes; the short-lived observer was task scratch and its owned-process observations are recorded above.

Later S13 read-only inspection found three `Win16S-Dialog-*` desktops still holding Explorer COM-server windows, with creation times and titles consistent with the S9 experiments. The S9 observer stopped as soon as the directly launched Explorer process exited, so its `other_windows=0` observation did not cover delayed COM activation and did not prove those desktops were fully reclaimed. Exact causation and ownership of the system server processes remain unproven; they were not manipulated. See the [S13 investigation](M0-T21-S13-vb-global-investigation.md).
