# T26 S1 当前覆盖台账

基于 P44 后的工作区盘点，统计 `.c`、`.h`、`.def`；它是待审全集和现有证据索引，不等于逐文件、逐函数审计完成。初始路径基线见[冻结清单](M0-T26-S1-initial-inventory.md)。目录数可能随后续迁移变化，应在收口前重新枚举、逐项对账。

| 生产范围 | 文件数 | 已有证据与未完成工作 |
| --- | ---: | --- |
| `lib-style/common` | 92 | 对话框包装、窗口切换、控件协议、菜单与呈现已有实际消费者；逐接口核对两风格调用、所有权及反向依赖仍未完成。 |
| `lib-style/uwp` | 19 | 宿主安装、所有者/标题消息、表面刷新和目标分发部分共享；真实可见宿主及跨进程钩子未通过验证，见[现代外框审计](M0-T26-S1-modern-frame-audit.md)。 |
| `lib-style/win16-common` | 7 | 七个文件的风格资源、配色、布局和产品 ABI 已分类；公共位图已复用，高对比度策略的未来通用边界见[逐项复核](M0-T26-S1-win16-common-review.md)。 |
| `lib-style/win16-control` | 106 | 类身份、状态栏快照、非客户区滚动条分派等已部分抽离；绘制、消息、框架识别、附着和字体生命周期仍须逐项分类。 |
| `lib-style/win16-window` | 33 | 外框和菜单部分机制已共享；安装事务、菜单嵌套和消息分派边界仍需逐项验证，见[外框](M0-T26-S1-frame-lifecycle-review.md)和[菜单](M0-T26-S1-menu-scope-review.md)局部审计。 |
| `lib-style/win16-window-uwp` | 4 | 宿主通用安装和所有者/标题分派已抽离；表面输入、菜单动作和风格绘制仍需核对可复用边界。 |
| `lib-style/win2k-window` | 6 | 外框及菜单使用部分公共机制；DWM 恢复失败、风格差异和 Win2000 消费范围仍需验证。 |
| `lib-win32/style` | 10 | 启动、窗口身份、分发及伴随模块部分通用；须对照每个产品调用、头依赖及旧路径逐一检查。 |
| `ntw16s-dll` 根目录 / `global` / `injection` / `uwp` | 20 / 23 / 10 / 4 | 部分接入已调用公共库；会话协议、具体 DLL 导出及风格策略留产品侧，未审协调不能笼统归为业务，见[接入复核](M0-T26-S1-product-adapter-review.md)和[对话框复核](M0-T26-S1-dialog-current-review.md)。 |
| `ntw2ks-dll` / `ntwshk-dll` / `run16s-exe` | 6 / 9 / 19 | 已有共享启动及主题状态；须完成所有入口、链接、普通和全局模式的实际调用和恢复边界核对。 |

对应测试全集：`tests/lib-style` 66、`tests/lib-win32/style` 8、`tests/ntw16s-dll` 222、`tests/ntw2ks-dll` 7、`tests/integration` 49。测试路径只是覆盖候选，不代表每个生产入口都有测试；还须逐项映射 CMake 注册、可运行条件、双位宽和实际结果。

以下是按实际调用核对的公共交集，而不是仅按头文件名称推断：

| 能力 | Win16 调用位置 | Win2000 调用位置 | 边界 |
| --- | --- | --- | --- |
| DWM 状态与客户区恢复 | `win16-window/frame_lifecycle.c`、`frame_layout.c` | `win2k-window/frame.c` | 共用捕获和恢复辅助函数；Win2000 DWM 有已记录的恢复失败，安装与回滚事务尚未统一。 |
| 菜单指针、键盘和助记键 | `win16-window/menu_bar.c` | `win2k-window/frame.c` | 共用 `menu_input.h`；Win16 的系统菜单项及 Win2000 原生菜单几何仍是各自策略。 |
| 临时绘制缓冲 | `win16-window/frame_view.c`、`menu_scope.c`、`win16-control/scroll_paint.c` | `win2k-window/system_menu.c` | 两风格共用 `present.c`；滚动条是否预拷贝由调用方决定，绘制和配色不共享。 |
| 系统主题压制 | `ntw16s-dll/attachment.c`、`theme.c` | `ntw2ks-dll/controls.c` | 共用 `theme_state` 的捕获和压制；具体控件准入及拥有者由产品负责，原主题只能尽力恢复。 |
| 普通对话框和消息框调用 | `ntw16s-dll/injection/window_titles.c` | `ntw2ks-dll/controls.c` | 共用 `STYLE_DIALOG_HOOK`；Win16 创建作用域和 Win2000 观察器分别提供开始、清理及失败策略。 |
| 现代窗口宿主与输入 | `win16-window-uwp/frame.c` | 尚无消费者 | `uwp/host`、`input`、`surface` 是可配置的公共机制；隐藏自建窗口验证不证明其他风格或真实宿主已支持。 |
| 列表/编辑滚动协议及 VCL 微调身份 | `win16-control/scroll_native.c`、`spin_scope.c` 和 `ntw16s-dll/attachment.c` | 尚无消费者 | 公共识别和协议可供其他风格使用；Win2000 滚动条和对应控件渲染尚未实现。 |

建议按依赖推进：先收齐以上未审入口及测试映射；再完成共用窗口生命周期与菜单、控件及对话框事务、现代宿主表面和产品适配；最后做两风格实际调用、反向依赖、旧路径、发布包和远端复核。先前用户决定并入的 T25 S9 去重包含在这些单元中，不另分配任务。当前只有 S1 准入；以上次序不表示后续 S 已准入或工作已完成。
