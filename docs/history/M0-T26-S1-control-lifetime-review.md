# T26 S1 控件生命周期局部复核

范围是 `lib-style/win16-control/control_lifetime.c/h` 的当前全部生产函数，不代表整个控件库审计完成。

| 入口 | 审核结论和调用证据 |
| --- | --- |
| `win16_control_spin_before` | 在 WM_SETFONT 前记录编辑区域；按钮识别依赖 `win16_control_spinedit_button` 中 VCL `TSpinEdit` 角色及父子关系。保留在 Win16，不能把框架名称冒充通用条件。 |
| `win16_control_finish_message` | 包含微调按钮保留区恢复、应用字体来源/Win16 字体租约、嵌套绘制计数和 Win16 状态销毁。可复用的子类所有权判定已由 `control_scope` 提供；整函数上移会反向依赖 `control_style` 和 Win16 字体。 |
| `win16_control_updown_acquire/release` | 所有权核验复用 `style_control_subclass_owned`；计数与 `control_style` 存活期和 Win16 析构绑定。不能直接改用现代窗口宿主生命周期对象，也不应为两行计数额外复制一套管理器。 |
| `win16_control_scrollbars_paint` | 使用 Win16 调色板、具体滚动条状态和绘制回调；表面呈现基础设施已有公共组件。保留该风格组装。 |
| 轨迹条/工具栏解绑 | 产品回调现直接使用公共 `style_control_subclass_remove_owned`，Win16 库中两份重复实现已删除；报告控件也使用该公共机制。双位宽实际控件回归见[记录](M0-T26-S1-owned-subclass.md)。 |

进一步拆分应以某个真实第二消费者及其生命周期契约为依据；不能仅靠重命名结构或增加空转发宣称通用。上述判断不排除将来发现可共享的其他边界。
