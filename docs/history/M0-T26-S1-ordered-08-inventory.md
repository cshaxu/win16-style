# T26 S1 顺序第 8 项：附着器逐函数盘点

基线 `aa19b490`。范围为 `win16-control/attachment_guard.c` 的 51 个非静态函数；产品的主要组装和调用位于 `ntw16s-dll/attachment.c`。这是一份进行中的分类，不是迁移完成证明。

| 函数组 | 职责、已共享部分与剩余边界 |
| --- | --- |
| `retire_frames`, `guard_cool`, `guard_roles` | 根据父子关系和角色变化撤销 Win16 附着；具体状态保存在 `attachment`。VCL 身份核验已调用 `common/control_vcl_identity`，VB 身份由公共控件识别和风格状态共同决定。 |
| `post_box`, `post_controls`, `post_toolbar`, `post_verified` | 绘制后验证证据并调用所选 Win16 安装器；包括重入撤销及协议租约释放。`common/control_protocol` 已提供绑定和身份基础，但完成状态、安装回调及回滚次序耦合 `attachment`，不能直接整体上移。应继续审计能否提取不依赖这些字段的事务契约。 |
| `native_prepare`, `post_native` | 保存打印剪裁或更新区域，识别颜色、日期和状态控件，调用 Win16 绘制回调。识别已有 `common/control_native_combo` 和 `common/control_status`；剪裁快照及恢复有可共享潜力，但现有 `common/clip.h` 只判断覆盖范围，不能把它当作同一功能。需先明确 DC 所有权、失败状态、无剪裁和重入行为。 |
| `install_style` | 按 Win16 候选、具体控件和滚动条策略选择安装器，保存错误并在失败时撤销。主题状态由 `common/theme_state` 提供；整函数包含风格资格和绘制选择。通用事务抽离须保留失败错误、同步重入和主题回滚语义，不能只增加转发层。 |
| `report_bind`, `vcl_report_bind`, `report_unbind`, `report_notify` | 绑定公共控件协议和父窗口通知子类，Win16 绘制作用域与报告控件策略留在风格侧；子类所有权移除已使用 `common/control_scope.h`。普通与 VCL 路径的可共享绑定事务仍需细核协议寿命与重入。 |
| `attached`, `scroll_changed`, `remove`, `terminal` | 查询 Win16 角色状态、刷新滚动条、普通移除和终止清理。普通移除可调用恢复路径，终止清理只退役节点且保留错误；两者均持有引用以应对重入。Win2000 `ntw2ks-dll/controls.c` 当前仅对部分标准控件管理公共主题抑制，不具备同一套子类/字体/滚动条状态。`common/control_scope` 的根窗口作用域引用计数不能直接替代这里的窗口附着生命周期；应设计独立通用所有权机制并用两种实际调用方验证后再合并。 |
| `classify_initial`, `classify_vcl`, `eligibility`, `initialize` | 分类和记录 Win16 当前支持的标准、VB/VCL 和自定义控件；共同的类、协议及部分 VCL 识别已在公共组件。`eligibility` 和具体候选组属于风格接入政策；`initialize` 中的通用身份及引用持有是否能独立抽出仍需核对其生命周期。 |
| `observe_native`, `dbedit_ansi_witness`, `dbcombo_ansi_witness` | 原生过程和调用模块归属已调用公共 `control_native_witness`；哪些消息、控件及附着实例可以生成证据仍由 Win16 策略决定。双位宽 ANSI DBEdit 和 Unicode 轨迹条正式包回归见[交付记录](M0-T26-S1-ordered-08-witness.md)。 |
| `static_paint`, `spin_message`, `prepare_report`, `reset_witness`, `dispatch_kind` | Win16 按实际候选和消息选择绘制/回调作用域；报告绘制含具体字体及 owner-draw 策略，不应将这些分支命名为通用引擎。 |
| `mdi_identity`, `combo_same_role`, `vcl_combo_candidate`, `vcl_combo_valid`, `group_button_valid`, `group_push_button`, `group_choice_button` | VCL 和 MDI 角色、父子、样式及已安装状态检查；部分身份已复用 `common/control_vcl_identity` 和 `lib-win32/style/window_identity`。MDI 组合检查是否与其他消费者相同仍待核实。 |
| `vb_data_valid`, `vb_data_attached`, `vb_user_label_target`, `vb_user_edge_dormant`, `vb_user_edge_valid`, `grid_valid`, `vcl_toolbar_valid`, `vcl_tab_valid` | VB/VCL 特定身份与风格安装状态检查，公共身份不包含 Win16 的候选与生命周期状态；不能只移动条件表达式而保留反向依赖。 |
| `inplace_paint`, `checklist_draw_target`, `box_witness`, `rich_valid`, `rich_witness`, `cool_identity` | 验证绘制目标、线程/父子及各框架证据；部分调用公共 VCL 身份。`rich_witness` 使用模块指定的 ANSI/Unicode 类，而已抽出的公共证据函数固定查询全局类并要求窗口类模块等于调用方，二者契约不同，不能机械替换。 |

函数名前缀均为 `win16_attachment_`。文件静态辅助函数 `release_failed_protocol` 和 `db_ansi_witness` 分别服务于协议回滚及 ANSI 证据路径，已包含在相邻函数组判断中。

尚须完成：对上表事务和剪裁潜力做重复实现及真实消费者检查，补充涉及的失败/重入覆盖，完成后才能将第 8 项标为过关。既有导航焦点、Win2000 DWM 恢复和真实现代窗口覆盖限制仍开放。

## 进一步对照和回归

- 打印剪裁：全库检索 `GetClipRgn` 和 `SelectClipRgn`，该保存—恢复路径当前仅在此处出现；公共 `clip.h` 的覆盖判定不保存跨调用快照，Win16 菜单和位图路径使用的是不同的裁剪组合。当前没有第二个相同消费者，不能简单合并为一行转发而声称两种风格已共享；后续为其他风格实现打印控件时应复用同一快照契约。
- 报告绑定：普通路径允许现有父窗口并按协议决定树或列表；VCL 路径要求父窗口与绘制证据匹配，并在绑定后安装外框、失败后回滚和允许重试。两条路径均使用公共协议和公共子类所有权移除，但不能直接把整个绑定过程替换为同一事务。
- 已发布版本 `aa19b490`：分别运行 32 位和 64 位 `core-box-model`、`core-vcl-report-module`、`core-vcl-toolbar-module`、`core-vcl-edit-module`，均通过；观察了伪装身份、异常展开、重入移除、父窗口变更、绘制及清理。测试显式使用 `--private-worker` 直接入口，没有创建或切换桌面。测试没有覆盖所有 51 个函数的全部分支、Win2000 的对应行为或真实第三方程序。
