# Project Status

## Current Work

M0 T8 S1字体调研/修复已verified并双发布，P1待提交推送与实际复核；完成复核和
owned scratch清理后收口S1。按用户最新指令，T8保持开放等待字体外观反馈，
不宣称整任务关闭。见[字体修复记录](../history/M0-T8-S1-font-layout.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S1 P1 (verified locally; remote review pending)
- Open task: M0 T8 (owner appearance acceptance pending)

## Technical Baseline

当前同批产品0.8.0.0 / M0 T8 S1，纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja、
ccache4.13.6；不用C++/MSVC/MSYS运行库。三组件四产物：x64 run16s.exe、
x86 ntw16s32.dll、x64 ntw16s64.dll、x64 demo.exe。ABI3，31名称ordinal2..32及
ordinal1 NONAME helper；无兄弟仓库构建、链接、运行或验收依赖。

标准控件精确保留应用原HFONT（含NULL默认字体）及后续WM_SETFONT，字重/大小/
文字宽度与原布局一致，不重排应用。title10pt粗体/menu8pt常规MS Sans Serif，
按窗口DPI换算一次，各由窗口拥有并释放；控件只借用app字体。显式20px公开
compat font API保留ABI，实际默认不再使用。应用自绘Score/Time等文字不干预。

五份原图形头及resources.md与固定来源5b0b65f逐字节一致，两DLL各18原表和完整
声明。滚动条17逻辑像素、端钮/固定滑块正方形；菜单取消后hover不重开，键盘
Escape层级保留。原资源图形未替换，原声明中的历史20px描述不是当前默认配置。
124来源处置/294 includes、123本地回执、MinHook14清单和负例门禁通过。

启动器保持Console：CUI默认等待并返回实际退出码、GUI默认异步0、--wait均等待；
样式和后代传播默认开启，demo本身仅原生UI。同包六场景真实cmd/held目标通过。

本S126输入，干净双宽全构建/ABI各1/1；之后仅修订menu-dispatch/default两个
非产品测试的固定预热，124其余输入不变，双宽只重建对应测试，四产物不重链且
SHA不变。原构建/最终测试快照均保留证据，不能虚称修订后又新建了一批四文件。

相关完整core+launch/demo覆盖：x86注册26，完整运行仅default失败，经唯一修订
后1/1复测替代，最终25通过/1权限跳过；x64最终完整运行29，28通过/1权限跳过。
合计53通过/2跳过，保留原失败JUnit及修后单项来源，不伪造新完整x86运行。
实际包14/14通过，含两宽各12字体组合/84控件场景、原生DS_SETFONT对话框、
font更新/NULL/卸载、frame刷新、GDI43/43与45/45；窗口实际DPI96/144，名义
192字号不是实际192-DPI显示器证明。demo裸/注入文字advance均41，实际包菜单
取消双宽各15场景及传播、等待、长路径、错误回滚通过。旧T7两个DLL均被负例拒绝。

用户指定原生探针、不用computer-use，不冒称物理输入。两个隔离clipboard用例
CreateWindowStation错误5/exit77跳过，不计通过、不碰共享剪贴板。原XP sol.exe
不在受控fixture中，用户实际Options/标题/菜单外观验收待反馈，T8因此保持开放。

同批四文件已发布assets根目录及O:/winnt/system32，8条before/after摘要一致。
身份、初始构建/测试修订边界及证据见[release.json](../etc/release.json)。旧发布
before与T7 P1 411ab6758517b307d44f4597dbc00fc07b925d44 assets同字节，可从Git恢复。

## Recent Closure

- [T8字体修复](../history/M0-T8-S1-font-layout.md)：本轮自动交付verified、已双发布，实际远端复核待完成；T8开放。
- [T7菜单取消](../history/M0-T7-menu-cancellation.md)：411ab67/101e92d交付收口，修复保留。
- [T6历史审计](../history/M0-T6-completion.md)：历史验收不能代替本次字体修复证据。

## Retained Assets

本轮owned scratch精确build/t8-package，实际复核后提炼证据并回收；保留稳定
build/.ninja及build/.ccache、两处发布。预览PNG位于本聊天visualizations目录，
只为用户查看，不是第五产品或构建/运行依赖。原始截图不代表实际sol验收通过。

## M0 T8 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective；T7关闭后字体/布局新缺陷，T8从S1开始 |
| Admission And Approval | 用户批准字体调整方案并要求实施、发布验收；沿用每S双发布、原资源不替换、不用computer-use、自己用原生探针 |
| Objective | 控件字体与原生布局一致，取消强制20像素粗体；标题/菜单独立DPI字体，保留程序自绘文字 |
| Non-goals | 不改sol.exe、不重排应用业务布局、不替换五原图形及声明、不全局改字体、不改传播/启动/公开ABI |
| Reference Baseline | 101e92dc03d52524c007652f983d27080e5c4a04；四文件T7/0.7.0.0，ABI3，用户截图为失败契约 |
| Candidate Proposal | [字体与布局兼容](../proposals/font-layout.md)，证据见[修复记录](../history/M0-T8-S1-font-layout.md)，S1发布后等待用户外观验收 |
| Files And ABI Surface | controls及frame字体所有权；保留既有公开font_acquire legacy API；字体/原生布局探针、旧假设测试、版本和构建/回执/清单/文档，ABI3不变 |
| Applicable Rules | Task Reading Set、新缺陷新T、原资源身份、有限覆盖与负例、每S编译同包测试双发布推送及实际复核 |
| Verification | 双宽实际DLL字体探针：原字体handle/度量/文本fit/控件rect、后续WM_SETFONT与移除、不同字号/字重及96/144/192度量；实际native dialog；旧包负例；双宽完整core及相关launch/demo、同包含菜单/字体测试 |
| Expected Markers | 附着前后字体与布局精确相同，应用后改字体生效、不裁切新增文字；标题/菜单随DPI且两者独立；GDI/窗口清理、菜单取消、启动传播正常 |
| Asset Needs | build/t8-package/probe-tree、focused等开发运行；release最终冻结快照及regression/regression-final/regression-verified/regression-complete及default-retest；negative旧包；package-test中断记录及package-test-final最终实际包；publish双位置before；hidden owned构建600秒/宽、包1800秒、scratch1GB；证据提炼后回收 |
| Reporting Requirements | 区分字体度量/DPI上下文与真实多显示器；只报告实际覆盖；原生探针不冒称物理操作；用户外观验收pending明示 |
| Stop Conditions | app字体/布局改变、引用寿命/GDI泄漏、原资源变、菜单或发布混包失败先修复；外部权限/占用不关闭用户进程 |
| Exit Criteria | S1新同批四文件构建/原生与包测试/双发布/提交推送/实际复核；自动证据通过、实际远端复核及cleanup后关闭本轮S1；T8按用户最新指令保持开放等待外观验收 |
| Original Owner Request | 字体字号不和谐，Options文字裁切/重叠而sol自绘Score/Time较小；批准调整并发布验收 |
| Similar-Issue Sweep | 所有font_acquire/release、WM_GETFONT/WM_SETFONT拦截、列表tab度量、combo/list/edit、菜单/title字体与DPI刷新/销毁、自动注入测试旧字体断言 |
