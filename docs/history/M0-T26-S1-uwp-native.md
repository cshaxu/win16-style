# M0 T26 S1: shared native UWP title services

The native ApplicationFrame title selection, full-screen UI Automation query
and asynchronous invocation moved from `ntw16s-dll/uwp/native` to the pure C
`lib-style/uwp/native` component. Its API names are style-neutral. The worker
retains the existing COM initialization, target identity checks, per-call
timeouts and module-reference lifecycle. The native title handles are borrowed.
The startup/target coordinator and Win16 frame painter remain product-specific;
this change does not add Win2000 UWP rendering or claim WinUI coverage.

The existing Win16 hook and target coordinator, launcher, and manager test
consumers link the common static target. The old product-side implementation
was removed. The native fixture moved alongside the component and now runs
directly with self-owned windows; its CTest entry no longer invokes the
private-desktop driver.

Both architectures incrementally built the product DLL and common component.
The x64 launcher, activation-resolution fixture and UWP manager fixture also
linked. Direct x86/x64 tests passed title selection, hidden and ambiguous
children, wrong-thread and invalid-target cases; x64 Calculator resolution
passed. Undefined-symbol inspection showed only Windows and C-runtime imports,
without Win16, Win2000 or product-private symbols. Actual UWP full-screen UIA,
live activation, physical input and the complete regression matrix were not
verified by those fixtures. The existing per-call UIA timeout is not a proven
overall worker deadline.

The clean x86/x64 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 build passed;
snapshot SHA-256 is `a437d510c062b18aa16f5920789b790d0ac4cbea4d300fb11843f30eb78ffa70`.
The eight-file package was published to `assets` and `O:\winnt` before testing.
Eleven post-publication direct checks passed: native title, UWP frame layout,
shared window lifecycle, published Win16 menu and Win2000 scrollbar geometry
for both architectures, plus x64 Calculator resolution. Hashes matched the
candidate and both destinations. No private desktop or computer-use was used.
Live UWP activation, real UIA full-screen invocation, physical input and the
complete matrix remain unverified; T26 S1 remains open.

| Published file | SHA-256 |
| --- | --- |
| demo.exe | `371d4475b7000d08d78f77302af84d92f2b26bcfb15569dac3a19bda6b497ce9` |
| ntw16s32.dll | `46686f851b7aed9241d7920818c966b914a59ad8b5228fe1a1a9741f41d7ebfe` |
| ntw16s64.dll | `9f780197326f6b89d3b33f62b977b46e2df5f207d1527d4a0934ec49df8ba394` |
| ntw2ks32.dll | `da294a2160646f1172e384121e5bd18bb5533f0fa956fcb09bf9639c5b7f6faf` |
| ntw2ks64.dll | `32a32076b36fbe53722631b4e61fdb20ccffa0601fd14fe7c0b53ca09fd8889d` |
| ntwshk32.dll | `906e5b00968c96594bbe33b7bc1e29c1adcea94fa38e1f77c898617d97b215ca` |
| ntwshk64.dll | `f9ab7166bfc587dbd338a3815d512a2b3bbb2b4827021629ec7d0bb527149180` |
| run16s.exe | `2e802b7b436b7558d4346f990625ca37b9a9a3b772d82a99f7fe80db5a0ab763` |
