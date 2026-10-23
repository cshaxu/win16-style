# Project Status

## Current Work

当前无活动任务包。M0 T7 S1菜单取消修复已双发布、推送并实际复核，T7六单元审计
通过，S1/T7已关闭，永久M0。用户原始缺陷与有限审计见[菜单修复](../history/M0-T7-menu-cancellation.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 7
- Last allocated Td S: 5
- Last delivery: M0 T7 S1 P2 (P1 411ab67实际复核、清理及关闭)

## Technical Baseline

当前同批产品0.7.0.0 / M0 T7，纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja及
ccache4.13.6；不使用C++或MSVC、不依赖MSYS运行时。三组件四产物：x64 run16s.exe、
x86 ntw16s32.dll、x64 ntw16s64.dll、x64 demo.exe。ABI3，31公开名称ordinal2..32
及ordinal1 NONAME helper；无兄弟仓库构建、链接、运行或验收依赖。

鼠标切换菜单后取消会清理残留键盘模式，焦点/capture/菜单项状态恢复，取消后
hover不重开；保留键盘Escape层级、子菜单、system-menu及命令行为。
新探针双宽各15场景通过；两个实际旧T6 DLL在case0被拒绝。状态仍归GUI线程，
tracking_menu栈重入及calls/detached生命期保留。

五个原图形头及原resources.md与固定来源5b0b65f逐字节相同，两DLL各18原表及完整
声明，不替换图形。滚动条17逻辑像素，端钮/固定滑块按实际横截面保持正方形。
124来源处置/294 includes、122本地回执及MinHook14清单通过。原资源外观用户此前已确认。

启动器保持Console：控制台目标默认等待并返回实际退出码，GUI默认异步返回0，
--wait均等待；同包六场景真实cmd/held目标通过。样式和后代传播默认启用，demo
本身仅原生UI。当前原生demo探针直接/注入均通过，本机DPI144/150%、scrollbar厚26物理像素。

本S125输入冻结，干净双宽全构建/ABI各1项通过。相同树未重建的相关回归注册/执行
25/28一致：24+27通过、两个隔离clipboard权限跳过、0失败；覆盖完整core及相关
launch/demo。实际候选包12/12通过，含两宽菜单取消各15场景及传播/等待/错误回滚。
本次未重跑完整propagate contract/dialog/messages矩阵，T6历史完整矩阵58通过/2
跳过只作历史证据，不冒称T7全局矩阵。

用户指定不用computer-use，自己用原生探针；不再要求整套手工操作。原生队列探针
不冒称物理键鼠；CreateWindowStation错误5导致两个隔离clipboard用例跳过77，
不计通过、不触碰共享剪贴板。支持范围及未验证边界见[BUILD](../etc/operations/BUILD.md)。

被测四文件已发布assets根目录及O:/winnt/system32，8条before/after摘要核对一致。
身份、证据摘要及备份定位见[release.json](../etc/release.json)。原始T6发布可从
5b5ee58376d3ff9a98e5852cb2c3c5247e202e02 assets复取。

## Recent Closure

- [T7菜单修复审计](../history/M0-T7-menu-cancellation.md)：411ab67双发布/远端实际复核通过，清理及收口完成。
- [T6及M0历史审计](../history/M0-T6-completion.md)：S5 P1 5b5ee58/P2 89fe6b9关闭，当时未覆盖本次popup取消缺陷。
- [滚动条修复](../history/M0-T6-S4-acceptance.md)：原资源和方形几何保留。
- [执行规范](../history/M0-Td-S5-publish-policy.md)：每S同包测试、assets根目录和system32双发布、自己用探针。

## Retained Assets

本任务build/t7-package已安全回收：10顶层目录、2367文件、88498140字节，
无reparse或活动owned进程。稳定build/.ninja及build/.ccache和两处发布保留。
原始证据SHA/结果已提炼到release/history；T6/S1旧发布备份可从Git 5b5ee58 assets恢复。


P1 411ab6758517b307d44f4597dbc00fc07b925d44实际远端代码/提交、125输入、六资源
blob、四文件远端/双位置身份及原始包测试/JUnit证据已复核通过。P2仅记录复核
与清理；不改产品输入或二进制，不虚构新构建/发布。
