# M0 T21 S26：提取现有进程挂钩能力

产品提交 `22b2449` 提取到 `src/lib-win32/process-hook`：挂起目标的导入表 staging、跨位宽辅助交接、MinHook 启动事务及现有晚期 guard/patch。载荷解释、目标 DLL 选择及产品行为仍由调用方负责。未新增已安装钩子的物理移除，也未添加隔壁所需的纯载荷模式；跨宽交接仍使用原有 ordinal 1 协议。全局业务未纳入此项，隔壁仓库只读，未证明可直接移除 Detours。详见[库方案](../proposals/process-hook-library.md)。

干净包 `s26-clean3` 双宽编译、ABI、来源和原图资源核验通过；`s26-package4` 实际包聚焦 15/15、无跳过，真实交互 cmd 四条双宽传播边通过。该干净树中独立库链接与晚期 guard/patch/batch 双宽各 4/4 通过。未运行完整 UI 矩阵，也未在隔壁构建。首次实际包探针读错同名私有符号；其后两次遇到自有短命 helper 退出时读取错误 299；修正探针后最终包通过，失败记录保留在 scratch。

`s26-publish1` 同批四文件发布到 assets 根及 O:/winnt，SHA-256 均与测试包一致。远端 main 指向产品提交，四个 assets Git 对象及代表源码已通过 GitHub API 逐项核对。四文件 SHA-256：

- `run16s.exe`：`a8115231a1a053b1ca3197346d52779bc7ecd7f8fea0cadbc4470beda46440dc`
- `ntw16s32.dll`：`1162fc61997e9b3f38c8bf748e43329cd37472848a65c6d532839285e2c51814`
- `ntw16s64.dll`：`b0169a141d69ddfc86114e7f87d2bc15330606956982834ed6ab84d6c9a2d044`
- `demo.exe`：`3782f3a85da52d57daeed15911d939ac0883fc6b20cdb23dbd3842ae6237fc09`
