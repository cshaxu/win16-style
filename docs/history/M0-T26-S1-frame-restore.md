# T26 S1 外框重算与客户区恢复去重

`lib-style/common/frame.h` 的 `style_frame_changed_restore` 合并外框更新后
在窗口仍存在时恢复普通窗口客户区的相同操作。Win16 的
`win16_control_frame_restore` 和 Win2000 的 `win2k_frame_remove` 均实际
调用它。DWM 策略、待创建窗口检查及错误处理仍分别保留原行为。

双位宽 MinGW 增量编译和链接通过；Win16 控件链接测试与 Win2000
`core-win2k-frame-lifecycle --no-pump` 均在双位宽通过。带消息泵的
Win2000 `--direct` 两个位宽均返回 3：几何和重复附着检查通过，DWM 状态
恢复未通过。这与本变更前的[已有调查](M0-T26-S1-win2k-dwm-investigation.md)
所记录的结果一致，但仍是失败，不能证明原因或宣称完整恢复。

后续增加独立公共接口原生测试 `tests/lib-style/common/frame_restore.c`，
实际改变窗口大小并检查普通窗口客户区恢复和非普通分支不恢复；双位宽均
通过。该测试不覆盖 DWM 恢复或真实应用绘制。

该变更尚未完整构建、发布或提交；测试使用 `build/.ninja` 增量构建，未创建
额外临时构建。产品发布和 T26 范围内其余迁移仍未完成。
