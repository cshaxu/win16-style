# Project Status

## Current Work

M0 Td S9 implemented/verified：已完成效率修复、完整回归及同包双发布，待P1实际远端复核和清理。M0固定，T8开放，T9产品任务
仍关闭。修改工具与测试编排，不改运行时代码、版本、ABI、标题字体菜单或原资源。

## M0 Td S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；下一未用Td S9，不复用已关闭编号。 |
| Admission And Approval | 用户在效率审计后明确要求准入一个S修复测试效率；永久提交推送授权继续适用。 |
| Objective | 统一持久ccache、修复toolchain生命周期，支持目标/快速构建、隔离输出及有界并行，实测耗时与缓存收益。 |
| Non-goals | 不改测试断言/轮数/超时来制造提速，不减少完整集合，不改运行时代码/版本/ABI/UI/资源，不写softpc或ntvdm64，不动用户进程或全局缓存。 |
| Reference Baseline | 3245d15 / 0.9.0.0；审计确认临时缓存分裂、toolchain被清理、全目标构建、串行和重复验证。 |
| Candidate Proposal | [构建与测试效率](../proposals/build-test-efficiency.md)。 |
| Files And ABI Surface | CMakeLists、tools构建/打包/包测试/测试driver、测试入口和交付文档；产品C/资源/ABI不变。 |
| Applicable Rules | Task Reading Set、治理工具归Td；构建输入改变仍完成干净双宽四文件同包测试，binary change则双发布。纯C17/GCC/Ninja/ccache，参考仓库只读。 |
| Verification | 修前同集合串行基线、修后并行对照；直接增量no-work、toolchain在清理后存续、干净树共享缓存命中、目标构建与quick集合、并行目录/状态隔离、完整注册及同包回归。 |
| Expected Markers | QUICK PASS、CACHE REUSE PASS、INCREMENTAL PASS、完整JUnit/包标记、治理/来源/资源门禁及远端实际复核。 |
| Asset Needs | build/td-s9-efficiency为基线/bench原始日志，build/t9-package为同版本干净候选/包测试/备份；本轮Testing日志亦先提炼回收；稳定.ninja和.ccache保留，无reparse/活动进程才清理。 |
| Reporting Requirements | 有界耗时、对照集合/身份、cache增量计数、完整数量/skip、四SHA及必要发布、失败与未覆盖；公开数据脱敏，M0/T8开放。 |
| Stop Conditions | 资源/UI/ABI或断言改变、共享输出/竞态、完整测试失败则修正后复验；不以提高超时或删测试放行。 |
| Exit Criteria | 工具修复、可复跑入口、性能证据与完整验证、必要同包发布、提交推送实际复核和安全清理后收口S9。 |
| Original Owner Request | 准入一个S任务修复测试效率；此前要求对照隔壁softpc的Ninja、ccache及项目优化方案。 |
| Similar-Issue Sweep | 公共脚本/直接CMake/复制干净树的cache一致性，所有持久构建图对临时输入依赖、目标/测试分层、全CTest及包测试隔离和重复完整矩阵。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 9
- Last allocated Td S: 9
- Last delivery: M0 Td S9 P1 (verified locally；推送实际复核待完成，T8开放)
- Open task: M0 Td S9；M0 T8 remains open；T9已收口

## Technical Baseline

产品0.9.0.0 / M0 T9；Td S9仅修复构建/测试工具。纯C17、MinGW GCC16.1.0双宽、
Ninja1.13.2/ccache4.13.6；三组件四文件，x64 Console启动器/双宽DLL/x64 demo，
ABI3/31名称及ordinal1 NONAME helper不变。产品C/H/RC及六原资源与3245d15一致，
两DLL各18表；标题、System字体菜单、控件、SysShadow及应用自绘策略保持。

构建树使用持久.ninja/<arch>/toolchain.cmake；仅确认编译器变更才fresh，不再因
普通变量没有cache项清空对象目录。生成命令固定项目ccache，复制干净树显式
共享；不清全局缓存或用sloppiness。开发Quick仅构建所需目标，默认8编译jobs/
4测试slots、重UI占2；每例独有工作/捕获目录，原断言/轮数/超时及77保持。

热Quick6+8全通过，两宽Ninja no work，build0.38/0.34秒、test0.70/2.98秒。
同注册9项本机串行278.05→并行134.73秒，减少约51.54%，非同二进制微基准或
任意负载承诺。最终单份干净构建共享cache170 direct hits/0miss，build12.80/
15.30秒；131源码/测试/配置输入与冻结快照一致。该最终构建上的完整67项：
64通过、双宽clipboard权限5/77及未配置可选后端77共3跳过；实际四文件包19/19，
270.81秒。工具自测4例及silent exit0缺标记负例、DWORD退出保真通过。

已先备份再发布assets根及O:/winnt/system32，8摘要与被测包相同；8份before可从
3245d15恢复。PE/输入/工具链/性能及脱敏原始摘要见[release](../etc/release.json)和
[S9记录](../history/M0-Td-S9-test-efficiency.md)。P1尚待实际远端复核及安全清理，
本轮物理输入/真实192DPI/显式后端8例未重跑，不能借历史结果称为本S覆盖。

T9外部导入表修复源码保持；此前NE间歇失败仍未证实根因，既有裸NTSRV复用无
传播为用户接受边界。本次不改兄弟协议/源码，不注入或结束用户已有进程。
英文README及8张demo/真实XP Solitaire精选PNG保持，来源见[截图清单](../etc/screenshots/manifest.json)。

## Recent Closure

- [T9 S1原生嵌套cmd及外部导入表](../history/M0-T9-S1-native-cmd-staging.md)：P1实际远端复核及清理完成；仅本轮S/T9收口，T8开放。

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
