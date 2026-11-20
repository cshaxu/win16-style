# M0 T29 S20 Win32 基础控件

## 首轮基线

两套生产入口分别在 `appearance/win3x/control/control_install.c` 和
`appearance/win2000/control/control.c` 打开主题。先统一按钮、复选、单选和分组，
随后接入编辑框、列表和组合框、静态文字与分隔、滚动条。保留原控件数据、通知、
捕获、DPI、启停和应用自绘契约。复合控件及专用框架能力由 S21 承接。

| 能力 | 现有共享基础 | 迁移重点 |
| --- | --- | --- |
| 按钮、复选、单选、分组 | 按钮状态快照、`classic_push_color`、`classic_choice_entry` | Win16 读主题索引图而 Win2000 使用自己的边框及字形。共用图形装入、状态、几何和绘制。 |
| 编辑 | `application/control/edit`、`edge` 和控件实例 | 对照客户区边界、只读、禁用、高 DPI 及原生文字、选择。 |
| 列表和组合 | `list_snapshot`、`combo_snapshot`、`control_native_combo` | 从主题读颜色和表面，保留父窗调色、自绘、下拉及标记。 |
| 静态文字、分隔与分组 | `application/control/basic`、按钮快照 | 共用主题可见配方，保留应用字体、内容、位图及通知。 |
| 滚动条 | `scroll_controller`、`scroll_surface`、`scroll_track` | 从主题装入共享几何和绘制，覆盖客户与非客户、长短轨、DPI、命中和恢复。 |

在既有 Ninja 树中运行双宽基础共享测试 18/18，覆盖按钮、组合与列表快照及
滚动条消息、几何、托管、跟踪、控制器和资源。另跑 Win2000 原生控件矩阵
双宽 30/30，覆盖按钮、静态、编辑的普通／DPI 感知及 Common Controls v6
组合，和滚动条布局、附着。它们均为迁移前基线。
