# M0 T1 S3 本地清单校验与交接

用户整个 T1 授权下继续；S2 9cf7183 已推送并实际提交复核后准入。
状态 closed，最终交付推送后已完成实际 commit 复核。

交付 tools/verify_source_manifest.py，Python 3.10+ 标准库，默认只读本仓库：
124 条去留、294 include、105 seed 对应、5 原资源 replace、ABI 与路径/声明/
回执/原字节 SHA-256 一致性。没有导入时 local imports=0；后续真实文件必须
有回执，retain 必须原字节，adapt/replace 需 review，不允许以新编名义绕过
受限资源目的路径。Detours 导入须有原 LICENSE。工具不自动生成或更新回执。
操作和局限见 import/VALIDATION.md；CI 增加同一默认检查，不依赖源仓库。

## 验证与缺陷处置

默认自检通过：23 个内存负例 + 1 个合法自编替代回执，未把 fixture 写成产品。
另以可选 --source-repository 研究复核，124/124 固定 tree/blob/size/SHA-256
通过；该选项不属于构建/产品验收所需依赖。治理正向/13 负例和空白检查通过。

首次执行揭示 ABI “34 声明 == 34 独立导出”的错误假设。固定 attachment.h 与
theme.h 都声明同一个 Ntw16sSetWindowTheme，签名相同：33 个不同 symbol，
31 保留 public + 2 转 private，另加 helper。工具保留两处来源声明并要求一致，
不是删除来源记录以凑数；冲突声明有永久负例。S2 的 32 public 指声明条数，
不同 public symbol 计数以此纠正为准。

相似问题搜索覆盖源/目的路径（含大小写别名）、重复条目、blob/size/SHA、
include 外部边界、ABI 声明冲突、受限资源/声明与回执内容。各类均有拒绝用例。
工具是本地清单校验唯一实现，未新增产品运行状态或替代 renderer。
最终代码行数及提交复核在 T1 审计中记录；未运行/声称产品构建、Detours 注入或 UI。

P1 d10aeb9 已推送；实际提交复核 9 文件 +435/-19，新增 Python 工具 +266/-0。
git show --check 通过，S3 目标与交付一致，无阻塞；S3 关闭。
两精确 scratch 子目录已经在校验后回收，build/ 为空，未终止任何非本任务进程。
P2 记录 T1 级完成审计与空闲状态，不准入后续实现。
