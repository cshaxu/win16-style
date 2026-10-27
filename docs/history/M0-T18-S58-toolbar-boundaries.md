# M0 T18 S58 — toolbar clip and count boundaries

The self-owned S57 C module was extended to compare styled and native client
pixels for a clipped button and for 64/65 unique text-only toolbar items.
The first attempted clipped fixture failed because the native toolbar
re-applied its automatic height: it remained 26px high with a 22px item and
was correctly styled. Adding CCS_NORESIZE to this fixture made its client 6px
high while its item remained 22px, and the native fallback comparison was
zero changed pixels in both widths. This was a fixture correction, not a
product defect.

With 64 items the visible toolbar pixels changed by 5,928 per width; with 65
they changed by zero, proving the cap boundary for these self-owned samples.
The test also reruns fake forwarding, unique zero ID, duplicate ID, image,
application skip, synthetic click, reparent and cleanup. This does not measure
all high-item-count workloads, physical input, images, other VCL versions or
DPI. No production source, original graphics or ABI changed.

An independent clean GCC 16.1.0/Ninja/ccache 4.13.6 build snapshot manifest
SHA-256 was `fb60773d7cb1a521df10f6e8056f69d2c312bd1fb33ed2fc84723097f2a0d69e`.
PE and ABI checks passed; 14 actual-package focused cases passed in 6.77s at
four jobs, no skips. Other cases and physical input were not run. Because
fresh binary hashes changed, the tested four files were published to assets
root and O:/winnt/system32; the installed x86/x64 model passed, with both
locations matching:

| File | SHA-256 |
| --- | --- |
| demo.exe | `80eeaf4bc2b526d10afb172747b7e19caffe889ded54853fbd36629bb49a2399` |
| ntw16s32.dll | `1840b626cbe571d8cb5b4c0917003d6338ce276cd276815ecad6f1575db3a363` |
| ntw16s64.dll | `df0de8ebd865f84dba0126da2d3e871b61693882d04a95c08b6249ce0bc216ef` |
| run16s.exe | `3163aa52b7e55ea1a9fcb31cfea3038d3c56a40f58050d25169463a105266c3f` |

M0/T18 remain open; full task integration remains separate.
