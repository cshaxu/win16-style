# Win16 Style package

Place the three runtime files at the root and preserve the theme paths.

| File | Role |
| --- | --- |
| [run16s.exe](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/run16s.exe) | x64 application launcher |
| [ntwshk32.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntwshk32.dll) | Runtime for x86 targets |
| [ntwshk64.dll](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/ntwshk64.dll) | Runtime for x64 targets |
| [Win16.theme](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/themes/Win16/Win16.theme) | Windows 3.x theme description |
| [Win16.msstyles](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/themes/Win16/Win16.msstyles) | Windows 3.x visual style data |
| [Win2K.theme](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/themes/Win2K/Win2K.theme) | Windows 2000 theme description |
| [Win2K.msstyles](https://raw.githubusercontent.com/cshaxu/win16-style/main/assets/themes/Win2K/Win2K.msstyles) | Windows 2000 visual style data |

```powershell
.\run16s.exe notepad.exe
.\run16s.exe --2k notepad.exe
```

The launcher selects Win16 by default. Console targets wait automatically.
Use `--wait` to wait for the initial GUI target and return its exit code.

The [release manifest](../docs/etc/release.json) records hashes, build identity
and validation. The runtime reads the themes without installing a Windows
desktop theme. The [source build](../docs/etc/operations/BUILD.md) includes the
native demo. Original artwork and its [provenance](../src/ntw16s-dll/resources.md)
are retained.
