# M0 T29 S2 主题迁移与 ABI 冻结

迁移分母沿用 [S1 台账](M0-T29-S1-msstyles-audit.md)的原编号和列值。相同一行的两个适用年代共用下列验收口径，分别保存结果。`—` 单元不参与计数。优先覆盖标准字段，主题生成和发布验收见[构建契约](../proposals/msstyles-build-contract.md)。

## 113 单元的接收者和验收

| S1 行 | 阶段 | 接收组件 | 可观察验收项 |
| --- | --- | --- | --- |
| 01 | S5 | appearance / themes | 色槽和局部画刷 RGBA 对照 |
| 02 | S4 | appearance / render | 活动与非活动标题颜色、渐变端点 |
| 03 | S5 | appearance / render | 字体 LOGFONT、标题文字色和基线 |
| 04 | S5 | application / graphics | 菜单与控件字体选择及度量 |
| 05 | S5 | application / graphics | 保留来源的字形字节、advance 及采样 |
| 06 | S5 | appearance / render | 普通与最大化边缘像素 |
| 07 | S4 | application / window | 96/144 DPI 标题栏和边框几何 |
| 08 | S4 | appearance / render | 标题按钮状态和位图来源 |
| 09 | S4 | appearance / render | 系统按钮图形及其有无 |
| 10 | S4 | application / window | 标题按钮排列、间隙和命中矩形 |
| 11 | S5 | application / window | 最大化、无图标及不可调整组合 |
| 12 | S5 | appearance / render | 菜单普通、热点及禁用绘制 |
| 13 | S5 | appearance / render | 勾选、分隔与子菜单原始栅格 |
| 14 | S5 | application / menu | 菜单三列、字体测量及极值 |
| 15 | S4 | appearance / render | 普通、默认、按下及禁用像素 |
| 16 | S6 | appearance / render | 复选三态、单选和禁用像素 |
| 17 | S6 | appearance / render | 静态字色与蚀刻边缘 |
| 18 | S6 | appearance / render | 分组框标题与边线 |
| 19 | S6 | appearance / render | 编辑框普通、只读与禁用 |
| 20 | S6 | appearance / render | 列表背景、选中和边缘 |
| 21 | S6 | appearance / render | 组合框下拉、禁用和边线 |
| 22 | S4 | appearance / render | 横纵箭头、滑块、轨迹及尺寸框 |
| 23 | S4 | application / control | 标准滚动条厚度和端钮矩形 |
| 24 | S4 | application / control | 短条与特定厚度、绘制和命中一致 |
| 25 | S6 | appearance / render | 标签页普通、选中和边缘 |
| 26 | S6 | appearance / render | 表头普通和按压状态 |
| 27 | S6 | appearance / render | 列表视图及树视图已支持状态 |
| 28 | S6 | appearance / render | 进度条槽、段和填写范围 |
| 29 | S6 | appearance / render | 滑杆通道、滑块与刻度 |
| 30 | S6 | appearance / render | 微调按钮各态与方向 |
| 31 | S6 | appearance / render | 工具栏普通按钮与分隔 |
| 32 | S6 | appearance / render | 状态栏分隔和尺寸柄 |
| 33 | S6 | appearance / render | Rebar 已支持抓手 |
| 34 | S6 | appearance / render | 日期控件边线与下拉 |
| 35 | S6 | appearance / render | 月历标题、日期及高亮 |
| 36 | S6 | appearance / render | RichEdit 和热键控件边缘 |
| 37 | S7 | application / adapters | 两年代 VB/VCL 私有表面以原实机样本与同角色模型分别对照 |
| 38 | S8 | appearance / render | 公共框及标准子控件像素 |
| 39 | S8 | application / dialog | 文件／目录模板角色与实际控件矩形 |
| 40 | S8 | application / dialog | Win2000 General 三种页面选择和几何 |
| 41 | S8 | application / dialog | 展开色板、经典查找框和输入保留 |
| 42 | S8 | appearance / render | 消息框图标、按钮边距和文字 |
| 43 | S9 | application / adapters | 两年代 Win32 与现代宿主同 DPI 的最终像素样本 |
| 44 | S10 | application / lifecycle | 附着撤销、原主题及资源恢复 |
| 45 | S4 | application / window | 客户端尺寸、边界与命中区域 |
| 46 | S9 | application / adapters | UWP/WinUI 坐标、焦点、重排和恢复 |
| 47 | S5 | application / menu | Esc、失焦、重新进入及通知归属 |
| 48 | S6 | application / control | 真值、重入、命令及回退 |
| 49 | S4 | application / control | 双向拖拽、捕获、范围及通知 |
| 50 | S5 | application / graphics | 原应用字体归还和引用释放 |
| 51 | S8 | application / dialog | A/W 文件选择、缓冲区及错误 |
| 52 | S8 | application / dialog | Shell/COM PIDL、代理对象、选中及清理 |
| 53 | S8 | application / dialog | 查找通知与字体颜色 callback |
| 54 | S8 | application / dialog | Print/Apply、回退、设备和句柄 |
| 55 | S8 | application / dialog | 按钮命令、帮助及准确返回值 |
| 56 | S7 | application / adapters | 已支持 VB/VCL 角色及应用自绘退路 |
| 57 | S10 | access + 产品 | 正式入口和自动／手动选择、撤销 |
| 58 | S10 | 产品装配 | 两位宽实际入口和兼容 ABI |

