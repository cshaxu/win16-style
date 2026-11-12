# M0 T26 S1：控件系统主题状态接入核对

这项核对发现实际差异，尚未完成统一或恢复实现。

- `src/ntw2ks-dll/controls.c::classic_control` 仅在自己的 `CreateWindowExA/W` 包装返回后，对指定名称的新建子控件调用 `SetWindowTheme(window, L"", L"")`。它没有调用公共 `style_theme_capture`、`style_theme_suppress` 或 `style_theme_remove`；也没有跟踪应用随后自行设置的主题。其调用点没有检查 `SetWindowTheme` 结果。
- `src/lib-style/common/theme_state.c` 已提供同线程状态捕获、抑制、可观察设置及移除，`src/ntw16s-dll/theme.c` 和创建作用域使用该机制。公共状态只能恢复经 `style_theme_set` 路由后观察到的请求，不能从系统读取此前任意应用指定的完整主题字符串。
- 两种产品是不同 DLL，`style-theme-state` 目前是静态库，仅通过 `win16-control` 链接进 Win16 DLL。简单地给 Win2000 DLL 再链接同一静态库，并不使两个 DLL 的内部状态或 subclass 标识自动互通，也不能自动获得 `SetWindowTheme` 请求拦截。
- `src/ntw16s-dll/global/hook.c::children` 明确只处理 Win16 子窗口；`Ntw2ksWindowRemove` 只调用 `win2k_frame_remove`。因此现有 Win2000 顶层窗口移除路径不提供上述直接子控件主题更改的逐项恢复。该判断来自代码调用关系，并非一次用户界面的动态测量。

迁移须以两种风格共用同一状态所有权、进入与退出路径为目标，明确普通启动和 global 对已有/新建子控件的行为；在解决跨 DLL 实例和原主题可观察性之前，不能只把 `SetWindowTheme` 包进公共函数就宣称可恢复。需用自建控件测试新建与已有、应用侧后续主题设置、16/2k/原样互切、销毁、重复移除以及双位宽；未观察到的主题设置须如实描述恢复边界。不能在不验证的情况下改动现有默认外观或钩子生命周期。

## 后续工作区变化

公共状态源码现由已有 `ntwshk` 装载并导出，Win16 产品使用同一实例；两位宽测试通过共享 DLL 捕获、Win16 实际入口设置、共享 DLL 恢复的跨模块路径。以上发现描述的是迁移前状态；Win2000 尚未接入，也未证明 Win2000 子控件恢复。此变更尚未发布。
