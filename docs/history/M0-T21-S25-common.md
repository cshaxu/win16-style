# M0 T21 S25：Win16 共享基础与库拆分

用户确认验收通过，S25 收口；T21 保持开放，未准入 S26。四个内部静态库为 `win16-common`、`win16-window`、`win16-control` 和 `win16-dialog`。common 提供字体、调色板、几何及通用呈现；window 和 control 分别依赖 common，dialog 独立。`global_*` 保留在 DLL，公开 ABI、原始图形资源与来源声明不变。

产品提交 `c57c612` 的干净包 `s25-common-final-clean13` 以 GCC、Ninja 和 ccache 完成 x86/x64 构建及 ABI、来源、资源和四文件门禁。实际包 `s25-common-final-focused13` 的 14 项聚焦用例通过、无跳过；范围并非全部对话框或控件组合。四个同批产物发布至 `assets/` 与 `O:/winnt`，部署摘要和远端四个 Git blob 已逐项复核。

测试结构校正提交 `4733862` 增加 common、window 各自的链接适配器，`s25-test-layout-p2b` 双位宽各 8/8 通过。现有测试地图按源文件族映射到适配器，并非每个源文件都有单独测试文件；此次结果不证明完整行为矩阵。

用户验收时曾观察到系统菜单移动后消失，随后确认现象已恢复；未确定原因，也未修改产品。自有私有桌面真实鼠标探针通过普通 run16s 启动的 x86/x64 测试窗口的菜单打开、移动后保持和自定义命令点选；不代表所有应用或全局模式均已覆盖。

四文件 SHA-256：

- `run16s.exe`：`317ccf6422542480073d9126b4dfa6ef07695a77744a1ce1d76769f8825f3d21`
- `ntw16s32.dll`：`483cb5ca35fe67ab298dce11ad0cc354a34366fb3a608c6ba1b7d011c9fb3651`
- `ntw16s64.dll`：`2cb0b78e828fdfda4fdad42ffb4e3782ac4f268573cc3205cf9544b1056bc641`
- `demo.exe`：`911cb4c700dd46edceb080564c8bf281fb40c6942b2a2369fe92d1b188dc9435`
