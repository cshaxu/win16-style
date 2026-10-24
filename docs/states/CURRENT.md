# Project Status

## Current Work

M0 T14 S3已验证并双发布，待P1提交推送/实际远端复核，不提前关闭。
已有共享/隐藏/无窗口控制台保留一次stderr；有窗口独占控制台走一次Windows系统UI。
最终27实际包及已安装入口通过；标题/字体/菜单/滚动条与UAC源保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T14 S2 P2 (P1 f08c7da远端150输入/四产物、四删除路径404及八部署摘要通过；两登记scratch回收)
- Open task: T14 S3；T14按用户重开；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## M0 T14 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen；使用下一S3，不复用S2；T14版本固定0.14.0.0 |
| Admission And Approval | 用户“行 你重开刚才那个s 进行这个改动”；永久提交推送及既定双发布授权 |
| Objective | 自动区分共享/无控制台与有窗口独占临时控制台；前者保留stderr193退出4，后者仅请求一次Windows原生MZ/NE错误UI，不重复报错 |
| Non-goals | 不自制弹窗/暂停；不改普通PE注入、UAC、renderer、资源、系统设置及兄弟仓库 |
| Reference Baseline | c302f45，工作树干净；S1系统UI原型与S2初次stderr/Job原生回归可复用 |
| Candidate Proposal | docs/proposals/run-dialog-launch.md；本S更新当前分流契约 |
| Files And ABI Surface | run16s私有错误路由模块、main、CMake、纯C原生测试及包入口；无公开ABI和第五产物 |
| Applicable Rules | EXECUTION/ARCHITECTURE/CODING/DOCUMENT及已读Task Reading Set；纯C17 GCC Ninja ccache |
| Verification | 原生私有桌面：MZ/NE各测试真实新控制台、共享cmd、无控制台重定向；对照直接Shell类/标题、UI次数、stderr、退出码、精确映像与整个Job归零；双宽单元及27项FullIntegration实际包 |
| Expected Markers | STARTUP ERRORS PASS；LEGACY REJECTION PASS；系统Shell对照UI恰好一次/无stderr，cmd一次193无UI，退出4 |
| Asset Needs | 原图形及声明全保持；同批四文件发布assets根和O:/winnt/system32，八摘要核对 |
| Reporting Requirements | 识别实测、负例、失败/跳过/边界、源码统计、双宽/包/发布/实际远端复核；可能仍有Console创建瞬间，不承诺消除系统控制台闪现 |
| Stop Conditions | 不能证明真实控制台分类或系统UI唯一性则不发布；权限失败保留证据，不扩大权限 |
| Exit Criteria | 探针和旧版负例通过；干净双宽/同批包/双发布/推送/实际远端复核；证据提炼后回收登记scratch |
| Original Owner Request | Win+R错误一闪而过、上版两个错误；自动判断环境，不要求用户判断；本S实现条件路由 |
| Similar-Issue Sweep | 搜索所有diagnostic/ShellExecute/193路径；限定旧MZ/NE分支，普通PE/740保持；scratch为build/t14-startup-errors、空的build/t14-s3-errors和build/t14-package（只含本S s3-*），退出前提炼安全清理，稳定.ninja/.ccache保留 |

## Technical Baseline

0.14.0.0 / T14 S3已验证并双发布；固定T14版本，由Git/SHA区分S3。
154输入冻结、GCC16.1/Ninja/ccache干净双宽构建，27/27实际包253.77秒通过，
双宽新单元/聚焦通过。MZ/NE系统Shell对照、独占/共享cmd/显式隐藏及无窗口共12路径；
系统UI一次且stderr空，其他一次193无UI，退出4，精确映像/内核Job归零。
S2无UI和S1双错误通道旧包均被检出；80份DLL/demo/UAC源逐字节保留。
两处八摘要一致，已安装启动器12路径及双宽ABI3/31通过；原图形/声明/18表保留。
73注册CTest未全部执行；物理Win+R/所有终端及历史两框来源不作已覆盖声明。
新控制台创建瞬间仍可能短暂闪现，错误UI由Windows持有。详见
[S3记录](../history/M0-T14-S3-console-errors.md)及[证据](../etc/evidence/console-error-routing.json)。

## Recent Closure

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

M0 T14 S2 Owner-Reopen完成：删除4个多余源/测试文件及入口，main恢复97999c0，
新增纯C首次拒绝回归。150输入干净双宽构建、24实际包/双发布和安装入口验证通过；
f08c7da实际读取远端166文件，150输入/四产物/八部署摘要及四删除路径404通过。
全部DLL/demo/UAC及发布规范保持；两登记scratch已提炼安全回收。
T14全任务审计：S1二次Shell方案被用户撤回，S2清理交付完成；S2/T14关闭、M0开放。
历史缺失输出原因未复现，不新增弹窗/暂停，不改其他任务或TODO状态。
