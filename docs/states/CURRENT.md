# Project Status

## Current Work

无活动任务包。M0 T2“双位宽样式核心”已交付并完成全任务审计，S1/S2/S3 均关闭。
下一候选为启动器与首次自动附着，尚未准入，不预分配 T 号。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 2
- Last allocated Td S: 3
- Last delivery: M0 T2 S3 P2 (本次关闭记录)

## Technical Baseline

固定来源 5b0b65f66c281050aa4ce1454c6cd31441a6d484；124 处置、294 include。
本地 53 回执、唯一 renderer 的 17 C、五个项目自编几何头；MSVC C17 Release
/MT /W4 /WX 双宽图 build/.ninja/x86、x64。实际 ntw16s32.dll/ntw16s64.dll
ABI 3、31 public、ordinal 2..32，1 预留，私有 binding 不导出，版本 M0 T2。
S2 6243b15 的完整核心回归两宽各 13/13；来源 23/治理 13 负例及素材一致检查通过。

当前 DLL 为显式同线程 API 核心；DllMain 被动，没有载荷/helper/自动注入/传播。
启动器和产品 demo 尚未实现，也没有完整四文件发布包。打印图与私有桌面回归
不等于物理输入、任意 DPI/字体环境或任意应用兼容性通过。后续任务接收范围见
完成审计和候选。默认校验/构建/运行不读取 sibling。

## Recent Closure

[M0 T2 完成审计](../history/M0-T2-completion.md)：六单位覆盖、三 S 实际交付与
失败处置、双宽 PE/资源/来源核验通过。S3 P1 527f40c 已推送并实际复核。
本任务 scratch、CTest 日志及研究字节码已回收，build 只保留 .ninja。

[M0 T1 完成审计](../history/M0-T1-completion.md)：来源、资源、独立化设计及本地
校验已关闭于 80eccb2，其计划状态由当前基线取代。

[M0 Td S3 层级纠正](../history/M0-Td-S3-task-level.md)：T 级候选各自拥有 S，
队列不预分配 T 号；此前治理记录继续保留在 history。
