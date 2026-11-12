# T26 S1 第十四批共享候选

双位宽 MinGW、Ninja 和 ccache 干净构建通过，源码快照清单 SHA-256 为 `56a4599bddb7bbb453ff1b836f188440aac894aa3654395390d821cebf349623`。源码、资源来源、治理和双位宽公共归档边界检查通过。先发布至 `assets` 和 `O:\winnt`，以下八文件与同批包摘要一致。

本批将现代宿主所有者子类的安装及通用消息分派移入 `lib-style/uwp/host`，Win16 提供菜单与激活策略；详情见[现代外框审计](M0-T26-S1-modern-frame-audit.md)。发布后使用同快照静态链接的自建测试，x86/x64 均通过安装失败及回滚、所有者与标题消息重入、延迟释放，以及 Win16 隐藏外框的重复安装、移除和窗口销毁。测试未创建或切换隔离桌面。这些是同快照代码测试，不是通过发布 DLL 的真实应用测试；可见 UWP/WinUI 宿主、表面呈现及此前错误 126 的真实钩子均未通过验证。

第十三批记录的 VB 文件控件完整产品夹具错误 126、Win2000 DWM 恢复和打印夹具限制不因本批改变。S1 完整审计、提交、推送与远端复核尚未完成。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 0c69cfce123f4bf348300defaa9ea8910a2b92a8384eab931acba605b1242337 |
| ntw16s32.dll | 240def5a2353ba88d4b2d11f3748a0f48c124519cc63482989922c9c4f4aa6ab |
| ntw16s64.dll | e4094ef4d6c125d6b1dc8f69ea13d816afdbd00e8b44b7913a44bff7c84cb046 |
| ntw2ks32.dll | a853887bd392b593e18fa06e8aa3ac9b2a389a3acb31755cad9dc34a5e7bdc62 |
| ntw2ks64.dll | fbf49924498fdd5b249e7291a7419872adb1710eced9edd802b90efa2e50c161 |
| ntwshk32.dll | b12863b038dd0e8986124dc33700bfb28bb842f6e967edda993ea3b571d025f3 |
| ntwshk64.dll | 8eb83a3b7a14f8e2bf540a36867276e4ca47a16cd4e15e3fed17e381c9717c88 |
| demo.exe | 141a901bee1eeef2c5139fc28cfdd76a73bb9c93b2a52c92ba0b093839e5d046 |