待证的两组共四个按年代计的单元。第 37 行在 S7 分别使用原有 VB/VCL 自有测试实例与已冻结模型，标明哪个结论有真实宿主证据。第 43 行在 S9 执行同 DPI Win32/UWP/WinUI 样本的最终像素对照，窗口矩形和合成层尺寸单独记录。

## ABI 基线及旧导出的处置

从 `.def` 取两位宽公共名称和序号，再用 `tools/check_core_binary.py` 的 PE 解析函数直接读两处正式发布。四个 DLL 在两个发布根逐件相同。Win16 每宽序号 1–39，Win2000 每宽序号 1–5。两者各只有序号 1 是 NONAME，公开有名导出每宽分别是 38 和 4，实际 PE 无多出的装饰名。x86 存在内部调用约定装饰，x64 有普通名或内部别名，公共名和序号一致。

下面的源码列是生产实现、声明或调用的代表位置。代码仅有定义或声明时，对外二进制消费者仍属未知。测试位置单列。对于动态按名称查找的入口及序号调用，切换时先迁实际消费者再检查导出。发布旧 DLL 被删除后，按旧模块名绑定的外部二进制需要继续使用旧包，或经 S10/S11 明确认可的兼容安排。目标包在无旧模块的基础测试通过前，保持两种已发布实现都可从 P45 整包恢复。

