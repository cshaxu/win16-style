# T26 S1 顺序审计第 8 项：原生过程校验交付

将 Win16 的 Unicode 原生过程归属检查与 ANSI DBEdit/DBComboBox 路径中相同的检查合并至 `lib-style/common/control_native_witness.h`；返回值是借用模块，不增加模块引用。实际 Win16 调用方直接使用，类型和绘制策略仍归风格侧。第 8 项其他职责仍在审计，本记录不代表整个文件过关。

MinGW GCC、Ninja、ccache 双位宽干净构建和公共库依赖边界检查通过。先发布同一候选到 assets 和 O:\winnt，八文件摘要一致；随后以已发布 `run16s.exe` 验证双位宽 ANSI DBEdit 及 Unicode 轨迹条自建夹具，两者通过，并覆盖错误身份、绘制和清理。公共校验测试双位宽通过。采用夹具的直接工作入口，不创建或切换隔离桌面。真实第三方程序、全局模式和其他风格尚未覆盖。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | de6ea639939065f8ef2ba6e5ccc0a9d7ae158dd12cafccac90af8289c7407c31 |
| ntw16s32.dll | 50a2a65ff4435ee8ccb44c4e4d2c65222561ff4d6dab917fd3829219688bcd7e |
| ntw16s64.dll | 3a3121351dd7cd43cbaaa7ba3e1cd32d0dba32c478dbd6b82c75ee94fc9b5e02 |
| ntw2ks32.dll | 9adfd12dd616916ca732e76f654deae9ea24d2dc650cc8816c953e0e65c1e501 |
| ntw2ks64.dll | 7a5883bf4ba2873ce66447d2a93e1280d0f72011dfb2b1fdaaa44f2b1dbb1954 |
| ntwshk32.dll | 37a47f2db38170dfaa073d9fe5a50874a08600524f351a76270657bc0871e7f9 |
| ntwshk64.dll | 29fd88ae80e4a1cee1c5d0c1252d0f2da155d19c0eec14c220adc2f688e8663d |
| run16s.exe | 76635bd3846b958cc3146da6b7343c6e55babef6c09a2f857d9a1faa50130186 |

既有导航焦点与 Win2000 DWM 恢复失败及现代窗口/全局未验证范围继续开放。
