# T26 S1 第二十一批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `6f1bf975f44f68b6614befed286e0f7eb51f993c691ce240d539c1a8d8fc15ed`。源码、资源、治理及两位宽公共归档边界检查通过。先发布至 `assets` 与 `O:\winnt`，以下八文件与同批包摘要一致。

本批将现代窗口表面输入事务移入公共 UWP 宿主，Win16 提供命中与动作。发布后双位宽同快照公共输入、宿主、安装回滚、Win16 隐藏外框和普通外框测试通过；正式 `O:\winnt\run16s.exe` 驱动的自建列表控件双轴滚动通过。公共宿主和外框测试使用构建树，不代表发布 DLL 的真实可见 UWP/WinUI 宿主或物理鼠标已验证。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等失败和未覆盖范围仍保持开放。S1 全面审计和 T26 尚未收口。

提交差异统计（`git diff --cached --numstat`，不含本记录）：10 个文本文件，新增 183 行、删除 60 行、净增 123 行，另有 8 个二进制产物。公共输入层负责捕获、状态和转发；Win16 保有布局及动作策略，原私有输入分派已删除。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 600601f5126d6e243b1fab294e3d20193d5ed2c8eb03bcb02d680682ae539045 |
| ntw16s32.dll | 1b7d51f1db2bda13bdec1e32b2e279f8e532e91c7c2b3f4d101d386dd3a0a07d |
| ntw16s64.dll | c534a856935139dd0c642b0e59386ccf5cf5c7695df485ab6b6972387be97568 |
| ntw2ks32.dll | 76919b15a9987a063a9eabf9305af43a6be217e115226db03b3889807dccd34c |
| ntw2ks64.dll | 9a1645955104b8145fad5f99f94a814a892d85d07680513caa5e039955cdd746 |
| ntwshk32.dll | 1980297048852f7f21ec937673a7014caecc1a0aa6bca48ff57a33c7d31ffe88 |
| ntwshk64.dll | 6ec4763889ab0a6ad37d693bf0d6b98e2f075f7a33cf3fffb9ddb2434d22cbaa |
| demo.exe | 3913a73091c4ae09129fe5b0a581d4985fc2c062dd049347bb48a3eec4b6dbf3 |
