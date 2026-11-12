# T26 S1 第十八批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `9dd1a2eaed39a600682ccd069b82c03939e36f4ef077c2de5a412a0aa448a57f`。源码、资源、治理及两位宽公共归档边界检查通过。先发布到 `assets` 和 `O:\winnt`；下列八文件与同批包摘要一致。

本批把滚动条按键到原生滚动命令的映射及现代窗口表面子类的引用、分发和销毁生命周期移入公共组件，Win16 实际调用。发布后同快照 x86/x64 公共滚动状态、UWP 宿主、安装失败和隐藏外框回归通过；正式 `O:\winnt\run16s.exe` 驱动自建滚动条模型在两种位宽通过键盘命令、鼠标、重入移除及清理。UWP 宿主测试使用同快照构建树，并非实际可见 UWP/WinUI 应用或发布 DLL 的跨进程测试；Win2000 尚未接入这两项接口。

真实 UWP/WinUI 宿主、真实全局会话、线程钩子错误 126、Win2000 DWM 恢复及打印夹具限制不因本批改变。S1 全范围审计、提交、推送和远端复核尚未完成；测试未创建或切换隔离桌面。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 2fc867e7e8b206930e4f5be516de21509bc83ceeb40fe0fa7c1deffd87c41041 |
| ntw16s32.dll | f3de8eba47a1e3ed89cdf40d398685df395959034079dfc84d9ddcc54f65064f |
| ntw16s64.dll | 8d89844b2fdbfde24e2e947eae1e87bbe1461473b4a413f4fdee9f6ea1d8fb66 |
| ntw2ks32.dll | ed449db1e47e4bb06aacd1831c1038d8c7a69ff698dc8a3c9d30bf6aa98ecbda |
| ntw2ks64.dll | 599b0e3ddd3268857832a5df9bb828befae3b432ea3de9887655a47ffd3f79dd |
| ntwshk32.dll | 3348e460719bc365907dac8b147b91042bb647413d297c7708554135cd743e47 |
| ntwshk64.dll | 69672c9a49bcfa22e5292521aa68699755173ccbb0fcad443a970e82c5574645 |
| demo.exe | 6561339811fbfc130afc55ea85044abd4917b054aac95accd3155218b0b4cd2f |
