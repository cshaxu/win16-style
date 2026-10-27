# M0 T18 S8 — VCL choice superclass coverage

The existing native BUTTON superclass path was tested with self-owned C fixtures registered as `TCheckBox`, `TRadioButton` and `TGroupBox`. On both architectures and at 96/144 DPI, the tests check Windows' actual superclass identity, three-state and radio click behavior, notification counts, borrowed font, and styled-versus-detached pixels. The historical DoubleKiller observation identifies its `TCheckBox` as a BUTTON superclass; the fixture does not establish this for every Delphi release.

No production implementation, original glyphs or ABI changed. Custom wrappers without a verified public native protocol remain unproven, and real Delphi integration remains a separate T18 closing requirement.

The same-snapshot clean x86/x64 candidate passed 87 actual package cases, zero skipped, in 346.46 seconds on self-owned private desktops. Its four tested files were published to both authorized locations and their hashes checked. Physical input was not tested. The test and package scripts add the choice witness to the release gate; no production-source files changed.
