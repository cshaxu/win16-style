# Project Status

## Current Work

M0 T14 S1 active：调研并修复Win+R直接run16s sol启动失败、cmd内正常的差异。
准入工作树干净，先核对真实目标和启动环境；保留T13回退基线与所有关闭状态。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T13 S2 P2 (P1 bcf3d08远端149输入/四产物及八部署摘要通过；两登记scratch回收)
- Open task: T14 S1；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / M0 T14 S1已验证并双发布，P1实际远端复核待完成。
完整MZ头非PE的旧目标被校验拒绝时，让Windows Shell呈现原生错误UI，不自制对话框；
关闭后仍原始193/退出4。排除标准及重叠PE签名，普通PE注入和既有UAC/Console契约保持。
T13字体回退保持；71份DLL源除版本RC原字节保持，原图形资源/声明和ABI3/31不变。

153输入独立冻结/干净双宽构建；实际包24/24、两宽冻结新单元各1通过；注册72项未全运行。
最终系统UI对照和严格IOCP整Job清理通过，旧发布启动器被新测试检出；前轮观察器
即时计数失败已修订并重做最终包，不放宽断言、不关闭原TODO的T11独立债务。
八发布摘要一致，system32实际双宽ABI及MZ/NE系统UI验证通过。未计物理Win+R、
任意系统语言/版本、真实UAC同意操作和任意Shell关联。见[交付记录](../history/M0-T14-S1-system-shell.md)。

## Recent Closure

- [T14系统启动错误体验](../history/M0-T14-S1-system-shell.md)：24实际包/双发布和安装入口通过；P1实际远端复核待完成。

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

M0 T13 S2 Owner-Reopen完成：精确恢复8个原文件、删除1新增测试；149冻结输入，
双宽干净构建/26聚焦/23实际包、双发布8摘要、实际远端165文件均通过。
版本回退是用户原样revert要求的明确例外，治理T13不递减；没有新增字体方案。
发布占用规范保持，旧S1证据只作历史；登记scratch已提炼安全回收。
已审计T13全任务：S1产品/测试全部撤回，S2完成撤回交付；T13关闭，M0开放。

## M0 T14 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; 新故障请求，下一未使用T14/S1，M0固定 |
| Admission And Approval | 用户“下一个，故障：从Win+R直接运行run16s sol，无法启动”，补充cmd内正常；沿用提交推送与完整双发布授权 |
| Objective | 已定位cwd选中不同NE/PE；按用户最新要求让旧MZ/NE拒绝走真实系统Shell错误界面，不自制对话框；保留UAC和注入 |
| Non-goals | 不改外观/字体/资源，不改兄弟仓库或系统全局环境，不扫描注入既有进程 |
| Reference Baseline | 97999c0，准入工作树干净；产品0.12恢复源保持 |
| Candidate Proposal | [Win+R启动差异](../proposals/run-dialog-launch.md) |
| Files And ABI Surface | run16s新增私有legacy_shell.c/h，main拒绝分支及纯C shell-errors测试；CMake/版本0.14及实际包入口；无公开导出变化 |
| Applicable Rules | 完整Task Reading Set，纯C17/GCC/Ninja/ccache；无computer-use，实际自有实例验证 |
| Verification | Explorer环境只读与自有重放，cmd对照，绝对映像/PE/退出诊断；必要时新回归、双宽干净构建及实际包 |
| Expected Markers | SYSTEM SHELL PASS 2 formats、SHELL UNIT PASS classification=8；实际包24/24、8发布摘要及已部署系统UI一致 |
| Asset Needs | 不增加资源；若有binary change同批四文件双发布 |
| Reporting Requirements | 根因/最小方案先说明，测试入口/旧版检出/实际环境及未覆盖项，远端源码和产物实际复核 |
| Stop Conditions | 若需全局设置或越界兄弟仓库修改，停在证据方案；不得隐藏错误或静默选择不同目标 |
| Exit Criteria | 请求范围有证据解释并完成可行修复；源码/包验证、双发布、提交推送/实际复核及scratch提炼回收 |
| Original Owner Request | Win+R/cmd差异；追加“由系统显示系统的那个错误对话框”“原来的整个用户体验”“run16s尽量透明” |
| Similar-Issue Sweep | SearchPath/显式相对绝对路径/同名NE与PE、App Paths、cwd/PATH、创建安装及UAC重入 |

scratch：build/t14-launch（原生诊断/重放/聚焦证据），build/t14-package（独立源码/构建/测试/发布备份）；本S提炼后安全回收，稳定Ninja/ccache保留。
