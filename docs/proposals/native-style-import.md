# 独立原生样式导入总览

本文件是六个独立 T 候选的共同来源与验收总览，不是将整个项目合成一个 T 的候选。
来源 commit 与文件指纹见 [inventory](../etc/import/README.md)。

## 已确认的边界差异

| 来源 | 本项目接收者 | 导入前处置 |
| --- | --- | --- |
| src/ntw16s-dll | ntw16s-dll | 逐文件保留单一 renderer、API/线程生命周期；先处置原始图形资源 |
| nthook32-dll/window_titles.cpp | ntw16s-dll 拦截层 | 合并到同一 DLL；移除动态加载独立 renderer 的双层结构，保留原 API trampoline |
| nthook32-dll/create_process.cpp | ntw16s-dll 传播层 | 提取 A/W 原生创建传播；删除 DOS/Win16 路由、broker 观察和 RPC 上下文依赖 |
| nthook32-dll/installer.cpp、context.cpp、entry.cpp、detours/ | ntw16s-dll 私有注入实现 | 收敛复制载荷；审查跨位宽 helper、失败回滚及 loader-lock，引入必要 license |
| run16-exe/main.c、launch_options.c 等 | run16s-exe | 仅取命令/原生启动所需部分；不携带 NTSRV、VDM、PIF 或字符前端职责 |
| w16style-exe/main.c、gallery.c | demo-exe | 现有入口检查 DLL ABI，gallery 主动调用样式；改为纯原生示例，不能直接复制冒充合格 demo |
| tests/app 的 style 系列、ntw16s 测试 | tests/ntw16s-dll | 保留可复验契约；清除旧仓库路径、历史编号和失效注册 |
| tests/component-integration、observation 的 Hook 链 | tests/integration | 改为本地产品包测试，不复制源服务与 Win16 guest 验收体系 |
| tools/build/New-Ntw16sNinja.ps1、New-NativeHook64Ninja.ps1、根 CMake | tools / 根构建 | 仅作参考，建立稳定本地双架构图；禁止第五产品和源树外依赖 |

## 准备阶段有限覆盖清单

| 单位 | 本次处置/证明 | 后续接收者 |
| --- | --- | --- |
| 治理权威 | docs 六目录、固定文件集、MTSP、单活动包 | 治理 gate |
| 产品集合 | 三组件四输出、PE 架构机器映射 | 产品包 gate |
| 来源冻结 | 固定 commit + blob inventory；不复制脏工作区 | 来源冻结任务 |
| 独立边界 | 上表逐家族列出责任迁移 | 各实现任务 |
| 资源与声明 | source-policy + 资源待处置；当前仅元数据 | 来源冻结任务，逐文件处置后才能复制 |
| 验收面 | 下表明确必须覆盖和证据局限 | 独立包验收任务 |
| 提交发布 | 治理正/负检查、实际 diff、推送对照 | Td 交付/关闭记录 |

准备完成谓词：以上七单位均有当前文件或交付记录，不能将“待实现”解释为
已实现。所有产品接受项当前均 not run，不引用旧项目通过数字作为本项目证据。

## T 候选与局部 S

每个任务独立准入、交付和关闭，各自从 S1 开始。正式 T 号在准入时递增分配；
M0 固定不变。队列顺序以[QUEUE](../states/QUEUE.md)为准。前次单 T 六 S 方案已被用户纠正，当前方案取代它。

| 独立 T 候选 | 本任务候选 S 分解 |
| --- | --- |
| [来源冻结与独立化设计](source-baseline.md) | 逐文件来源、依赖闭包及资源处置；ABI、初始化与跨位宽设计及工具链验证；本地来源 manifest/校验工具、设计交接和本任务审计 |
| [双位宽样式核心](style-core.md) | 本地双架构构建与已审源码/资源导入；真实 DLL 附着、外观与线程/资源生命周期复验；双位宽完整核心回归、ABI/来源核验及本任务审计 |
| [启动器与首次自动附着](launcher-injection.md) | x64 CLI、目标识别与唯一注入事务；DLL 引导及窗口自动附着、双位宽目标验证；失败回滚、核心回归与本任务审计 |
| [子孙传播与创建语义](descendant-propagation.md) | A/W 创建拦截与后代上下文传播；四种位宽边、第三代及创建语义/回滚验证；首次启动和核心回归、依赖清理与本任务审计 |
| [纯原生 demo 与启动对照](native-demo.md) | 纯原生 x64 demo 与本地构建；同一文件裸启动/注入对照及行为验收；ABI/import 边界、已有回归与本任务审计 |
| [独立包与 M0 产品验收](package-acceptance.md) | 四文件同批打包、PE/哈希/声明和独立重建；完整自动矩阵及真实桌面交互验收；本任务审计、跨任务覆盖及 M0 产品完成审计 |

各任务随实现交付其必要测试，包验收负责组合复验。相关任务须独立审计目标、各 S 实际交付及未解决项；
最后一个任务另外做跨任务覆盖和 M0 产品完成审计，不追溯重开已经满足独立完成标准的任务。

## 需求覆盖及接收任务

| 需求 | 实现接收者 | 最终复验 |
| --- | --- | --- |
| 来源/资源/依赖与初始化设计 | 来源冻结；后续任务实施 | 独立包验收 |
| DLL 外观、字体/控件和线程资源 | 样式核心；首次自动附着 | 独立包验收 |
| x64 启动器支持 x86/x64 | 首次自动附着 | 独立包验收 |
| 子孙传播与原生创建语义 | 子孙传播 | 独立包验收 |
| x64 demo 两种启动行为 | 原生 demo | 独立包验收 |
| 三组件四产物、构建运行独立及整体交付 | 各任务按边界交付 | 独立包验收及 M0 审计 |

## 验收矩阵（planned，全部尚未运行）

| 面 | 必须证明 |
| --- | --- |
| 构建/包 | x64 run16s、demo、ntw16s64；x86 ntw16s32；PE 架构、导出/导入、哈希、仅四产品；移开源仓库仍可构建/运行 |
| 首次注入 | run16s x64 -> native x86、x64；缺 DLL、错位宽、拒绝访问、无效 PE 的具体失败 |
| 后代 | x86->x86、x86->x64、x64->x86、x64->x64；再到第三代；每代同位宽 DLL 且真实窗口附着 |
| 创建语义 | A/W、空应用字段、空格/Unicode/长路径引用、环境、cwd、句柄继承、STARTUPINFO/EX、CREATE_SUSPENDED、失败 LastError 与所有者回滚 |
| 窗口生命周期 | 新线程窗口、动态控件、创建重入、窗口销毁、DLL 引用、资源释放、失败附着；不依赖标题前缀判断成功 |
| 外观/输入 | frame、menu、button/static/edit/list/combo/scroll；鼠标/键盘/焦点/捕获、最小化/最大化、DPI；不支持项逐条处置 |
| demo 对照 | 同一 demo 直接启动原生、run16s 启动 Win16；静态 import 无样式 DLL/API；运行模块与实际绘制对照 |
| 独立性 | 无 ntvdm/WOW/broker/RPC 路由、无额外 hook DLL、无系统安装和全局主题修改 |

测试 fixture 可构建两种位宽，不作为产品包成员。实测不支持的 API/变体显式
记录能力边界；不得用 renderer 的定点单测代替透明应用集成验收。
