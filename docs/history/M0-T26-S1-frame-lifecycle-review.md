# T26 S1 普通外框生命周期局部审计

范围：`win16-window/frame_lifecycle.c`、`frame_layout.c`、`frame_message.c`、`frame_dispatch.c` 与 `win2k-window/frame.c` 的安装、消息和移除路径，以及其实际调用的公共外框接口。它不覆盖所有外框绘制或现代宿主。

| 能力 | 已有归属及调用 | 待做或边界 |
| --- | --- | --- |
| 线程身份、DWM 观察与设置 | Win16 生命周期经服务回调验证线程，Win2000 直接调用 `win32_style_window_current_thread`；两者均调用 `style_dwm_capture` / `style_dwm_policy`。 | Win16 对子窗口不设置 DWM；Win2000 对失败设置进行回滚，且未知原策略时使用 `USEWINDOWSTYLE`。原策略不可完整观测；Win2000 实测恢复失败已在 CURRENT 记录。 |
| 位置和客户区恢复 | `style_client_screen`、`style_frame_changed_restore`、`style_restore_client` 已由两种风格使用；Win16 还有 XAML 工作区边界保护。 | 两者何时触发重算不同。Win16 对隐藏创建、取消及重入有状态检查；Win2000 通过已注册消息延迟重算，不能仅合并调用顺序。 |
| 子类安装、回滚和终止 | Win16 维护 `style_window`、字体、滚动条、菜单及重入引用；Win2000 使用窗口属性、活动菜单和两个子类过程，并拒绝在菜单或按钮活动时移除。 | 通用安装事务有待设计并在两个真实调用方接入，但必须保留各自失败、重入及移除契约；不能把 Win16 状态结构上移当成公共接口。 |
| 非客户区抑制与绘制 | 两者使用 `style_frame_suppress_native_theme`；Win16 使用 `style_frame_ncpaint` 和风格画笔，Win2000 使用 `style_frame_native_region` 保留原生非客户区服务，随后绘制。 | 绘制序列不同，特别是滚动条处理；不能直接替换，以免破坏现有功能。 |
| 客户区计算、命中和光标 | Win16 根据自身几何覆盖 `WM_NCCALCSIZE` 和命中，Win2000 修正原生几何并保留原生穿透/错误命中；部分命中与光标基础已有公共函数。 | 布局、按钮及滚动条几何属于风格策略；应审计尚未共享的消息事务而不是复制几何。 |
| 延迟显示与现代窗口 | Win16 跟踪隐藏创建及 XAML 表面首次可用，复用公共 XAML 桥和重绘调度；Win2000 对无标题窗口等待原生样式变化，并用消息延迟重算。 | 两种等待条件和状态所有者不同；Win2000 现代宿主支持尚未由此证明。 |

主要验证入口：`tests/ntw16s-dll/frames.c`、`pending_frame_owner.c` 和 `tests/lib-style/win2k-window/frame_lifecycle.c`。前者的自建外框回归及后者的 DWM 恢复限制见 CURRENT 链接；测试文件存在不等于全部生命周期组合通过。后续抽离前应补齐双风格的重入销毁、子类安装失败、取消、活动菜单移除和原生错误值契约，再验证实际产品调用与旧路径删除。
