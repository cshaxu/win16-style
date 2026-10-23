# Project Status

## Current Work

无活动任务包。M0 T6 S5及T6已完成发布、推送、实际复核和scratch清理，
M0本次有界产品目标完成；milestone永久保持0。后续工作须由新请求准入。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 6
- Last allocated Td S: 5
- Last delivery: M0 T6 S5 P2 (P1 5b5ee58实际复核、清理及关闭)

## Technical Baseline

最终产品0.6.0.0 / M0 T6，纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja及
ccache4.13.6；静态GCC/线程支持，不使用C++或MSVC编译器、不依赖MSYS运行时。
Windows系统CRT imports见发布清单。三组件四产物：
x64 run16s.exe、x86 ntw16s32.dll、x64 ntw16s64.dll、x64 demo.exe。ABI3，31公开
名称ordinal2..32及ordinal1 NONAME helper；私有SDK staging/有界OS helper与
MinHook/HDE后端，无broker、驻留服务或兄弟仓库构建/链接/运行依赖。

原图形五头及原resources.md与固定来源5b0b65f逐字节相同，两DLL各18原图形表并
内嵌完整声明，禁止替换。124处置/294 includes、121回执及MinHook14清单通过。
原资源外观一致已由用户确认；滚动条17逻辑像素，端钮/固定滑块按横截面保持正方形，
绘制及命中测试共用几何，横纵17/28/35/42像素双宽探针通过，旧28像素实现负例失败。

启动器保持Console：控制台目标默认等待并返回实际退出码，GUI默认异步返回0，
--wait始终等待根目标。真实cmd/held目标六场景及stdio/退出码通过，同console身份
由GetConsoleProcessList验证。默认加载样式并传播到后代，demo本身仅Windows原生UI。

S4 418be5e最后产品/测试源码变更的完整矩阵注册/执行24/36一致，58通过/2隔离
clipboard权限跳过，0失败。S5新干净双宽全构建、ABI各1/1、实际包10/10通过；
124输入与S4相同，未虚称S5重跑完整矩阵。原生demo探针覆盖Tab焦点、业务数据/
通知、菜单/About、下拉、滚动捕获、移动及窗口状态，本机DPI144/150%，滚动条厚26像素。

用户指定不用computer-use、自己用探针做，不再要求整套手工操作。探针不冒称物理
键鼠；本宿主CreateWindowStation四种access mask均返回5，两个隔离clipboard用例
跳过77不计通过，共享剪贴板未触碰。支持范围及未验证边界见[BUILD](../etc/operations/BUILD.md)。

S5 P1 5b5ee58四文件在assets根目录、O:/winnt/system32及远端与被测包SHA一致，
124输入、六资源blob、原始测试/矩阵及30个历史交付提交已实际复核。每S编译测试，
binary change时按规范双发布、先备份及核验；当前无binary子目录。最终身份见
[release.json](../etc/release.json)，覆盖、失败处置与边界见[完成审计](../history/M0-T6-completion.md)。

## Recent Closure

- [T6及M0审计](../history/M0-T6-completion.md)：S5 P1 5b5ee58新构建/10项实际包/双发布及实际复核；T6/M0有界目标关闭。
- [S4修复与探针](../history/M0-T6-S4-acceptance.md)：418be5e修复方形滚动条并完成58通过/2权限跳过及同包10项，双发布/远端一致。
- [S3等待语义](../history/M0-T6-S3-wait.md)：4b83994保留Console并按目标默认等待，实际包14/14及相关回归6/9通过。
- [Td S5执行规范](../history/M0-Td-S5-publish-policy.md)：618c51b/815f115双位置发布、assets根目录及用户指定探针验收生效。

## Retained Assets

本任务build/t6-package已安全回收（40顶层目录、13003文件、521103134字节），
无活动owned任务；稳定build/.ninja及build/.ccache保留。原始日志/抓图未提交，
事实与摘要已提炼到history/release；历史包及部署before可从对应Git交付复取。
