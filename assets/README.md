# Win16 Style binaries

Keep these eight files in the same directory:

| File | Architecture | Role |
| --- | --- | --- |
| [run16s.exe](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/run16s.exe) | x64 | Application launcher |
| [ntw16s32.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntw16s32.dll) | x86 | Styling and descendant hooks |
| [ntw16s64.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntw16s64.dll) | x64 | Styling and descendant hooks |
| [ntw2ks32.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntw2ks32.dll) | x86 | Windows 2000 presentation |
| [ntw2ks64.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntw2ks64.dll) | x64 | Windows 2000 presentation |
| [ntwshk32.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntwshk32.dll) | x86 | Shared styling runtime |
| [ntwshk64.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntwshk64.dll) | x64 | Shared styling runtime |
| [demo.exe](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/demo.exe) | x64 | Native controls demonstration |

```powershell
.\demo.exe
.\run16s.exe .\demo.exe
.\run16s.exe --2k .\demo.exe
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

## T29 theme previews

Keep each description beside its visual style resource.

| Style | Description | Visual style |
| --- | --- | --- |
| Windows 3.x | `themes/Win16/Win16.theme` | `themes/Win16/Win16.msstyles` |
| Windows 2000 | `themes/Win2K/Win2K.theme` | `themes/Win2K/Win2K.msstyles` |

The published launcher still uses the eight-file runtime above. The seven-file
package and runtime loading of companion themes are tracked in the
[T29 build contract](../docs/proposals/msstyles-build-contract.md).
