# T26 S1 Win16 控件当前逐文件索引

基线 `eb1e85de`。本表按当前 `.c` 文件列出 51 个条目，测试列仅表示直接包含对应头文件，不能证明运行覆盖；CMake 编译与实际产品行为另见测试映射及候选记录。分组是待审决策，不声称逐函数审计已完成。

| 源码 | 当前分类 | 直接包含其同名头的测试 |
| --- | --- | --- |
| `attachment_guard.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `basic_message.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `basic_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | `tests/lib-style/win16-control/control_link.c` |
| `choice_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `choice_view.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `combo_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `combo_text.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_dispatch.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_guard.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_install.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_lifetime.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_remove.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `control_scope.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/lib-style/common/control_scope.c`, `tests/lib-style/win16-control/control_scope.c` |
| `control_state.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/lib-style/win16-control/control_link.c` |
| `cool_grip_identity.c` | 身份/资格与风格条件混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `edit_message.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `fileowner_message.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/lib-style/win16-control/control_link.c` |
| `header_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `list_message.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `list_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-control/list_paint.c` |
| `month_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `month_text.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/ntw16s-dll/month_model.c` |
| `native_combo_identity.c` | 身份/资格与风格条件混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `native_presentation.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `notify_message.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `other_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `progress_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `scroll_assets.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | `tests/lib-style/win16-control/control_link.c` |
| `scroll_layout.c` | Win3.1 几何或交互；保留并对照其他年代 | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |
| `scroll_message.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `scroll_native.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |
| `scroll_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `scroll_track.c` | Win3.1 几何或交互；保留并对照其他年代 | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |
| `special_install.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `special_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `spin_scope.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `status_font.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `status_frame.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `status_text.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `tab_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `text_policy.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/lib-style/win16-control/control_link.c` |
| `theme_kind.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/lib-style/win16-control/control_link.c` |
| `toolbar_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `trackbar_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `updown_adapter.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/ntw16s-dll/updown_owner.c` |
| `updown_paint.c` | 以 Win16 绘制或资源为主；仍须核对公共机制 | 未发现直接包含；须查运行入口 |
| `vb_text.c` | 生命周期、消息或策略混合；逐函数复核 | `tests/ntw16s-dll/global_vb_text.c`, `tests/ntw16s-dll/vb_font.c` |
| `vcl_extra_frame.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `vcl_frame_policy.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |
| `vcl_identity.c` | 身份/资格与风格条件混合；逐函数复核 | `tests/lib-style/common/control_vcl_identity.c` |
| `vcl_native_paint.c` | 生命周期、消息或策略混合；逐函数复核 | 未发现直接包含；须查运行入口 |

## 已确认的公共边界

- 创建作用域的嵌套链、观察器与延迟释放位于 `common/control_scope`；Win16 适配器保留选择、安装和调色板。Win2000 对话框观察器并未采用该作用域。
- 控件协议、主题状态、坐标核验、自定义绘制协商、滚动消息基础和 VCL 身份子集位于 `common`；公共接口的存在不代表 Win2000 已实现对应控件外观。
- 以上混合条目需要逐函数核对产品调用、状态所有权、回滚及现有测试；未映射项目不按通过或已迁移处理。滚动条的固定拇指与 Win3.1 取消/计时行为不能直接替代 Win2000 的行为。

## 待办顺序

1. 先核对控件安装、移除、子类和重入生命周期，抽出与主题状态无关的事务，并验证原调用路径。
2. 再核对通知、消息、DC 与控件身份，只共享明确契约；原生应用自绘、字体及风格像素继续由调用方决定。
3. 对照 Windows 2000 源码完成滚动条布局和交互差异分析后再决定共同模型；不得将 Win3.1 固定拇指冒充 Win2000 行为。
4. 与窗口、对话框和现代宿主清单共同形成后续 S 序列，并逐项关联双位宽、真实调用方及未通过的测试。
