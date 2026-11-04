# M0 T21 S30：独立 module-import

用户反对将通用导入表机制放在 process-hook 下，产品提交 `94781d1` 将其作为独立 `src/lib-win32/module-import` 静态库，原 VB 适配保留目标和调用策略。双宽扫描和回滚测试通过；`s30-clean1` 双宽干净构建、ABI/资源检查通过；`s30-package1` 实包 14/14 无跳过，含双宽原调用方；`s30-publish1` 四文件发布两处并核对，远端四产物与库源码对象复核一致。隔壁未适配。

SHA-256：run16s `2a04bedb62868566952fd07392caac322b89c8afe0767e85f2df5cda97f2b8c3`；ntw16s32 `06c032f7d5464a66ff7a206ff2ed83c3f51cebf11e601b3b871151fc3ef3eb35`；ntw16s64 `63def011813d476f34a14b5896a9face24989062561564d9e57aac08b97aedac`；demo `d07daac93dd7eb55d8a1784d34d0cf2c8a896ac02b7e4211b004811778dec82a`。
