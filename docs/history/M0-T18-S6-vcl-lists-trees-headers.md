# M0 T18 S6 — VCL lists, trees, and headers

S6 completed the public-protocol review of existing list/tree roles and added the missing exact `THeaderControl` role. It accepts only the main-EXE registered native-superclass wrapper; native `SysHeader32` stays separately exact and a same-name `DefWindowProc` impostor is rejected.

The restored session allowed self-owned private-desktop native verification: x86/x64 common-controls v5/v6 header probes and 144-DPI witnesses passed. The final clean four-file package passed all 87 cases with no skips in 351.34 seconds; physical input was not tested. A DPI-aware TextBox visual fixture was corrected to capture both client and owned non-client print surfaces; the previous client-only hash was a sampling blind spot, not a renderer failure.

This does not replace the final real-Delphi integration phase. No graphics resource, ABI, component, or product-file count changed.
