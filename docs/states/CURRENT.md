# Project Status

## Current Work

当前无活动任务包。本轮M0 T8 S1字体调研/修复已双发布、推送、实际复核及清理，
S1交付收口。按用户最新指令，T8保持开放等待字体外观反馈，不宣称整任务关闭。见[字体修复记录](../history/M0-T8-S1-font-layout.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S1 P2 (P1 980b496实际远端复核、清理及S1收口；T8开放)
- Open task: M0 T8 (S1 closed; owner appearance acceptance pending)

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

- [T8字体修复](../history/M0-T8-S1-font-layout.md)：980b496实际远端复核通过、S1已收口；T8仍开放待外观验收。
- [T7菜单取消](../history/M0-T7-menu-cancellation.md)：411ab67/101e92d交付收口，修复保留。
- [T6历史审计](../history/M0-T6-completion.md)：历史验收不能代替本次字体修复证据。

## Retained Assets

本轮build/t8-package已安全回收：10顶层目录、1342文件、68852600字节，
无reparse/活动owned进程。保留稳定build/.ninja及build/.ccache、两处发布。
原始证据已实际读取、提炼结果及SHA到history/release；旧before可从Git T7 P1恢复。
预览PNG保留本聊天visualizations目录，只供用户查看，不是第五产品或构建/运行
依赖，也不代表实际sol验收通过。


P1 980b49693c59b3b4947109990843ec71cfeb3237的实际远端代码/提交、126输入、
六原资源、四文件双发布、原始14项及完整覆盖/失败项复测来源已复核通过。
P2仅记录复核/清理及S1收口，不改产品输入或四文件，不虚构新构建/发布。
T8保持开放；后续根据用户外观反馈继续，不把本次S1收口当成T8全部验收通过。
