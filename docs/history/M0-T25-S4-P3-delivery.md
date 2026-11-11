# M0 T25 S4 P3：修复重复标题图标

用户测试 Solitaire 发现标题栏有两个图标。P2 错误地把系统菜单区域之外的 `rcTitleBar` 当作包含图标的区域，重复绘制图标；原因、原版源码定位与增量探针见 [审计](M0-T25-S4-window-audit.md)。P3 撤除额外图标及额外预留空间，继续使用原生系统菜单图标及命中。

源码快照 SHA-256 `85f25f693d260338c9641474b917078d7590b82e5002826ab99a66bfd9cd8573` 使用纯 C17、MinGW GCC 16.1.0、Ninja、ccache 4.13.6 双位宽干净构建。正式包在私有桌面 34 项重点测试通过、无跳过；包括两位宽的原生图标区变化和标题文字区不跟随图标变化。未测试物理输入或完整集成矩阵；同进程普通和工具窗口连续创建的先前桌面清理失败仍保留。用户尚未验收此修复，也进一步报告配色问题。

同批八文件发布至 `assets` 根目录和 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 164968f59bced7a931157d3444c78f0f246b1712b93ca3e6b2f388439d2de094 |
| ntw16s32.dll | 9fab19e5b1d24e39b3367b2f5685eb0d78b1960f67ee212d4790b726edee7b4b |
| ntw16s64.dll | b365d18a71296091cf72ca389f02a664d515dbc9c2046eb4bb53a42a97d84abf |
| ntw2ks32.dll | 6db5313ba8852d458ce38c4a162e3ae962333c5e8f68af93f251842a949a7182 |
| ntw2ks64.dll | 2ba8c7ee230fcda607c234e283d518ef09a74ee7c6eb6e4b20e68e27205ad291 |
| ntwshk32.dll | dfdc1264cb31772c5a884b1995d842942670ff55db7344ebf63cf41e696e34f3 |
| ntwshk64.dll | e868e5f1d895046eb5eb031e6b0926dda17f67f61b5ab176032be223dad23199 |
| run16s.exe | b42e2413c2ccfbca502d45ced710f3c229637e39b0aa2e40c70566de3ed95656 |

S4、T25 保持开放；需处理用户反馈的颜色和原版窗口几何与交互。
