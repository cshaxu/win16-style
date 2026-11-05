# M0 T21 S42：产品全局适配清理

S42 收口，M0 固定，T21 保持开放。按用户要求审视目录、文件、结构、函数和变量；逐项职责与保留原因见[全局进程审计](../proposals/global-process-audit.md)。

- 目录和文件：产品专属适配集中到 `src/ntw16s-dll/global/`；删除纯发现转发的两个文件和单独加载包装的两个文件，将必要 DLL/导出配置并入 remote。原 27 个 global_ 文件变为目录内 23 个文件，含 12 个 C 实现和 11 个头；这是保留产品职责的适配集合，不宣称产品侧零全局逻辑。
- 结构和函数：发现、worker ready/close、monitor stop/event、policy init/revalidate、加载释放及系统 loader 解析等纯转发删除，调用者直接使用公共库；产品回调保留会话、目标策略和样式职责。
- 变量和类型：删除公共类型和身份函数的产品别名、无用 include/常量、管理器仅启动时使用的桌面字段和仅单次使用的截止时间字段。

P1 `7301e504` 为产品提交，68 个文件变更，新增 431 行、删除 486 行（统计包含文档、测试和重命名，二进制行数不计）。`s42-clean1` 同快照 GCC、Ninja、ccache 双宽干净构建、ABI、来源、原资源和公共库反向依赖检查通过；干净树双宽各 11 项核心测试通过。`s42-package1` 实际包聚焦 44 项通过、无顶层跳过，但两个其他 window station 子项因 exit 12 跳过，不计通过。真实管理员 ETW 和托盘端到端未在此证明；停止后 Button 绘制摘要与旧版相同但不等于原始值。

首次 `s42-publish1` 因 `O:/winnt/ntw16s32.dll` 被 `D:/programs/misc/sol.exe` PID 4120 持有而中断并短暂混包；核对映像、启动时间及模块后正常关闭，`s42-publish2` 用同一已测试候选包完成 `assets/` 和 `O:/winnt` 的四文件发布及摘要核对。P1 已推送并逐项复核实际远端树中 88 个增删改路径。

四文件 SHA-256：

- `run16s.exe`：`da3c9c172daf206e9b8f4d7b87ba4f712794494e17ed7a23ba8fe7f98dcec6e4`
- `ntw16s32.dll`：`a508dcd18f34e51aa4aca29da733f50231bff23148ba24bc416d2d8c427a078d`
- `ntw16s64.dll`：`a9f7ef4b4779570ab56c2fa0dbf869303e3f7c649ffe4a7fae5b98b3d0b9597f`
- `demo.exe`：`6c0fe2b012c0b19484cd1105362ff43368a432c61f294387f8fe6942e4606a52`
