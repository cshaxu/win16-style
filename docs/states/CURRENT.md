# Project Status

## Current Work

M0 T21 S13：真实 VB6 全局模式对照及钩子注册错误 126 调查。

## M0 T21 S13 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；T21 S13 新准入 |
| Admission And Approval | 用户授权全局模式适配真实传统程序，自动推进 S |
| Objective | 以 MadShow 对照全局效果，并调查实际部署位置的系统钩子注册错误 126 |
| Non-goals | 不依赖该应用构建或测试，不操作现有用户进程，不处理 UWP/XAML |
| Reference Baseline | S4 已验证 demo 和可选记事本；VB6 全局对照缺少证据 |
| Candidate Proposal | [手动全局样式模式](../proposals/global-style-mode.md) |
| Files And ABI Surface | 自有原生私有桌面探针及必要修复；不增加组件，新增双宽内部控制入口须核对 ABI |
| Applicable Rules | M0、C17、GCC、Ninja、ccache，原图形资源不变 |
| Verification | 自有隔离实例比较原版、普通启动、全局启动及停止后窗口与控件 |
| Expected Markers | 区分附着、实际绘制、控件交互和恢复，记录路径及失败 |
| Asset Needs | build/t21-s13、build/t21-package 自有 scratch |
| Reporting Requirements | 真实样本有界结论、差异和未覆盖条件 |
| Stop Conditions | 不能确认独立目标实例和私有桌面时不操作 |
| Exit Criteria | 完成调查及必要修复、双宽干净构建、实际包测试、必要发布、提交推送和复核 |
| Original Owner Request | 全局样式覆盖已有和新启动程序并统一附着逻辑 |
| Similar-Issue Sweep | 运行时控件、启动钩子差异、系统钩子 DLL 共享表及实际部署路径 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 21
- Last allocated Td S: 18
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 13
- Last delivery: M0 T21 S12 双宽动态控件回归；[记录](../history/M0-T21-S12-late-controls.md)。
- Open task: T21 S13；Explorer.exe 仍被排除。
- S13 正在实现目标内线程钩子备用路径：内部工作单元、双宽测试及 ABI 控制入口已加入源码，尚未接入控制器或发布；当前部署路径 126 未解决。
- S13 阶段包已发布至 assets 和 O:/winnt；干净包 23 项聚焦通过，但本会话部署路径全局钩子注册仍报 126，未通过部署验收；[阶段证据](../history/M0-T21-S13-vb-global-investigation.md)。
- 打印当前页已纳入受限 PrintDlgEx 转接；跨 DPI、实际打印和本地化仍待覆盖。
- COM并发：已观察的双多选并发由系统原生路径止损；旧框目录串用根因、非多选并发仍未查明，详见[S3证据](../history/M0-T20-S3-com-safety.md)。
- COM多选：默认回退系统新框，O: 长名、单项、取消、链接及双宽并发经有限验证；旧框实现及其他参数仍未全面解决。
- Shell筛选：受影响的带回调组合保留系统新树及原筛选接口；旧树中完整支持该回调仍未实现，其他标志组合未全面验证。详见[S3证据](../history/M0-T20-S3-shell-filter.md)。
- P253 已发布：干净包 58 项通过；系统打印宿主自然更替后，已发布包及无注入对照的双宽 A/W Print/Apply 实际虚拟打印各八项通过。此前 32 位 EndPage 失败的原因未明；详见阶段证据。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；P234 处理旧式列表系统主题动画覆盖项目滚动条，同源码增量候选的双宽真实拖动通过，干净包的双宽聚焦消息驱动测试及 x64 真实拖动通过，干净包 x86 真实鼠标复测因近期输入跳过。按用户最新指示暂停COM布局桥接及其长尾。事件桥接实验默认关闭，定制、隐藏目录及完整矩阵仍未完成；公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；S1已收口；T20此后按用户要求有限范围收口。

## Recent Closure

- M0 T21 S12：动态标准按钮双宽附着、点击和退出恢复；13 项实际包回归，远端 P1 7cf6481 及四文件摘要已复核。[记录](../history/M0-T21-S12-late-controls.md)。

- M0 T21 S11：双宽干净构建、18 项实际包回归及双位置发布；[记录](../history/M0-T21-S11-restart-timeout.md)。

- M0 T21 S10：双宽自有 Shell 视图的外框及滚动条附着/解除已验证；DirectUI 内部未样式化，远端 6760373 已复核。[记录](../history/M0-T21-S10-shell-view.md)。

- M0 T21 S9：Explorer 三种私有桌面启动均未产生自有窗口，不修改排除规则；远端 3947c81 与两处四文件摘要一致。[记录](../history/M0-T21-S9-explorer-isolation.md)。

- M0 T21 S8：双宽 96 DPI 虚拟化及 144 DPI 对照通过，远端 be2e5c4 与两处四文件摘要一致；[记录](../history/M0-T21-S8-dpi-unaware.md)。

