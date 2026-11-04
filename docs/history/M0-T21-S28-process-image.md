# M0 T21 S28：process-image

产品提交 `736f483` 将原 installer 的 PE 校验及进程位宽查询提取到 `src/lib-win32/process-image`，保留原 ntw16s 适配和语义。NTVDM64 的 SEC_IMAGE 查询行为及直接适配未验证，MZ/NE 判断仍由启动器处理。

`s28-clean1` 双宽干净构建、ABI/资源校验通过；干净树独立测试双宽通过，`s28-package1` 实包聚焦 13/13 无跳过。`s28-publish1` 同批四文件发布 assets 和 O:/winnt 并核对摘要；产品提交、四产物及组件源码远端对象复核一致。未运行完整 UI 矩阵。

SHA-256：run16s `2f2d0e70ca4b716d5accad51342f897e06bd1a1477e443163bc2191bb1d8f433`；ntw16s32 `7369080f80e65e4e3fc62f5be3dd0b890fa76763371dc4db7855de1c75c203b0`；ntw16s64 `f159891ccd09bfe979b6fe3650256966ed0ae4f1dcb92c5d86d6ae744bf08e81`；demo `44b5ed964b7a4a92f7947f6a75ff372b2d72361dc3fc99bcd15b61dc2c352358`。
