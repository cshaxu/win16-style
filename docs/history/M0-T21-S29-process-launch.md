# M0 T21 S29：process-launch 首项

产品提交 `6d13e80` 将原始命令行尾部解析提取为独立静态库，进程创建和等待策略未迁移。首次干净构建因 tray-fixture 漏链接失败；补齐后 `s29-clean2` 双宽干净构建及 ABI 通过，独立解析双宽通过，`s29-package1` 实包 13/13 无跳过。`s29-publish1` 同批四文件发布 assets 根及 O:/winnt 并核对，远端产品与代表源码对象复核一致。完整启动事务和隔壁适配未验证。

SHA-256：run16s `52eeda35fc18cbfa81def8912a90530041aa9078991e8a2aab6017a33f312b9e`；ntw16s32 `6eaf27b9fb9cd7892ca944f4bc5bae6279e8b6977d09c3bfc1385b27a3eea5c4`；ntw16s64 `e42968afccb889e9b3014039787ec39213dfb914fef8dba7cd5ccdf08b2d9201`；demo `82a9fa643993880b88ea59616fa5218121ba214f0905f63797ba48699c3d22da`。
