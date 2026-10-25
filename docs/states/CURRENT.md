# Project Status

## Current Work

M0 T15 S1 verified：VB6/MadShow控件及客户区文字适配完成；同包31项、双发布和
安装复验通过，等待P1提交推送及实际远端复核，不提前关闭。T15等待最终用户外观验收，
M0开放；本次不扩大为全部VB6/OCX支持，既有任务/债务保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 11
- Last delivery: M0 Td S11 P2 (P1 bb59d0a实际远端六文档通过；154产品输入/八部署未变；登记scratch已回收)
- Open task: T15 S1；Td S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## M0 T15 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | 用户批准VB6适配实施、必要时自动划分并执行后续S；提交推送和双发布沿用永久授权 |
| Objective | 适配MadShow的VB6按钮、client-edge文本框及保留无窗口标签绘图的容器；验证受控标签字体路径 |
| Non-goals | 不承诺所有VB/OCX；不修改兄弟仓库、系统设置、应用业务或原图形；不猜私有VB对象布局 |
| Reference Baseline | 4a93d2e；T14 S3 0.14.0.0；Td S11原生类/窗口证据 |
| Candidate Proposal | [VB6适配](../proposals/madshow-controls.md) |
| Files And ABI Surface | DLL内部分类/绘制、原生测试及CMake/版本；三组件四产物，ABI3/31导出保持 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT；Task Reading Set已读 |
| Verification | 双宽原生自包含分类/绘制/清理测试；真实MadShow隔离实例、按钮/文本/标签消息及截图；双宽干净构建及完整实际包、原资源门禁 |
| Expected Markers | VB_WINDOWED_OK、VB_CONTAINER_PAINT_OK、VB_EDIT_OK、VB_CLEANUP_OK；未覆盖与失败独立记录 |
| Asset Needs | 原资源逐字节保留；scratch build/t15-vb、build/t15-package，仅本T拥有，证据提炼后安全回收；验收PNG保留于任务visualizations/win16style-t15-vb6 |
| Reporting Requirements | 分类/绘制/实际交互分别证明；旧包负例、四文件八摘要、输入冻结、统计、提交推送及实际远端读取 |
| Stop Conditions | 需要私有布局、全局设置或共享进程注入时停止该路径并给证据；不以模块存在代替实际绘制 |
| Exit Criteria | 相关实测/双宽回归、同包发布复核；必要续S自动执行；T15完成后等待用户外观验收 |
| Original Owner Request | 以VB6适配为重点；一个S装不下则多S自动准入执行，整个适配完成后停下来验收 |
| Similar-Issue Sweep | 检查attachment/control_scope/controls全部类名单及容器、client-edge、字体释放、重复附着/清理 |

## Technical Baseline

0.15.0.0 / T15 S1本地verified并双发布；提交推送/实际远端复核待完成。
163输入冻结，GCC16.1/Ninja/ccache干净双宽构建；31/31实际包250.62秒、
最终双宽各5项聚焦及安装ABI/协议/真实MadShow通过，无skip。76注册CTest未全部执行。
实际MadShow4按钮/13文本框/2分组附着及窗口less文字接入；两次整图一致，标题24行
及画布与旧包相同。借用源字体/原编辑消息保留，32私有租约与映射释放、33源字体
保留、分配配对0/0、容量/失败/屏障通过。宿主GDI总数另列观测，不假称全部OS缓存释放。
两处八摘要一致；ABI3/31、原五头/声明/18表保持；三组件四产物，无第五文件。
范围以[记录](../history/M0-T15-S1-vb6.md)及[证据](../etc/evidence/vb6-adaptation.json)为准；
真实VB6只验证本机x86 MadShow，其他类/OCX/复杂文字和物理多屏未承诺。
T14 S3启动错误语义、既有标题/标准字体/菜单/滚动条及其他任务状态保持。

## Recent Closure

- [T15 S1 VB6适配（verified/待远端复核）](../history/M0-T15-S1-vb6.md)：31实际包/双发布及安装复验已通过，暂不标记收口。

- [Td S11 MadShow控件调查](../history/M0-Td-S11-madshow-controls.md)：3有效对照/6自有实例，VB6的19控件分类及13个client-edge断点、无窗口标签边界已明；无产品改动；bb59d0a远端六文档通过，S11关闭。

- [T14 S3条件错误路由](../history/M0-T14-S3-console-errors.md)：27实际包/双发布/安装入口及47785d1实际远端复核通过；S3/T14关闭，M0开放。

- [T14 S2清理二次启动](../history/M0-T14-S2-cleanup.md)：实际包24/双发布/安装验证通过，f08c7da实际远端150输入/四产物及四删除路径通过，两登记scratch回收；S2/T14收口。

- [T14系统启动错误体验（已撤回）](../history/M0-T14-S1-system-shell.md)：24实际包/双发布和安装入口通过，91bba9f远端153输入/四产物、168文件复核通过，两登记scratch已回收；S1/T14收口。

- [T13 S2原样撤回](../history/M0-T13-S2-revert.md)：原样撤回、26聚焦/23实际包及双发布通过，bcf3d08实际远端复核通过，S2/T13收口。

- [T13多语言控件字体](../history/M0-T13-S1-multilingual-font.md)：83df08c远端150输入/四产物通过，71项68通过3跳过、实际包25/25、八发布摘要及真实扫雷通过；当时S1/T收口，后按用户要求由S2撤回。

- [Td S10英雄榜字体调查](../history/M0-Td-S10-leaderboard-font.md)：11cea15远端6文档/四产物通过，中文墨迹缩小已定位；无产品修复/发布。

- [T12缺失IAT启动修复](../history/M0-T12-S1-legacy-iat.md)：507847c实际远端139输入/四产物、8发布摘要及真实DoubleKiller通过；S1/T12收口，M0开放。

- [T11按需UAC](../history/M0-T11-S1-uac.md)：d77017f实际远端139输入/四产物及8发布摘要通过；S1/T11收口，M0开放。

- [T10取消研究](../history/M0-T10-S1-xaml-research.md)：用户取消，无产品支持发布；T8按用户明确要求收口。

- [Td S9构建与测试效率](../history/M0-Td-S9-test-efficiency.md)：425b7df实际远端复核通过；S9收口，M0/T8开放。

- [T9 S1原生嵌套cmd及外部导入表](../history/M0-T9-S1-native-cmd-staging.md)：P1实际远端复核及清理完成；仅本轮S/T9收口，T8开放。

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。

## Last Packet Closure

M0 Td S11纯调查完成：本机MadShow为x86 VB6，实际DLL/四个入口detour已生效、
主窗附着；4按钮/13文本框/2分组仍被标准类名单跳过，13个client-edge为第二门槛，
标签无独立HWND。有限按钮状态往返/原生截图及注册模块身份已查，未伪称已实现适配。
3有效批次/6自有实例均正常退出、Job0；P1 bb59d0a实际取回六份文档通过，154产品
输入及八部署摘要未变，无产品构建/发布。证据提炼后精确登记scratch已安全回收，
提升只读进程预检无存活探针，稳定Ninja/ccache保留；治理自测通过。
Td S11关闭，M0开放；产品T14等状态保持；该调查后续已按新用户请求进入当前T15，调查本身不回写为产品交付。