- M0 T21 S7：简单窗口所有权语义保留，144 DPI 双宽受控输入与 18 项聚焦包测试通过；远端 782d2c3 与两处四文件摘要一致。[记录](../history/M0-T21-S7-input-dpi.md)。

- M0 T21 S6：双宽四线程、八窗口和三次启停通过，远端 4a88f58 及两处四文件摘要一致；[记录](../history/M0-T21-S6-threads-resources.md)。

- M0 T21 S5：普通与全局共用窗口附着实现，远端 35bcfaa 及两处四文件摘要一致；[审计](../history/M0-T21-S5-coverage-audit.md)。

- M0 T21 S4：实际应用和双宽混合顺序已验证，宿主故障检测已修复并在远端 8f21d36 复核；[记录](../history/M0-T21-S4-real-lifecycle.md)。

- M0 T21 S3：手动全局模式已交付，远端 a9c178c 与两处四文件摘要一致；正常 run16s 效果独立保留。[记录](../history/M0-T21-S3-global-controller.md)。

- M0 T21 S2：同线程窗口接入基础已交付；全局模式仍在实施。[记录](../history/M0-T21-S2-global-window.md)。

- M0 T21 S1：双位宽全局钩子隔离可行性和恢复限制已验证，用户接受残影；未实现产品全局入口。[证据](../history/M0-T21-S1-global-prototype.md)。

- M0 T20 S19 / T20：按用户指令有限范围收口；系统菜单与首次绘制交付，真实桌面边界留TODO。[收口审计](../history/M0-T20-closure-audit.md)。

- M0 T20 S18：颜色框基本色及自定义色块恢复细黑框平面外观，双宽包验证和发布；[记录](../history/M0-T20-S18-color-swatches.md)。

- M0 T20 S17 / T20：最终双宽干净构建及 18 项精简包回归通过；当前发布包和剩余边界见[交接](../history/M0-T20-S17-final-handoff.md)。

- M0 T20 S16：普通文件快捷方式的受限默认解析，32 项包回归、双位置发布及远端复核；[记录](../history/M0-T20-S16-file-link-resolve.md)。

- M0 T20 S15：显式保留快捷方式的打开请求转接旧框，32 项实际包聚焦回归及双位置发布通过；[记录](../history/M0-T20-S15-file-link-flag.md)。

- M0 T20 S14 / T20：常用对话框有限范围审计、双宽干净包 23 项聚焦回归；首次超时及回退如实记录。[任务级审计](../history/M0-T20-S14-completion-audit.md)。

- M0 T20 S13：32 项干净包聚焦回归及远端复核，错误交接原型不进入产品；[记录](../history/M0-T20-S13-shadow-coverage.md)。

- M0 T20 S12：真实API错误原型及28项包回归；ChooseFont过期错误缺口有负例，未进入产品。[记录](../history/M0-T20-S12-shadow-prototype.md)。

- M0 T20 S11：扩展错误双宽对照及28项干净包回归；嵌套取消保留内层错误。[记录](../history/M0-T20-S11-commdlg-error.md)。

- M0 T20 S10：导航后选择原型、IDABORT修正及68项干净包回归；错误接口未实现。[记录](../history/M0-T20-S10-file-selection.md)。

- M0 T20 S9：双宽目录/筛选原型及24项干净包回归；尚未结合文件确认。[记录](../history/M0-T20-S9-file-navigation.md)。

- M0 T20 S8：固定两项原型和40项干净包回归；未进入产品，导航与筛选未验证。[记录](../history/M0-T20-S8-win32-multi-prototype.md)。

- M0 T20 S7：Win32文件多选双宽对照及24项干净包回归；长名多选仍回退Explorer。[记录](../history/M0-T20-S7-win32-multi.md)。

- M0 T20 S6：探针修订、双宽干净包26项及远端复核；临时目录旧Shell对照超时仍未解决。[证据](../history/M0-T20-S6-folder-probe.md)。

- M0 T20 S5：最终交接、双宽构建、部署包18项及实际路径探针通过；P1 87216a1 已推送并读取远端对象复核。T20 保持开放。[记录](../history/M0-T20-S5-handoff.md)。

- M0 T20 S4：双宽干净构建、实际重建包40项通过；部署包字节未改，旧探针超时保留。[记录](../history/M0-T20-S4-common.md)。

- M0 T20 S3：Shell筛选与COM多选已安全回退，双宽构建、实际包测试及远端复核；[Shell](../history/M0-T20-S3-shell-filter.md)、[COM](../history/M0-T20-S3-com-safety.md)。

- M0 T20 S2：入口与回退矩阵已交付、双宽构建及实际包代表回归通过；[证据](../history/M0-T20-S2-dialog-audit.md)。

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
