# M0 T18 S52 — VCL radio child lifecycle

The new S52 self-owned model demonstrated that S51 kept the style attachment
after a child was reparented from its original TRadioGroup to a different
parent; this failed in both widths against S51. The prior ordinary real sample
did not cover that transition.

The installed radio now records its original parent. The existing control
subclass checks the original parent and witnessed radio identity at each
message. On mismatch it lets the native transition run, retires the exact
attachment, restores the application's font and schedules a redraw. A test
also changes a second radio to BS_OWNERDRAW and verifies retirement and the
original font; it does not establish that every role or exception transition
is covered. The model passed fake classification, native selection, space key,
reparent, owner-draw role change and cleanup on x86 and x64. Before the final
redraw addition, real DelphiVCL 1.0.7 x86/x64 samples at 144 DPI retained
ordinary attachment and synthetic click mutual exclusion. The last redraw
addition affects the invalid-identity path and is covered by the final model;
the real application was not rerun after that small addition.

The independent clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest hash
was `41f4769ff5525b920f9f91da1fa14e5376207a959141c0c21d075682520a1c50`.
PE and ABI checks passed; 16 actual-package focused cases passed in 11.63s at
four jobs, no skips. Other tests and physical input were not run. The same
four files were published to assets root and O:/winnt/system32; installed
entry x86/x64 passed the model, and both locations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `dc3e7d2e1c3ab779206a53f5673eadd9f56cc75dfb460a89ca1b774858fab6d6` |
| ntw16s32.dll | `a6da1eb0e87fddb37c7ed8259687d8d114499904b7ca6f2fd568074103866608` |
| ntw16s64.dll | `710a2b00248c919aea7639eac8d627c04e3a1a8dffda6192414e7dc3a86f941a` |
| run16s.exe | `351cc447318db196733c6865bb3e36e5faa9ed79d50862f41b36465d2750dae0` |

Search `rg -n 'ntw16s_group_button_valid|group_button_identity' src` locates
the scoped guard and initial identity; other controls retain their existing
paths. Original graphics, ABI and application state ownership remain intact.
TRadioGroup container chrome, further role/parent variants and full real
application integration remain open. M0/T18 remain open.
