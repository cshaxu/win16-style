# T26 S1 菜单弹出窗口局部审计

本记录覆盖 `win16-window/menu_scope.c`、`win16-window/menu_iteration.c`、`win2k-window/system_menu.c` 和 `win2k-window/frame.c` 中菜单接入，与公共 `menu_input`、`menu_snapshot`、`menu_items` 的关系；不表示其他菜单文件已全部审核。

| 能力 | 现状与归属 | 后续验证或限制 |
| --- | --- | --- |
| 弹出菜单通知、左右键与指针路由 | 两种风格调用 `style_menu_popup_observation`、`style_menu_pointer`、`style_menu_bar_arrow` 等公共输入机制；两侧菜单定位和选中策略仍不同。 | 不应在两个调用方硬编码统一的命中规则；需保留多列、禁用项、键盘与鼠标路径。 |
| 系统弹出菜单内容与坐标 | 两种风格调用 `style_menu_read_row` 和 `style_menu_client_offset`，缓冲呈现调用 `style_present_buffer`；字体、配色、边框、文字和图标绘制仍属各自风格。 | 弹出窗口实际绘制及重入须分别验证；读取成功不等于像素一致。 |
| 显示项的选中状态 | Win16 `system_selection` 记录最多 64 项完整位集且读取失败时返回失败；Win2000 `selected_item` 返回首个选中项，单项读取失败则继续；公共 `style_menu_char` 为助记符循环记录最后一个选中项。 | 三者失败与多选状态语义不同，不能直接替换为同一个既有函数；若抽离须保留语义并验证同步消息重入。 |
| 子类和嵌套生命周期 | Win16 用作用域栈管理多个弹出窗口、引用与延迟释放，还隐藏并恢复原生阴影；Win2000 作用域管理单个系统菜单弹窗，追踪重绘状态。 | 共同的回调与销毁生命周期值得进一步拆分，但须先覆盖嵌套、窗口先销毁、移除失败和重入；阴影及菜单策略不能硬塞入公共层。现有两者行为并非相同，尚未迁移。 |
| 跨菜单切换 | Win16 `menu_iteration` 处理系统菜单槽、`WM_NEXTMENU`、跨窗口焦点和借用状态；Win2000 `frame.c` 根据实际菜单位置、禁用状态和原生子菜单进行跟踪。 | 可共享的输入判断已部分抽离；直接合并循环会改变 MDI、焦点或执行命令语义，需在产品路径分别测试后再拆。 |

证据来自上述当前源码及 `tests/lib-style/common/menu_items.c`；这份静态局部审计不证明运行时完整覆盖，亦不代表所有可复用机制已抽完。已发布的候选及测试结果以 CURRENT 为准。
