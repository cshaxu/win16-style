# T26 S1 顺序审计第 3 项交付

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布 assets 和 O:\winnt，再验证正式包。

`win16-common/font.h` 的字体资源和租约声明保留风格侧；窗口标题与菜单字体刷新声明原先混入基础头，现移至实现所属的 `win16-window/frame_font.h`，调用方显式包含该头。新增头文件已纳入顺序审计台账，文件总数由 150 变为 151。

双位宽正式 DLL ABI 测试、Win16 基础库/窗口库/控件库链接与隐藏现代外框生命周期测试通过。隐藏窗口不能代替真实可见 UWP/WinUI 验证。

导航焦点、Win2000 DWM 恢复等既有失败及真实全局等未覆盖项仍开放。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | be4d32ba63bf4cfdb7e0417eeba2e42e2d01a9a9927172ce8e3ecf7e23a4db56 |
| ntw16s32.dll | f706e5ce1cd8dd1d57644a4786418f470f6996e0386efec49fda666ab71ded4c |
| ntw16s64.dll | b1e40fa7652a9528f29a16a8a742b17f8015a9e85b4ba771e8fd3e08806b502e |
| ntw2ks32.dll | 9a0ae4565c4cd1f0c426481937d8dd9f84a5a87673d7f3d368823f7d1ac38308 |
| ntw2ks64.dll | c5f0d7dc8a219e3710347ce18b4e0643d11eb2f169bd485196d7b803b505e48b |
| ntwshk32.dll | b1d7996df4cc5b23eac4c568935e701d29e5125f61f712fe696a66a0d777a1dc |
| ntwshk64.dll | c0fe6bdd7747323d01cbcfcfa7f320f6f80a4111f0d382f3fb05156520e9aa34 |
| demo.exe | 9eb96b5f0ede04c12c9767763e282c7b96b23833db5cc376a79d44989cdc59c8 |

源码快照清单 SHA-256：`4e0a97353aed76bf955f9cbcc04945910069d37efe5c100a2c1c7ceaf474c1e9`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：9 个文本文件，新增 67 行、删除 43 行、净增 24 行，另有 8 个二进制文件；台账中的行变化包含新增文件后的顺序重排。字体基础头只保留自身接口，窗口字体声明由窗口头负责。
