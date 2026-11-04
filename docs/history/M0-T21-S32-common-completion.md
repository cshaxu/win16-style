# M0 T21 S32：公共 Win32 机制收束

产品提交 `e0e9d11` 将原 MZ/非 PE 判断迁到 process-image；将新建挂起目标创建、调用方准备、恢复/失败清理和等待迁到 process-launch；将调用方参数化 Shell 执行/等待与句柄清理迁到 security，拒绝预先传入的借用进程句柄。module-import 独立并加 C ABI 头；原始启动、全局和 UAC 策略仍由产品负责。源码统计 22 文件、增加 244 行、删除 73 行（含文档及测试）。

双宽自有进程测试覆盖成功、准备失败、恢复失败和句柄处理；模拟 UAC 包括取消、拒绝、句柄错误和借用句柄负例。Windows SEC_IMAGE 映射与本项目 PE 校验对七种自造旧/无效文件双宽均返回 193；有效自身映像均接受，本项目额外检查预期机器。该有限样本不是 NTVDM64 的 NtQuerySection 实测，不能证明全部分类或错误码一致。

`s32-clean2` 双宽干净构建及 ABI、来源和原图资源门禁通过；干净树 process-image 与 process-lifecycle 双宽各 2/2；`s32-package2` 实包聚焦 23/23 无跳过，覆盖 CLI、失败、旧格式、模拟 UAC、控制台、导入表与真实四位宽传播；真实 UAC、完整 UI 矩阵和隔壁源码集成未验证。首次候选 23 项通过并阶段发布，后发现通用 Shell 需要保护借用句柄，补负例后重建、重测并以 `s32-publish2` 最终发布 assets 根与 O:/winnt，四文件与测试包一致。产品提交及四产物与代表源码的远端对象已核对。

SHA-256：run16s `ac1c94bebee01d977a8cfcdf13ea7a17e79b366abdebc2dce40271c6c4286963`；ntw16s32 `77b5014a74ee61403e11d63059da3ff9bba388183f0057a0262b9b5c88a7a66c`；ntw16s64 `b252036c384b5099625f6700cd31f5e84ff5fedc8f80dfa1a142072d58ee6d0c`；demo `c3ad80e8fa4075901ce742d529d39f7bfc148bc0a09b90c9c03e7dce60cc9830`。
