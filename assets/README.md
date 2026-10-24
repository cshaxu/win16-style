# Win16 Style binaries

Keep these four files in the same directory:

| File | Architecture | Role |
| --- | --- | --- |
| [run16s.exe](run16s.exe) | x64 | Application launcher |
| [ntw16s32.dll](ntw16s32.dll) | x86 | Styling and descendant hooks |
| [ntw16s64.dll](ntw16s64.dll) | x64 | Styling and descendant hooks |
| [demo.exe](demo.exe) | x64 | Native controls demonstration |

```powershell
.\demo.exe
.\run16s.exe .\demo.exe
.\run16s.exe "C:\Path\To\application.exe" [arguments]
.\run16s.exe --wait .\demo.exe
```

Styling and supported descendant propagation are enabled by default. Console
targets wait by default; GUI targets normally return after launch. `--wait`
waits for the initial target and returns its exit code.

The [release manifest](../docs/etc/release.json) owns the current build identity,
SHA-256 hashes, architecture checks, and validation scope. These are development
binaries, not a claim of compatibility with every application or DPI setting.
Original graphics and their [provenance](../src/ntw16s-dll/resources.md) are retained;
the project license does not replace third-party notices. Review the
[project README](../README.md) for screenshots, scope, and licensing details.

For a source build, follow the [build notes](../docs/etc/operations/BUILD.md).
A normal user does not need to copy anything into a Windows system directory.
