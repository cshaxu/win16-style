# T26 S1 第二十批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `549b9c5bbdf5871194a1dcb6912ebafce306381a2fda9e3278a9de4ae091e8c3`。源码、资源、治理及两位宽公共归档边界检查通过。先发布至 `assets` 与 `O:\winnt`，以下八文件与同批包摘要一致。

本批将列表和编辑控件的原生滚动消息限制、VCL 微调控件身份识别移入公共库，Win16 直接使用，旧私有实现已删除。发布后双位宽同快照公共状态与独立身份测试、Win16 滚动几何测试通过；`O:\winnt\run16s.exe` 驱动的自建 VCL 微调、编辑框及列表滚动模型均通过。此为原生消息和自建窗口测试，并不代表真实 Delphi 应用、物理鼠标或 Win2000 控件已验证。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等失败和未覆盖范围仍保持开放。S1 全面审计和 T26 尚未收口。

提交差异统计（`git diff --cached --numstat`，不含本记录）：21 个文本文件，新增 139 行、删除 82 行、净增 57 行，另有 8 个二进制产物。VCL 控件识别和原生滚动协议由公共库负责；Win16 保留绘制、布局及附着策略，旧私有识别与协议实现已删除。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | ccfd1409c02faf561c62c2d72850ef1118c9724f2c70f2e82543ed2e7abe4253 |
| ntw16s32.dll | f2474bd4218a0dcce6ed412ada6031c8923dee79c6e4dffc517db011a082aea5 |
| ntw16s64.dll | 31104f80567040393a5b362d351312edcb3c11a9e11adc50637fb8460207d541 |
| ntw2ks32.dll | 72fe08e433ad53b1889992ea0b5fc96cf7168984207f18394eb640696e7cdbe4 |
| ntw2ks64.dll | e0fde5484fd276d4b6efb35f95f11da3d09df3ed962d554b1c7e5e19af893d8a |
| ntwshk32.dll | 51117f316a8d3881ce9e77c44aa0169632de86b330ed6fafbcfb1786a37901c7 |
| ntwshk64.dll | 0b763498428a797220372611713c55ee1e6c33979cfac6bd535461911e689dad |
| demo.exe | 6d84fd0da47f7e8e6cce60775de38ce54b138c933027c9172342e385fe3eef8a |
