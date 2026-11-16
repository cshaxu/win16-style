# M0 T26 S10 — 扩展控件与运行时适配

## 准入与边界

S9 P1 f9de3404 已完成同快照双宽八文件发布后146项聚焦通过、两项旧DWM失败不变及实际远端复核，按批准的14个S顺序自动准入。M0固定、T26开放；本S不是新T。不重做已经公共化的基本控件状态、combo、scroll和生命周期。

目标是扩展控件的原生协议、快照、通知与实际运行时状态/资源归application/control或application/adapters/win32；Win3x只提供既有度量、字体、栅格、颜色及呈现资格。公共对象拥有身份和资源，完整操作回调代替字段指针view，实际产品调用后删除旧机制和无意义接线。不新增Win2000控件或扩大VB/VCL支持集合。

## 按文件顺序的有限审计准备

先列冻结清单和逐项接收者，再按文件名顺序审计全部本S相关文件与实际调用，不随机搬叶函数。S9明确交接：attachment_state及产品attachment的扩展候选/parent/witness/通知/TLS，choice_paint的checklist原生身份与重入，control_dispatch的tab/header/progress/trackbar/fileowner分支，special_install/notify/native_presentation的扩展协议，control_status/native_combo等既有public view所有权及对应测试。复用S9的公共instance、control_state、retire和资源对象，不能另建平行根。

创建scope、creation_begin/before/return/end、old-file-dialog pending和结果事务仍归S11；现代composed-frame与宿主归S12；最终renderer注册及通用图形fit/present资源归S13。冻结时列具体函数和真实消费位置，不以“风格/业务逻辑”包揽这些机制。S10的身份和绘制支持政策不得改变。

## 实施与验收

按完整能力合批：先扩展状态/通知生命周期，再原生项目/文本快照及编辑/呈现协议，最后VB/VCL适配与产品装配精简。批次可按冻结审计发现合并；不预称每批都需完整交付。开发使用稳定.ninja/.ccache的双宽增量和相关原生测试；同一公共协议可由独立无Win3x测试使用，实际Win3x消费者必须调用同一实现。

既有原生返回/LastError、应用自绘、文本/图片/数据/字体所有权、通知时序、换父/角色变化、重入解除/销毁和失败清理维持。使用自有当前桌面目标、有界等待和清理；禁止computer-use和私有桌面，不修改或依赖兄弟仓库。默认使用isolated_process夹具前检查显式--private-worker提前返回路径，不重复历史desktop违规。

全部有限清单落实后一次干净同快照双宽八文件打包，先发布assets根与O:/winnt，再实际包回归、提交推送及实际远端读取。保持原资源/来源/ABI，失败或环境限制如实记录；只停用不能称物理卸载。每批或约30分钟复核效率并立即清理已终态scratch，保留增量缓存。不逐文件发布。退出条件满足后自动S11，T26保持开放。

## 状态

S10 active，当前为冻结清单与审计阶段；无S10生产改动或构建启动。S9正式包为当前发布基线。不能把本brief写成全部扩展机制已迁移或所有框架验证通过。
