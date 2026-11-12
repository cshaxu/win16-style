# T26 S1 第十六批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `33ecf47af592deeb609578a6a5e394f7c92f0f5386259c671bcf0a5a7f308766`。源码、资源、治理及两位宽公共归档边界检查通过。先发布至 `assets` 和 `O:\winnt`，以下八文件与同批包摘要一致。

本批把状态栏标准消息失效判定和原生分栏、文本快照采集移入公共组件，Win16 实际调用；调用方拥有缓存、存储与身份策略。发布后同快照 x86/x64 的 v5/v6 状态栏公共测试和 Win16 控件独立链接测试通过；正式 `O:\winnt\run16s.exe` 驱动自建 VCL 状态栏模型在两位宽通过绘制、裁剪、重入移除、查询期间销毁和清理。Win2000 尚未接入该接口，不能声称其状态栏外观已实现。测试未创建或切换隔离桌面。

真实 UWP/WinUI 宿主、真实全局会话、线程钩子错误 126、Win2000 DWM 恢复及打印夹具限制不因本批改变。S1 全范围审计、提交、推送和远端复核尚未完成。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | da114107f602fd77e7591b936ae7557c655d5ca8033fbe6fe9a979ceb67b2069 |
| ntw16s32.dll | a4bbcf4ac57f67c8492fa1b81e43b9f8557929e8e7b17be5a6631562416483f4 |
| ntw16s64.dll | ad6bb487425529f021ad407d43d88eb95310558a3e858c966003dfa10b40674e |
| ntw2ks32.dll | c88d82ca6e11ff93cfa200a65b75f47a97164c5dec6bcad9adf8a8ef2738f033 |
| ntw2ks64.dll | 6934522ab1f3b792dc1b7f8188262a0770cd5e9ecbc1474f4cd8e50bac0558d4 |
| ntwshk32.dll | 56fbcb9102a8aca8784f119683323a0a6fcff1e88ae4d10d8d69159f1b241af1 |
| ntwshk64.dll | 353a020254d99fbdcb6521b372816f9a12823ec6a4d151102c427946d5db633f |
| demo.exe | 4864d345ba93d008b0132019f1ea9ec082840608918138dd5539feeb221a2f83 |
