# M0 T18 S2 Delphi native demo correspondence

## Result

S2 closes the demo-correspondence phase without altering the product. The frozen S1
seed contains 74 VCL UI families: 35 have a bounded, already-existing pure-C
USER32/COMCTL32 demo model and 39 deliberately have no deceptive model. The
complete one-to-one ledger is [delphi-demo-mapping.json](../etc/evidence/delphi-demo-mapping.json).

The mapped group means only that the demo can exercise the same public native
interaction shape. It does not prove a VCL or FMX object, wrapper, property,
custom drawing route, image ownership, action dispatch, database binding or
application canvas route. Canvas, data, embedded and nonvisual rows remain
native until a real owner and content-neutral public protocol are evidenced.

## Verification and boundaries

The ledger set check passed exactly: 74 seed entries, 74 mapped entries, 35
native models, 39 preservation boundaries, and zero duplicate, missing or
unknown names. `tests/demo-exe/boundary.py build/.ninja/x64/demo.exe` passed:
the demo remains an x64 GUI program with only the declared Windows/system-CRT
dependencies, no delay imports, exports, private component headers or style
consumption. Its observed SHA-256 was
`32fc3f929f97bfe0a87249df130fb4be3291a24c2fee9c0ca90455074928a271`.

The self-owned `demo-observe.exe` returned
`PRIVATE_DESKTOP_CREATE_FAIL error=5`; this is an exact display/interaction
skip, not a pass. No current-desktop or computer-use fallback was used.
Governance self-test passed all 13 negative cases. No product source or binary
changed, so no artificial clean build, package or deployment was performed;
the existing assets and `O:/winnt/system32` four-file pairs were hash-identical.

## Handoff

S3 begins real VCL wrapper/protocol evidence for a narrowly bounded basic
receiver set. It must use a self-owned real VCL instance where the environment
permits, distinguish wrapper ownership from native child classes, and retain
an exact `error=5` skip if private desktop creation remains unavailable.
