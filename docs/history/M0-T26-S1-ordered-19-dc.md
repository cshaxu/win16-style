# T26 S1 顺序第 19 项：DC 坐标检查

组合框和滑块共用原有非 RTL、MM_TEXT 及零窗口/视口原点判定； Win16 颜色、图案和布局仍由风格侧提供。滑块尚待顺序审计。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共依赖检查通过；先发布至 assets 和 O:\winnt，八文件摘要一致，再对发布包运行双位宽 DC 坐标测试、VCL 颜色与驱动器组合框、滑块自建夹具，全部通过。未创建或切换隔离桌面。真实第三方应用、其他风格与全部 DPI 未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 6a176fc73093e986c19a04c69a0e8d521ae5bcbf28945f23a70cabb8446ec29d |
| ntw16s32.dll | 7eb498f3a79a23b0fd498b7671c462a572b7b3e665a9fdcf71d105cc47316e79 |
| ntw16s64.dll | 4b9070370319118806ad6367433d56ae59b84d35f43d2a419252bea31880bae2 |
| ntw2ks32.dll | 9a82efaaaeb7bd870f8d69894dc1814f62235ec046ab7cc918dcf72c71b64404 |
| ntw2ks64.dll | 734a1055fad4a9632e2d74f9785678f82bede0c5bd953cad1f698b723b636af9 |
| ntwshk32.dll | 8cd37527c497331c61caea993ec6abe3e28b82ae8638ac7ccd6971ffc5666736 |
| ntwshk64.dll | a2fc4a20c92326e87ea6b5f80792bdf35dad31004ea4a70d580ce78f5882f09d |
| run16s.exe | 217b068b079b6946f7b05df3434c78087ecc492ba686a3730b0ad926b1729293 |

既有导航焦点、Win2000 DWM 恢复与现代窗口覆盖限制继续开放。
