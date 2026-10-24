# Project Status

## Current Work

无活动任务包。M0 T14 S3条件错误路由已完成27实际包、双发布、安装入口验证和
P1实际远端复核；S3/T14关闭，M0开放。共享/隐藏/无窗口控制台只输出一次193，
有窗口独占控制台只请求一次Windows系统错误UI。原外观/UAC/资源保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T14 S3 P2 (P1 47785d1实际远端167文件：154输入/四产物/八部署摘要通过；登记scratch回收)
- Open task: none；T14 S3/T14关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / T14 S3已验证/双发布/实际远端复核并收口；固定T14版本，由Git/SHA区分S3。
154输入冻结、GCC16.1/Ninja/ccache干净双宽构建，27/27实际包253.77秒通过，
双宽新单元/聚焦通过。MZ/NE系统Shell对照、独占/共享cmd/显式隐藏及无窗口共12路径；
系统UI一次且stderr空，其他一次193无UI，退出4，精确映像/内核Job归零。
S2无UI和S1双错误通道旧包均被检出；80份DLL/demo/UAC源逐字节保留。
两处八摘要一致，已安装启动器12路径及双宽ABI3/31通过；原图形/声明/18表保留。
73注册CTest未全部执行；物理Win+R/所有终端及历史两框来源不作已覆盖声明。
新控制台创建瞬间仍可能短暂闪现，错误UI由Windows持有。详见
[S3记录](../history/M0-T14-S3-console-errors.md)及[证据](../etc/evidence/console-error-routing.json)。

## Recent Closure

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

M0 T14 S3 Owner-Reopen完成：窗口化独占控制台的旧MZ/NE失败交Windows一次系统UI，
共享/显式隐藏/无窗口控制台保留一次stderr193，退出4；PE/UAC/外观及原资源保持。
154输入冻结/干净双宽构建，27实际包253.77秒通过；S1/S2旧版负例均检出，已安装
启动器12路径/双宽ABI通过。P1 47785d1实际读取远端167文件，154输入/四产物/八
部署摘要一致。所有登记scratch已提炼安全回收，稳定Ninja/ccache保留；受限预检
CIM不可用，原生Job已归零，回收后提升只读进程核查无登记路径存活，限制如实记录。
T14审计：同名sol解析差异已解释；S1无条件方案被撤回；S2清理后Win+R可见性由
S3条件分流补齐。S3/T14关闭，M0开放；其他状态/TODO不变。历史两个不同框来源、
物理Win+R及所有终端未冒称已覆盖，Console创建瞬间仍可能闪现。
