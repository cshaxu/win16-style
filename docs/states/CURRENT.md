# Project Status

## Current Work

无活动任务包（S）。M0 Td S9效率修复已完成实际远端复核、同包双发布及安全清理后收口；
M0固定，T8开放，T9产品任务仍关闭。产品版本0.9.0.0。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 9
- Last allocated Td S: 9
- Last delivery: M0 Td S9 P2 (P1 425b7df实际远端复核通过；清理及清理后增量通过，T8开放)
- Open task: M0 T8 remains open；无活动S；Td S9及T9已收口

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
[S9记录](../history/M0-Td-S9-test-efficiency.md)。P1 425b7df实际远端131输入/四产物/八发布副本复核通过；
本S两个scratch根及本轮Testing日志已安全回收，持久toolchain/cache保留，
清理后两宽直接Ninja均no work（0.51/0.35秒）。
本轮物理输入/真实192DPI/显式后端8例未重跑，不能借历史结果称为本S覆盖。

T9外部导入表修复源码保持；此前NE间歇失败仍未证实根因，既有裸NTSRV复用无
传播为用户接受边界。本次不改兄弟协议/源码，不注入或结束用户已有进程。
英文README及8张demo/真实XP Solitaire精选PNG保持，来源见[截图清单](../etc/screenshots/manifest.json)。

## Recent Closure

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