| 年代 | 序号 | 公共符号 | 生产代码证据 | 测试证据 | 迁移处置 |
| --- | ---: | --- | --- | --- | --- |
| Win16 | 1 | `Ntw16sFinishHelper`（NONAME） | [序号调用](../../src/lib-win32/process-hook/helper.c) [entry.c:7](../../src/ntw16s-dll/injection/entry.c#L7) | — | 保留同序号中性入口 |
| Win16 | 2 | `Ntw16sCreationBegin` | [attachment.c:481](../../src/ntw16s-dll/attachment.c#L481) [attachment.h:13](../../src/ntw16s-dll/attachment.h#L13) | [abi.c:7](../../tests/ntw16s-dll/abi.c#L7) [control_extensions.c:53](../../tests/ntw16s-dll/control_extensions.c#L53) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 3 | `Ntw16sCreationEnd` | [attachment.c:483](../../src/ntw16s-dll/attachment.c#L483) [attachment.h:14](../../src/ntw16s-dll/attachment.h#L14) | [abi.c:8](../../tests/ntw16s-dll/abi.c#L8) [control_extensions.c:54](../../tests/ntw16s-dll/control_extensions.c#L54) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 4 | `Ntw16sGetAbiVersion` | [api.h:9](../../src/ntw16s-dll/api.h#L9) [dllmain.c:4](../../src/ntw16s-dll/dllmain.c#L4) | [abi.c:9](../../tests/ntw16s-dll/abi.c#L9) [runtime.c:136](../../tests/ntw16s-dll/runtime.c#L136) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 5 | `Ntw16sScrollChanged` | [attachment.c:322](../../src/ntw16s-dll/attachment.c#L322) [attachment.h:24](../../src/ntw16s-dll/attachment.h#L24) [window_titles.c:303](../../src/ntw16s-dll/injection/window_titles.c#L303) | [abi.c:14](../../tests/ntw16s-dll/abi.c#L14) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 6 | `Ntw16sSetWindowTheme` | [attachment.h:28](../../src/ntw16s-dll/attachment.h#L28) [theme.c:5](../../src/ntw16s-dll/theme.c#L5) [theme.h:9](../../src/ntw16s-dll/theme.h#L9) | [theme_shared.c:20](../../tests/lib-win32-style/application/lifecycle/theme_shared.c#L20) [abi.c:16](../../tests/ntw16s-dll/abi.c#L16) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 7 | `Ntw16sWindowAttached` | [attachment.c:320](../../src/ntw16s-dll/attachment.c#L320) [attachment.h:16](../../src/ntw16s-dll/attachment.h#L16) [window.c:8](../../src/ntw16s-dll/global/window.c#L8) | [automatic_target.c:80](../../tests/integration/automatic_target.c#L80) [doublekiller_probe.c:62](../../tests/integration/doublekiller_probe.c#L62) | 迁共享窗口事务，保留契约 |
| Win16 | 8 | `Ntw16sWindowRemove` | [attachment.c:266](../../src/ntw16s-dll/attachment.c#L266) [attachment.h:17](../../src/ntw16s-dll/attachment.h#L17) [controls.c:193](../../src/ntw16s-dll/controls.c#L193) | [doublekiller_probe.c:44](../../tests/integration/doublekiller_probe.c#L44) [win16_dwm_restore.c:26](../../tests/lib-win32-style/application/window/win16_dwm_restore.c#L26) | 迁共享窗口事务，保留契约 |
| Win16 | 9 | `Ntw16sWindowTerminal` | [attachment.c:275](../../src/ntw16s-dll/attachment.c#L275) [attachment.h:21](../../src/ntw16s-dll/attachment.h#L21) | [abi.c:20](../../tests/ntw16s-dll/abi.c#L20) [ntw16s_terminal.c:82](../../tests/ntw16s-dll/ntw16s_terminal.c#L82) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 10 | `win16_style_control_install` | [control_scope.c:5](../../src/ntw16s-dll/control_scope.c#L5) [controls.c:202](../../src/ntw16s-dll/controls.c#L202) [controls.h:13](../../src/ntw16s-dll/controls.h#L13) | [abi.c:22](../../tests/ntw16s-dll/abi.c#L22) [dialog_exception_recovery.c:17](../../tests/ntw16s-dll/dialog_exception_recovery.c#L17) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 11 | `win16_style_control_remove` | [controls.c:193](../../src/ntw16s-dll/controls.c#L193) [controls.h:39](../../src/ntw16s-dll/controls.h#L39) | [abi.c:23](../../tests/ntw16s-dll/abi.c#L23) [font_layout.c:238](../../tests/ntw16s-dll/font_layout.c#L238) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 12 | `win16_style_controls_begin` | [control_scope.c:13](../../src/ntw16s-dll/control_scope.c#L13) [controls.h:74](../../src/ntw16s-dll/controls.h#L74) | [abi.c:24](../../tests/ntw16s-dll/abi.c#L24) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 13 | `win16_style_controls_end` | [control_scope.c:15](../../src/ntw16s-dll/control_scope.c#L15) [controls.h:75](../../src/ntw16s-dll/controls.h#L75) | [abi.c:25](../../tests/ntw16s-dll/abi.c#L25) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 14 | `win16_style_default_palette` | [palette.c:10](../../src/lib-win32-style/appearance/win3x/common/palette.c#L10) [palette.h:9](../../src/lib-win32-style/appearance/win3x/common/palette.h#L9) [combo_paint.c:76](../../src/lib-win32-style/appearance/win3x/control/combo_paint.c#L76) | [common_link.c:12](../../tests/lib-win32-style/appearance/win3x/common/common_link.c#L12) [palette.c:10](../../tests/lib-win32-style/appearance/win3x/common/palette.c#L10) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 15 | `win16_style_edit_install` | [control_scope.c:5](../../src/ntw16s-dll/control_scope.c#L5) [controls.c:283](../../src/ntw16s-dll/controls.c#L283) [controls.h:38](../../src/ntw16s-dll/controls.h#L38) | [abi.c:27](../../tests/ntw16s-dll/abi.c#L27) [font_layout.c:239](../../tests/ntw16s-dll/font_layout.c#L239) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 16 | `win16_style_edit_scrollbars_enable` | [attachment.c:285](../../src/ntw16s-dll/attachment.c#L285) [controls.c:285](../../src/ntw16s-dll/controls.c#L285) [controls.h:64](../../src/ntw16s-dll/controls.h#L64) | [abi.c:28](../../tests/ntw16s-dll/abi.c#L28) [scroll_frame_target.c:112](../../tests/ntw16s-dll/scroll_frame_target.c#L112) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 17 | `win16_style_font_acquire` | [font.c:73](../../src/lib-win32-style/appearance/win3x/common/font.c#L73) [font.h:8](../../src/lib-win32-style/appearance/win3x/common/font.h#L8) | [abi.c:29](../../tests/ntw16s-dll/abi.c#L29) [runtime.c:60](../../tests/ntw16s-dll/runtime.c#L60) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 18 | `win16_style_font_release` | [font.c:75](../../src/lib-win32-style/appearance/win3x/common/font.c#L75) [font.h:9](../../src/lib-win32-style/appearance/win3x/common/font.h#L9) | [abi.c:30](../../tests/ntw16s-dll/abi.c#L30) [runtime.c:68](../../tests/ntw16s-dll/runtime.c#L68) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 19 | `win16_style_frame_scrollbars_install` | [controls.c:233](../../src/ntw16s-dll/controls.c#L233) [controls.h:42](../../src/ntw16s-dll/controls.h#L42) | [abi.c:31](../../tests/ntw16s-dll/abi.c#L31) [scroll_frame_target.c:109](../../tests/ntw16s-dll/scroll_frame_target.c#L109) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 20 | `win16_style_frame_scrollbars_palette` | [controls.c:273](../../src/ntw16s-dll/controls.c#L273) [controls.h:55](../../src/ntw16s-dll/controls.h#L55) | [abi.c:32](../../tests/ntw16s-dll/abi.c#L32) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 21 | `win16_style_frame_scrollbars_remove` | [controls.c:271](../../src/ntw16s-dll/controls.c#L271) [controls.h:54](../../src/ntw16s-dll/controls.h#L54) | [abi.c:33](../../tests/ntw16s-dll/abi.c#L33) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 22 | `win16_style_get_scroll_info` | [controls.c:277](../../src/ntw16s-dll/controls.c#L277) [window_titles.c:308](../../src/ntw16s-dll/injection/window_titles.c#L308) [win16_style.h:34](../../src/ntw16s-dll/win16_style.h#L34) | [abi.c:34](../../tests/ntw16s-dll/abi.c#L34) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 23 | `win16_style_hit_test` | [win16_style.c:53](../../src/ntw16s-dll/win16_style.c#L53) [win16_style.h:30](../../src/ntw16s-dll/win16_style.h#L30) | [global_mode_target.c:27](../../tests/integration/global_mode_target.c#L27) [abi.c:35](../../tests/ntw16s-dll/abi.c#L35) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 24 | `win16_style_install` | [win16_style.c:131](../../src/ntw16s-dll/win16_style.c#L131) [win16_style.h:17](../../src/ntw16s-dll/win16_style.h#L17) | [abi.c:36](../../tests/ntw16s-dll/abi.c#L36) [dialog_scope.c:88](../../tests/ntw16s-dll/dialog_scope.c#L88) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 25 | `win16_style_list_scrollbar_enable` | [attachment.c:402](../../src/ntw16s-dll/attachment.c#L402) [controls.c:215](../../src/ntw16s-dll/controls.c#L215) [controls.h:61](../../src/ntw16s-dll/controls.h#L61) | [abi.c:37](../../tests/ntw16s-dll/abi.c#L37) [scroll_frame_target.c:112](../../tests/ntw16s-dll/scroll_frame_target.c#L112) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 26 | `win16_style_menu_measure` | [menu_bar.c:49](../../src/lib-win32-style/appearance/win3x/menu/menu_bar.c#L49) [menu_paint.c:45](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.c#L45) [menu_paint.h:15](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.h#L15) | [abi.c:38](../../tests/ntw16s-dll/abi.c#L38) [runtime.c:64](../../tests/ntw16s-dll/runtime.c#L64) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 27 | `win16_style_menu_text` | [menu_bar_paint.c:15](../../src/lib-win32-style/appearance/win3x/menu/menu_bar_paint.c#L15) [menu_paint.c:122](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.c#L122) [menu_paint.h:13](../../src/lib-win32-style/appearance/win3x/menu/menu_paint.h#L13) | [abi.c:39](../../tests/ntw16s-dll/abi.c#L39) [runtime.c:66](../../tests/ntw16s-dll/runtime.c#L66) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 28 | `win16_style_remove` | [win16_style.c:128](../../src/ntw16s-dll/win16_style.c#L128) [win16_style.h:23](../../src/ntw16s-dll/win16_style.h#L23) | [abi.c:40](../../tests/ntw16s-dll/abi.c#L40) [dialog_scope.c:89](../../tests/ntw16s-dll/dialog_scope.c#L89) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 29 | `win16_style_scrollbars_paint` | [attachment.c:315](../../src/ntw16s-dll/attachment.c#L315) [controls.c:143](../../src/ntw16s-dll/controls.c#L143) [controls.h:69](../../src/ntw16s-dll/controls.h#L69) | [abi.c:41](../../tests/ntw16s-dll/abi.c#L41) [folder_dialog_target.c:103](../../tests/ntw16s-dll/folder_dialog_target.c#L103) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 30 | `win16_style_set_palette` | [win16_style.c:139](../../src/ntw16s-dll/win16_style.c#L139) [win16_style.h:26](../../src/ntw16s-dll/win16_style.h#L26) | [abi.c:42](../../tests/ntw16s-dll/abi.c#L42) [frames.c:78](../../tests/ntw16s-dll/frames.c#L78) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 31 | `win16_style_set_scroll_info` | [controls.c:279](../../src/ntw16s-dll/controls.c#L279) [window_titles.c:313](../../src/ntw16s-dll/injection/window_titles.c#L313) [win16_style.h:35](../../src/ntw16s-dll/win16_style.h#L35) | [abi.c:43](../../tests/ntw16s-dll/abi.c#L43) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 32 | `win16_style_tabbed_list_install` | [controls.c:231](../../src/ntw16s-dll/controls.c#L231) [controls.h:34](../../src/ntw16s-dll/controls.h#L34) | [abi.c:44](../../tests/ntw16s-dll/abi.c#L44) [win16_style_list_tabs.c:64](../../tests/ntw16s-dll/win16_style_list_tabs.c#L64) | 保留已发布调用契约；切换时核对外部调用 |
| Win16 | 33 | `Ntw16sShowModernFileDialog` | [dialogs.c:111](../../src/demo-exe/dialogs.c#L111) [api.h:11](../../src/ntw16s-dll/api.h#L11) [modern_dialogs.c:6](../../src/ntw16s-dll/injection/modern_dialogs.c#L6) | [abi.c:15](../../tests/ntw16s-dll/abi.c#L15) [modern_dialog_target.c:52](../../tests/ntw16s-dll/modern_dialog_target.c#L52) | 迁生产消费者，保留语义 |
| Win16 | 34 | `Ntw16sGlobalHook` | [global.c:54](../../src/run16s-exe/global.c#L54) [hook.c:28](../../src/ntw16s-dll/global/hook.c#L28) [global_dialog_thread.h:7](../../tests/integration/global_dialog_thread.h#L7) | [global_propagation_probe.c:99](../../tests/integration/global_propagation_probe.c#L99) [abi.c:10](../../tests/ntw16s-dll/abi.c#L10) | 迁生产消费者，保留语义 |
| Win16 | 35 | `Ntw16sGlobalHost` | [global.c:55](../../src/run16s-exe/global.c#L55) [hook.c:61](../../src/ntw16s-dll/global/hook.c#L61) | [abi.c:11](../../tests/ntw16s-dll/abi.c#L11) | 迁生产消费者，保留语义 |
| Win16 | 36 | `Ntw16sWindowAttach` | [attachment.c:416](../../src/ntw16s-dll/attachment.c#L416) [window.c:8](../../src/ntw16s-dll/global/window.c#L8) [window.h:6](../../src/ntw16s-dll/global/window.h#L6) | [win16_dwm_restore.c:24](../../tests/lib-win32-style/application/window/win16_dwm_restore.c#L24) [abi.c:17](../../tests/ntw16s-dll/abi.c#L17) | 迁共享窗口事务，保留契约 |
| Win16 | 37 | `Ntw16sGlobalThread` | [session.c:11](../../src/ntw16s-dll/global/session.c#L11) [global_module_fixture.h:7](../../tests/ntw16s-dll/global_module_fixture.h#L7) | [abi.c:12](../../tests/ntw16s-dll/abi.c#L12) [global_load_fixture.c:19](../../tests/ntw16s-dll/global_load_fixture.c#L19) | 迁生产消费者，保留语义 |
| Win16 | 38 | `Ntw16sUwpFrameHook` | [target.c:21](../../src/ntw16s-dll/uwp/target.c#L21) [hook.c:39](../../src/ntw16s-dll/uwp/hook.c#L39) [hook.h:13](../../src/ntw16s-dll/uwp/hook.h#L13) | [modern_frame.c:45](../../tests/lib-win32-style/appearance/win2000/window/modern_frame.c#L45) [abi.c:13](../../tests/ntw16s-dll/abi.c#L13) | 迁生产消费者，保留语义 |
| Win16 | 39 | `Ntw16sWindowThreadHook` | [window_target.c:13](../../src/run16s-exe/window_target.c#L13) [attachment.c:441](../../src/ntw16s-dll/attachment.c#L441) [window_thread.h:13](../../src/ntw16s-dll/window_thread.h#L13) | [abi.c:21](../../tests/ntw16s-dll/abi.c#L21) [window_thread.c:33](../../tests/ntw16s-dll/window_thread.c#L33) | 迁生产消费者，保留语义 |
| Win2000 | 1 | `Ntw2ksFinishHelper`（NONAME） | [序号调用](../../src/lib-win32/process-hook/helper.c) [dllmain.c:22](../../src/ntw2ks-dll/dllmain.c#L22) | — | 保留同序号中性入口 |
| Win2000 | 2 | `Ntw2ksDrawButton` | [button.c:4](../../src/ntw2ks-dll/button.c#L4) | [button.c:11](../../tests/ntw2ks-dll/button.c#L11) | 保留已发布调用契约；切换时核对外部调用 |
| Win2000 | 3 | `Ntw2ksWindowAttach` | [window.c:19](../../src/ntw16s-dll/global/window.c#L19) [window.c:8](../../src/ntw2ks-dll/window.c#L8) [window.h:6](../../src/ntw2ks-dll/window.h#L6) | [bars.c:222](../../tests/lib-win32-style/appearance/win2000/control/bars.c#L222) [button_paint.c:134](../../tests/lib-win32-style/appearance/win2000/control/button_paint.c#L134) | 迁生产消费者，保留语义 |
| Win2000 | 4 | `Ntw2ksWindowAttached` | [window.c:19](../../src/ntw16s-dll/global/window.c#L19) [window.c:9](../../src/ntw2ks-dll/window.c#L9) [window.h:7](../../src/ntw2ks-dll/window.h#L7) | [bars.c:223](../../tests/lib-win32-style/appearance/win2000/control/bars.c#L223) [button_paint.c:134](../../tests/lib-win32-style/appearance/win2000/control/button_paint.c#L134) | 迁生产消费者，保留语义 |
| Win2000 | 5 | `Ntw2ksWindowRemove` | [window.c:19](../../src/ntw16s-dll/global/window.c#L19) [window.c:10](../../src/ntw2ks-dll/window.c#L10) [window.h:8](../../src/ntw2ks-dll/window.h#L8) | [bars.c:224](../../tests/lib-win32-style/appearance/win2000/control/bars.c#L224) [button_paint.c:134](../../tests/lib-win32-style/appearance/win2000/control/button_paint.c#L134) | 迁生产消费者，保留语义 |

复查命令使用 `git grep -n -E` 一次按公共名扫描 `src` 和 `tests`，用两份 Win16 和一份 Win2000 DEF 作为全集。序号入口另读 helper 的调用约定。启动器、现代宿主及全局窗口分派的模块名消费者见[现有职责审计](../proposals/msstyles-classic-runtime.md)。模块名兼容属于 S10/S11 的实际包验收。
