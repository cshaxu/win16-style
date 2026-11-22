# M0 T29 S15 主题驱动迁移基线

冻结点为 S14 P1 5263e650 和收口记录 22c72486。范围以 CMake 的三件正式
运行文件为准，独立旧 DLL、demo、测试及主题构建工具另列。沿用 S12 已冻结
113 单元，S13 的目录／CLI／全局选择，S14 的混搭对话框策略作为新增验收轴。

## 实际构建图

从 x86/x64 Ninja 树使用 ninja -t inputs ntwshk 提取去重后的 src 对象输入。
两个运行时各 289 个 src 单元，包含 win3x 外观 50、win2000 外观 29、主题读取
7、两套历史产品目录 28、中性 DLL 装配 3。该数包含静态归档的构建输入，不等同
PE 中保留下来的函数数目。按 UTF-8 相对路径排序、每项以 LF 结尾的摘要：

| 构建目标 | 数量 | SHA-256 |
| --- | --- | --- |
| x86 ntwshk | 289 | 5823e8befeff81d3f1bc7b11558023f3ad91c067cb01889418569303e527d4bd |
| x64 ntwshk | 289 | c0eb85f1811f0f403cbf900dff0660e3dc8acc5ea51860987df32d861862516c |
| x64 run16s 传递闭包 | 305 | 4c78778c86e69fc17f8862421acb495e6f68d045f5679b1d568766dcd2c6aa1c |

run16s.exe 的直接链接通过 ninja -t query run16s.exe 核对，有 15 个项目内
直接对象及公共归档，年代绘制器经 ntwshk64 DLL 实现。终审须对 PE 实际符号、
资源与 CMake/Ninja 对象和归档证据交叉核验。

## 迁移接收者

| 现存依赖 | 当前入口与事实 | 接收者 |
| --- | --- | --- |
| 托盘目录、命令及名称 | [tray_controller](../../src/run16s-exe/tray_controller.c) 先用 Win16／Win2K 固定项分派，其余按目录显示。 | S15，从校验成功的主题描述取名称并按目录分配菜单项。 |
| 启动器目标与同一入口 | [main](../../src/run16s-exe/main.c)、[UWP 请求](../../src/ntw16s-dll/uwp/target.c) 按年代分流启动及已支持宿主。 | S16，主题引用与宿主能力分别建模。 |
| 主题身份、旧主题识别与跨进程载荷 | [classic_theme](../../src/lib-win32-style/appearance/msstyles/classic_theme.c) 的预设目录和角色猜测，profile 低位与 [startup context](../../src/lib-win32-style/access/startup/context.c) 年代标志。 | S16 拆分身份和能力，S23 移除运行时旧格式识别。 |
| DLL 普通启动与全局 | [bootstrap](../../src/ntwshk-dll/dllmain.c)、[全局窗口](../../src/ntw16s-dll/global/window.c) 用不同 provider 分派与切换。 | S16 迁接入和身份，S17/S18 消费通用布局，S23 退出旧 provider。 |
| 标题栏、外框、原生菜单按钮 | [Win3.x](../../src/lib-win32-style/application/window/client_frame_renderer.c)、[Win2000](../../src/lib-win32-style/application/window/native_caption.c) 绘制、边框与回退常量。 | S17，共享布局、命中和渲染原语。 |
| UWP/WinUI 外框 | [宿主 DLL 入口](../../src/ntw16s-dll/uwp/hook.c) 按 profile 低位区分 surface 应用。 | S18 使用 S17 描述，保留宿主本身的合成与标题输入。 |
| 菜单及字体 | [Win3.x 菜单](../../src/lib-win32-style/application/menu/client_bar.c) 与 [公用数据读取](../../src/lib-win32-style/appearance/msstyles/classic_theme.c) 的时代配方。 | S19，迁菜单度量和字体行为声明。 |
| 标准、复合及框架控件 | [Win3.x 控件](../../src/lib-win32-style/appearance/win3x/control/control_install.c)、[Win2000 控件](../../src/lib-win32-style/application/control/observed/control.c) 和产品状态桥接。 | S20 基础控件，S21 复合和适配角色。 |
| 公共对话框 | [文件／目录策略](../../src/ntw16s-dll/injection/file_dialogs.c)、[Win2000 打印模板](../../src/lib-win32-style/application/dialog/print_general.c)；各类路由已外置，仍依赖年代路径及回退。 | S22 迁到布局及原生行为的通用选择。 |
| 内置配方、链接和 ABI | [正式链接配置](../../CMakeLists.txt) 静态链接年代实现，历史命名导出仍由中性 DLL 提供。 | S23 实际移除专属绘制和内置时代逻辑，核实独立工具与 ABI 入口。 |
| 已验收行为、范围与债务 | [S12 终审](M0-T29-S12-final-audit.md)、[S13](M0-T29-S13-theme-folder-cli.md)、[S14](M0-T29-S14-dialog-policy.md)。 | 每个 S 对照已交付能力，S24 固定二进制完整复测。 |


## S15 开发验证

托盘从已映射主题取得 DisplayName，目录名保留可区分性。菜单排序及命令编号
以目录清单为依据。DisplayName 缺少、间接引用、重复字段、非法控制字符或
不是有效 UTF-8 时沿用目录。原生菜单中的 & 按字面量处理。
两宽增量构建通过描述文件负例和菜单控制器。实际目录探针在两宽分别识别
五份合法目录，包含两份同名主题和一份未命名主题，并排除缺件目录。
自有托盘夹具验证两份标准菜单与增加自定义主题的目录列表，均以闲置状态启动并正常退出。


## P1 正式交付

MinGW GCC、Ninja、ccache 从同一个干净快照完成双位宽构建与七件打包。读取门禁、
PE/ABI、原图及来源检查均通过。先发布 assets 和 O:/winnt，再执行实包测试。
四份主题的 SHA-256 与 S14 完全相同。正式 EXE 不含先前两套固定菜单标签。

15 项实包聚焦测试全部通过，无跳过。两宽 descriptor、菜单控制器和实际目录探针
通过。自有闲置托盘夹具读取标准和自定义菜单并验证退出。物理键鼠与管理员真实
托盘未运行。其余按年代的启动、宿主、绘制及对话框路径按上表继续迁移。
完整摘要及原生探针标记见 [发布清单](../etc/release.json)。


## P2 复核与收口

从远端 P1 32ca7ac0 实际下载核对七件文件，两处发布与远端逐件吻合。
另查九个退役路径不存在；1282 个源码输入复核，八处仅换行规范化。
四个临时构建、实包验证和备份目录已清理，共 462,129,327 字节。保留常用
Ninja 增量树及 ccache。S15 按已验证范围收口，T29 和全主题驱动目标保持开放。
