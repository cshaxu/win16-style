# Project Status

## Current Work

M0 T9 S1已交付并按P1实际远端复核/安全清理收口；本P2只记录关闭，无活动任务包。
T9有限范围审计通过。M0固定，T8保持开放；此前NE间歇失败未证实根因，既有
裸NTSRV复用无传播仍为用户接受边界，不追写为本次已修复。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 9
- Last allocated Td S: 8
- Last delivery: M0 T9 S1 P2 (P1 1efa924实际远端复核通过；仅文档收口，T8开放)
- Open task: M0 T8 remains open；T9及S1已收口，无活动S

## Technical Baseline

0.9.0.0 / M0 T9 S1，纯C17、MinGW GCC16.1.0双宽、Ninja1.13.2/ccache4.13.6。
三组件四文件、x64 Console启动器/双宽DLL/x64 demo；ABI3、31名称及ordinal1
NONAME helper不变，无C++/MSVC或兄弟必需依赖。唯一运行时代码改动为staging
允许有效可读外部导入表，保留溢出、终止、权限、挂起和回滚契约。

标题、System字体/菜单、控件、局部SysShadow及应用自绘策略保持T8 S4代码；
六原资源及声明逐字节保留，两DLL各18表。96/144DPI实际包字体和15菜单场景
均通过；物理输入、真实192DPI及本S新包NE冷/热复用未覆盖，不借历史结果。

独立干净双宽构建及ABI各1/1，128源码/测试输入与冻结快照一致；同批四文件
实际包19/19，包括完整创建/回滚/dialog/message集成。注册全集66项：63通过、
两宽clipboard权限5/77及默认可选后端77共3跳过；最终观测/清理修订另以聚焦和
实际包复验，不冒称所有结果来自一次构建。真实cmd仅ntw必需4边、nthook/双钩子
可选各4边均通过，独有子标记、37/23退出及内外job归零。双宽有效外部表真实
初始化；NOACCESS299、未终止/RVA越界193。旧生产源双宽和旧包均能检出缺陷。

四文件已先备份后发布assets根及O:/winnt/system32，8摘要与实际测试包一致；
8份before与Gite6d6b55 assets一致可恢复。版本、PE/imports、输入和脱敏证据见
[release.json](../etc/release.json)与[S1记录](../history/M0-T9-S1-native-cmd-staging.md)。
P1 1efa924abc7d13f4f0c04294512399edd4f18f2c已实际远端复核：128输入、四远端/八发布、
六资源、59份原始摘要及18份cmd记录身份核对通过。两个登记scratch已安全回收
6605文件/225749124字节，另6份Testing日志先提炼再回收；无reparse/活动进程，
稳定.ninja/.ccache及CTestCostData、两处发布保持。P2仅文档，128输入/四文件不变。

英文README及精选8张demo/真实XP Solitaire PNG原样保留，其展示与来源记录见
[截图清单](../etc/screenshots/manifest.json)，不扩写成当前新包的游戏验收。

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
