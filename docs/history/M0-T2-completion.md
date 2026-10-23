# M0 T2 — dual-width core completion audit

原请求为“收口并准入下一个t 开始执行”。T1 80eccb2 已关闭，下一候选按顺序
准入为 T2“双位宽样式核心”。本 T 的 S1/S2 已交付；S3 冻结以下六单位全集，
允许处置为本 T 有实际证明，或明确属于既定后续产品任务。本 T 内失败必须先
解决，不以转移掩盖。S3 未修改生产代码，复用无变更的完整回归，不重复耗时运行。

## Frozen task coverage

| 单位 | 实际交付与检查 | 完成谓词/状态 |
| --- | --- | --- |
| 已审核心与声明 | S1 20671cd；33 原源码/头、42 src 文件；S2 6243b15 加夹具，53 本地回执，24 retain/16 adapt/5 replace/8 authored | 本地摘要匹配；来源 124 处置、294 include、23 拒绝例通过；没有隐含来源依赖 |
| 五素材替代 | S1 自编几何生成器与五头，生成一致检查；S2 实际绘制及两宽图观察 | 原资源字节未复制；形状接口保留；不要求原 VGA 像素相等 |
| 本地双宽图与独立性 | 17 个显式 C，MSVC C17 Release /MT /W4 /WX；Ninja 两宽实际依赖复核，22 个本地头依赖，其余记录为 SDK/工具链 | 无 sibling 路径；过滤的系统头仍由已配置 MSVC/SDK 提供；没有服务/WOW/Detours/第二 renderer |
| ABI 与产物 | 两 DLL ABI 3、31 public、ordinal 2..32，1 未占用；无 private binding 或转发/额外 ordinal；版本 M0 T2/0.2.0.0 | 实际 PE 和 LoadLibrary 验证通过；直接 imports 仅六个 Windows 系统 DLL；未发布完整四文件包 |
| 核心运行有限支持面 | S2 固定 13 项 × 两架构，各 13/13；逐行契约见 S2 记录；核心 DLL、原生对照、纯数学单元分别报告 | frame/menu/六控件、原生状态/通知、线程、创建作用域、重入、资源与卸载路径通过；物理输入由包验收接收 |
| 交付/失败/卫生 | S1 与 S2 实际提交复核；S3 来源/治理/素材/PE 门禁、统计、scratch 回收与远端核验 | 本任务内失败均有下述处置；P1 推送及实际复核后由 P2 关闭 |

对应详细证据：[S1 导入/构建](M0-T2-S1-import.md)、[S2 运行矩阵与失败处置](M0-T2-S2-runtime.md)。
两宽 DLL 摘要：x86 4fa84d10e7253815f64418e0cf378a85373ec22f8159c21ae21458a80494bb9f，
x64 60e78dcb864a01466733b261b8ba4c19c5fbf1e358232fa000823709b2e20d79。
直接 imports 为 USER32/GDI32/COMCTL32/dwmapi/UxTheme/KERNEL32。

## Failure disposition and limits

S1 的 x86 export alias 失败及正/负退出码漏停已修复，双宽重建通过；相同错误
判断在两个后台工具均修正，并保留正负退出码回归。S2 的冷资源基线、busy
desktop 退出、max/min 恢复错误预期属于夹具问题：用直接资源所有权证明、
等负载预热及真实原生对照纠正，未修改生产 renderer 迁就断言。180 秒总预算
耗尽轮次不计完整通过；360 秒完整重跑成功。详细失败轮次留在 S1/S2 记录。

S3 加强 PE 检查为精确 ordinal/占用及无转发；对两宽实际二进制各构造五种
scratch 变体（占 helper ordinal、重复 ordinal、缺名、转发、machine/magic
矛盾），共 10/10 被实际校验路径拒绝，未改真实 DLL。治理 13、来源 23 负例
及五素材重生成一致检查均通过。

当前显式 API 只在窗口所属线程使用，消费者在窗口/作用域/字体/回调结束前
保持 DLL 加载。creation scope 的全部 root+9 自建 HWND 验证成功，另计
unsupported=1，不扩大为内部附带窗口均支持。三排除 exstyle 的拒绝路径有测试。
字体只证明宿主 System 对应 CP1252 字形的本地运行时生成；不发布字体像素，
不宣称 CP1252 外像素保真或宿主缺字/字体替代环境均已验证。

既定接收者：启动任务实现真实载荷、自动附着与私有 trampoline 绑定；传播
任务实现后代创建契约；demo 任务提供纯原生产品例；包验收负责真实桌面鼠标/
键盘/焦点/捕获、选定 DPI 与字体环境、菜单展开交互及干净无 sibling 环境重建。
本 T 私有桌面 WM_PRINT、96/144/192 的局部菜单绘制和 host 144 DPI 窗口
不是上述整机交互或任意应用兼容性证明；这些范围没有被删去。

## Statistics and ownership

统计命令 `git diff 80eccb2 --numstat -- src tests tools CMakeLists.txt`：
src 42 文件 +4324/-0（C/H 4216，其余为两 def、版本和说明）；tests 12 文件
+976/-0（C/H 951，其余说明）；CMake +77/-0；tools 6 文件 +296/-5，净 291。
合计 +5673/-5，净 +5668。其中大部分是已审来源导入，不能当作新增算法统计。
S2 新编三个 C/H 252 行，已审夹具适配 643 行。S3 PE 工具新增 19 行，未改产品。

T2 前没有本地产品路径，故没有旧本地实现待迁移删除。唯一 renderer 仍拥有
绘制/几何/字体/附着状态；原生控件拥有文本、选择、通知，应用拥有消息循环。
没有遗留的旧字号脚本、源资源头副本、私有 ABI 导出或第二 renderer。

## Closure hygiene

完整 worker PID 40364 已以 passed/exit 0 结束；其他已登记轮次均结束。只读进程
检查无本任务活动 worker/fixture。提炼证据后回收 build/t2-core（21,613,418 字节）、
两宽 CTest Testing 日志（12,558/14,222 字节）及本次 PE 研究导入产生的单个
Python 字节码（7,329 字节）。所有绝对路径先验证留在仓库、无 reparse；首个
一次性祖先检查停在未完成状态，已中断并改用有界类型检查后完成删除。
build 现只保留 .ninja 配置/对象/输出，未提交日志、图片或二进制。

来源正向/23 负例、治理正向/13 负例、素材一致、两个实际 PE、diff --check
均通过。S3 P1 交付后另做实际提交与远端一致性复核，P2 才记录 T2 关闭。
