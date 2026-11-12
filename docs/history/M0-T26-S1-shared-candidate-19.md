# T26 S1 第十九批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `04b908d28094672f8f8dcdf7494b40bbc6ff89e0f366692be5fb12a8bab7f24f`。源码、资源、治理及两位宽公共归档边界检查通过。先发布至 `assets` 与 `O:\winnt`，以下八文件与同批包摘要一致。

本批与第十八批功能相同，修正三个公共源码末尾空行并重新构建。发布后双位宽同快照公共 UWP 宿主、安装回滚、隐藏外框及滚动状态测试通过，正式 `O:\winnt\run16s.exe` 驱动的滚动控件模型双位宽通过。UWP 测试使用构建树静态链接，并未验证真实可见 UWP/WinUI 宿主或发布 DLL 的跨进程运行。

此前真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等失败和未覆盖范围仍然开放。S1 全面审计与收口尚未完成。

交付差异统计（`git diff --cached --numstat`，不含本记录）：300 个文本文件，新增 7017 行、删除 2682 行、净增 4335 行；另有 8 个二进制产物。公共宿主持有外框表面子类生命周期，Win16 保有绘制、命中及菜单策略；旧 Win16 表面子类分发路径已删除。其余公共迁移、实际调用方与保留的风格状态以各专项审计为准，未审范围不计完成。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | fb5a08050255afe1cfef72d447e3bc5b4ac337ac6a6cbeee7169a3c87e8a85e9 |
| ntw16s32.dll | 2afa72cc805ccd12969002be6c7154881351e45d7a2e5d0e184f5ac994f01ce8 |
| ntw16s64.dll | 23d8304596f23eecea9ee601eea961a3d80aed572032832971eae35b7c3a4abb |
| ntw2ks32.dll | aa93809b9587bd3a31a3416cc930f45a50bdf0aaeca36e2ee99c7e87f2a1e379 |
| ntw2ks64.dll | 5509e47b87533fc5e5dbed56889976247c1637ce915100ef0018d24b2307ffd5 |
| ntwshk32.dll | 5340d2d27eaeaadca728c38c75e30d49deb8b9d2732ca2123625ec151f8e3e6f |
| ntwshk64.dll | 20235298f8673a329a1408e98ed679f11aad197caa089debd4efae47a35682b7 |
| demo.exe | 50df7a95f1f56e94ab2a6045952b155cf540e152a7c3bf7b45234337244178fe |
