# M0 T21 S27：同位宽纯载荷

产品提交 `2227486` 新增 `process_hook_stage_payload`：向新建挂起的同位宽目标暂存数据，不添加 DLL 导入。目标自身链接公共库，在系统加载器结束后调用 stage_read 和 stage_restore；恢复原导入信息并释放暂存内存。重复相同数据幂等，替换载荷会回收旧纯载荷分配；现有 DLL 安装路径保留。该格式不是 Detours GUID 载荷格式；跨位宽纯载荷、隔壁协议/调用适配及替换 Detours 均未验证。

`s27-clean1` 双宽干净构建、ABI 和资源身份通过；干净树自有子进程测试双宽通过，检查数据、原导入恢复、DLL 未加载、重复写入及释放。`s27-package1` 实际包聚焦 13/13、无跳过，包含旧 DLL 安装与真实 cmd 四条位宽传播边；未运行完整 UI 矩阵。`s27-publish1` 将同批四文件发布到 assets 根与 O:/winnt 并逐项核验，产品提交、四个远端 Git 对象及代表源码和探针已复核。

SHA-256：run16s.exe `cd2c8be0339c1fdaed55da413450e988387987f3b1306ebc745bbb723fc7bb09`；ntw16s32.dll `ec6ddb3bbc99549212f922722741b212d8ce861826e7cad37b35f4711710645a`；ntw16s64.dll `a4909a6afc9ca4ba0904eaf0ea9ca5a37f575f55b7bad186b98b776666789ca1`；demo.exe `211ab4674cdd6e0c45714f73988b437203da76f1f37c5e7f42d24b3889b6691c`。
