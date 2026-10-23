# Source Admission Policy

导入 ntvdm64 时从固定 Git commit 读取，不从有未提交改动的工作区复制。
导入前记录 source path、commit/blob、destination、保留/修改/排除理由、声明、
依赖、验证接收者；导入后用本地 manifest 对照。不要求用户保留来源仓库来运行
本项目。原始行为与独立化改动分别审查。

本仓库 MIT LICENSE 不是重新许可外部文件。保留 Detours 自带 LICENSE.md，
逐项审查其现有 helper 修改，不把 upstream 版本号当作本地零差异证明。
源 renderer 的 resources.md 明确记载 Windows 3.1 提取图形仅按本地实验准入、
公开再分发依据未建立。首个导入 S 必须清点所有此类 header/资源，逐项决定
排除、替换为可发布的项目自制素材、或记录足够的发布依据；不得直接把这些
字节连同源码/二进制推到目标仓库，也不能删除声明后称为项目原创。

本阶段只允许文件路径和 Git blob 元数据，不复制上述素材或第三方源码。
导入方案先完成来源差异和资源处置，再开展产品实现。该前置条件不影响本次
治理和准备交付，也不声称对任何外部资源作法律结论。
