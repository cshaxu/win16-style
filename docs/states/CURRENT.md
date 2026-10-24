# Project Status

## Current Work

无活动任务包。M0 T14 S1已交付真实Windows Shell旧程序错误体验，完成24实际包、
双发布、已安装入口及P1实际远端复核；S1/T14收口，M0开放。
原生PE/UAC、T13字体回退及其他任务状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T14 S1 P2 (P1 91bba9f远端153输入/四产物及八部署摘要通过；两登记scratch回收)
- Open task: none；T14关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / M0 T14 S1已验证/双发布及P1实际远端复核并收口，M0开放。
完整MZ头非PE的旧目标被校验拒绝时，让Windows Shell呈现原生错误UI，不自制对话框；
关闭后仍原始193/退出4。排除标准及重叠PE签名，普通PE注入和既有UAC/Console契约保持。
T13字体回退保持；71份DLL源除版本RC原字节保持，原图形资源/声明和ABI3/31不变。

153输入独立冻结/干净双宽构建；实际包24/24、两宽冻结新单元各1通过；注册72项未全运行。
最终系统UI对照和严格IOCP整Job清理通过，旧发布启动器被新测试检出；前轮观察器
即时计数失败已修订并重做最终包，不放宽断言、不关闭原TODO的T11独立债务。
八发布摘要一致，system32实际双宽ABI及MZ/NE系统UI验证通过。未计物理Win+R、
任意系统语言/版本、真实UAC同意操作和任意Shell关联。见[交付记录](../history/M0-T14-S1-system-shell.md)。

## Recent Closure

- [T14系统启动错误体验](../history/M0-T14-S1-system-shell.md)：24实际包/双发布和安装入口通过，91bba9f远端153输入/四产物、168文件复核通过，两登记scratch已回收；S1/T14收口。

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

M0 T14 S1完成Win+R/cmd同名NE/PE差异诊断及用户明确要求的真实系统错误体验。
153输入独立冻结，双宽干净构建、两宽新单元、24实际包及已部署系统UI/ABI通过；
旧版负例有效，IOCP整Job清理保持严格归零。P1 91bba9f已实际远端复核，八摘要一致。
两登记scratch已提炼回收。T14仅S1，全任务按本轮有限支持范围审计通过；
S1/T14关闭，M0开放；既有PE/UAC/样式/资源与其他任务/TODO状态保持。
