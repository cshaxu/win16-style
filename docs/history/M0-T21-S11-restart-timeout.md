# M0 T21 S11: restart and bounded notification retry

On a private desktop, the lifecycle probe owns its target processes and controller. It checks duplicate startup, forced controller termination, host exit, a new unstyled window after the crash, a subsequent session attaching both old and new windows, and their restoration on normal stop. It does not terminate or inject existing user processes. An additional host-failure path remains covered.

The previous package intermittently loaded the x86 DLL without attaching the target window. Instrumenting the initial enumeration identified `SendMessageTimeoutW` returning 1460 (`ERROR_TIMEOUT`) for the target class `Win16GlobalE2ETarget` at the original 50 ms limit. In four isolated x86 baseline runs, three failed and one passed; in four runs of the candidate with the retry, all passed. Module presence was separately observed and is not treated as proof of attachment. This evidence establishes a concrete timeout failure, not that every possible failure has the same cause.

The controller retries only error 1460 once, for at most 200 ms and no longer than the remaining existing sweep deadline. Other errors retain their prior handling. Attachment and independent ownership semantics are unchanged; no process-level exclusion is introduced. A trial `WaitForInputIdle` returned `WAIT_FAILED` for the console-subsystem fixture and was removed; it was not relied upon for synchronization.

The probe covers controlled target windows and the tested session lifecycle, not all desktop processes or all possible crash timings. Explorer.exe remains excluded; UWP/XAML remains out of scope. Unexpected termination can leave visual remnants until a later session or other repaint.

The final clean GCC/Ninja/ccache x86 and x64 build passed the source, resource, governance and ABI gates. The same four-file candidate passed 18 focused package cases with no skips in 38.8 seconds, including both restart widths, host failure, ordinary/global coexistence, 96 and 144 DPI paths, multithreading and both system-menu widths. This was not the full integration matrix or physical input acceptance. The tested files were published to both `assets` and `O:\winnt`, with matching SHA-256 at both destinations:

- `run16s.exe`: `5df458abd619d93488f60d9cf8533ec35d3d92eceff154e52c45a7aae0248d9c`
- `ntw16s32.dll`: `9d972f09591e36257e559c04f53495856fce9a572e8469731259c7f625cc1bb2`
- `ntw16s64.dll`: `21c7d3a0e3c039363a36734b18aea594c068b0aee8f527108dd0263a25a9b613`
- `demo.exe`: `e5446f5fddd2c46ecdb2ba3d89742b230fb62d851fb48720a16b45135a859b2a`
