# M0 T18 S9 — EDIT superclass models

The native EDIT superclass path was tested with self-owned `TMemo` and `TMaskEdit` named models on x86 and x64 at 96 and 144 DPI. The probe checks multiline text replacement, undo, setting read-only, scrolling, font ownership and styled-versus-detached print pixels. Existing product code was unchanged.

The mask fixture does not implement Delphi masking or validation; the read-only test does not attempt a prohibited edit. `TRichEdit` and actual Delphi runtime behavior remain separate evidence requirements. No test here claims physical input.

The clean x86/x64 four-file candidate passed 87 private-desktop package cases with no skips in 345.50 seconds. The tested batch was published to both authorized destinations with matching hashes; original graphics and production source were unchanged.
