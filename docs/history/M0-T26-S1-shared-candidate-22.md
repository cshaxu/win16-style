# T26 S1 第二十二批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `3e3b2fa9e6e5ec286c2d2cbcaada8962d166d2f1a392e089344ba217febef480`。源码、资源、治理及两位宽公共归档边界检查通过。先发布至 `assets` 与 `O:\winnt`，以下八文件与同批包摘要一致。

本批让 Win16 滚动条复用两种风格使用的公共位图呈现缓冲，支持由调用者决定是否预拷贝目标；既有菜单和外框调用保持预拷贝。发布后双位宽公共缓冲及故障测试、自建控件非零坐标逐像素和失败清理测试通过；正式 `O:\winnt\run16s.exe` 驱动的 VCL 滚动模型及 Win2000 系统菜单绘制采样通过。逐像素测试使用同快照构建树，不代替所有 DPI、真实应用及物理鼠标验收。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等失败和未覆盖范围仍保持开放。Win2000 滚动条尚未实现，S1 全面审计和 T26 尚未收口。

提交差异统计（`git diff --cached --numstat`，不含本记录）：12 个文本文件，新增 80 行、删除 37 行、净增 43 行，另有 8 个二进制产物。公共呈现组件持有临时缓冲及提交，Win16 滚动条保留图案相位和绘制策略，旧私有缓冲实现已删除。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | df3209004b3cdc7330cdba2db1730f5c96ee011ee78179f7d65dff7703f6b7d2 |
| ntw16s32.dll | 5fac57835ff8acba6a43c3427035b3bc5132114e7d9a8b7d8a3b0cbef9489d38 |
| ntw16s64.dll | 7188da58eba0450fbd2bb1a38be3cc83f27a2721765984ffe9da8e0da4cef409 |
| ntw2ks32.dll | 320a1d058aefe87e7bc37b82b826cb86d8633eab36b2dfe824747306141a6943 |
| ntw2ks64.dll | 0109eac7ded3a644209eaa746f6da002e7e6ee50be788da2ec5c663f48908a33 |
| ntwshk32.dll | 7fb27d1ecc773e0c9b6ce70607c0bada6f7eca15b6ff2b5d110a8cc6d0385a1b |
| ntwshk64.dll | db0c24980d771c75240638a86be2c2b9f616f263ce4c8bb519cde23b85b799d5 |
| demo.exe | 9077359653da07226a7f0e280085510222d7d14c12511648e8ef4fdbd77abcc5 |
