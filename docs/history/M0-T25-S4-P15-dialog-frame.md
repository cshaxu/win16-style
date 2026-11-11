# M0 T25 S4 P15：对话框窗口外框接入

通过对话框创建调用期间的线程消息观察，在初始化返回后把符合普通顶层窗口条件的对话框交给现有 Win2000 外框附着器；覆盖 ANSI／Unicode 和资源名／内存模板的模态、非模态入口。观察钩子只在创建调用期间存在，正常返回及异常展开时清理，并保留调用方错误状态。不改变对话框内部控件的样式范围。

源码快照清单 SHA-256 `014b42d00829e5774eca86c20781466cf77f9fe3e4ccd184c2523de81dd2949e`；纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建。同批包 60 项重点测试通过、无跳过；新增自建测试验证 ANSI／Unicode 内存模板的模态和非模态窗口属性与标题高度，不证明资源名路径、实际绘制或异常情况已覆盖。先前增量探针观察到旧版录音机窗口附着且菜单打开，但该探针结束时未能正常等待进程退出并终止了自建实例，因此整个应用测试为失败，不能计作通过；未做正式包真实应用回归。旧版计算器已加载 DLL 但主窗口未附着，原因未确定，仍需修复。详见[窗口审计](M0-T25-S4-window-audit.md)。

发布至项目 assets 根目录与 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | b60327c7590f422402af51b41b7c392fbeeea0e4d6a56fe0dd490eb8b09c05cc |
| ntw16s32.dll | ba3c19abd3b5f225d0ea368840e2d539062112df3cdc2c9567246d5e9c286934 |
| ntw16s64.dll | 7e0c21f0a29cc3061888d013a9b3d282d75b079239893739e7a1cc1b170b5add |
| ntw2ks32.dll | 457bac60dc8724c8bd8e26c3356d0b3cff98f7c5421639d8c911c39f11418da9 |
| ntw2ks64.dll | 83577394fc8d9a0e3581a982ed7ceefe9ce3a942ffe714212df8f5cedbf21938 |
| ntwshk32.dll | 0024d4b31db2da863a5c92390fd2f2bb01202f8b58ab2bd8ddd42a266abb5b6c |
| ntwshk64.dll | f8d7ff52d0e7ebfa4d6ce9bc1dc5eaa5c3130b7e703bdf0066061aa663ca2bae |
| run16s.exe | 62f0a5f7eab9bd3bf0111e194b0598b2e7773d217f9e9a20013e75b65201fd4b |

未完成完整集成矩阵、全部对话框创建路径和人工视觉验收；S4、T25 继续开放。
