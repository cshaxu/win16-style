# M0 T21 S39：移除未使用的 file 公共组件

按用户要求删除 S38 导入的 file 四个源文件、独立测试和上游参考测试，移除 win32-file/core-file 构建目标及四条来源回执；现有文件内容校验和其他组件保持。`rg "win32-file|core-file|lib-win32/file" src tests CMakeLists.txt` 无命中。S38 来源和结果保留历史记录。

s39-clean1 使用 GCC 16.1.0、Ninja、ccache 4.13.6 双宽干净构建，ABI、来源和原图资源门禁通过；s39-package 同批实际包聚焦 13/13、无跳过，含 CLI、嵌套 cmd、传播基线；完整 UI、真实 UAC 未测。s39-publish 已发布至 assets 根和 O:/winnt 并核对：

- run16s.exe `88b6bf1f299176960133b5cadd71c008ef3bdd6dd358a9a2b6450041a957c196`
- ntw16s32.dll `a156e7d90c889fa4021a3858990058f7c0aa3bd6d8535b15dfccc59bebc3ce50`
- ntw16s64.dll `c951a8104c5c1df547b0bd62caff047eba0a98e11c033525d81b61ba132b7536`
- demo.exe `787d8f165130620ba711a04a958de1eae5cfc239071709a4bae6051452dfe5c2`

P1 `26c8bc73eb3143902d384d6e8445a67a8b5e54e8` 已推送，实际远端树确认 file 源码和测试移除并复核四产物；两处发布摘要再次核对。S39 关闭，T21 保持开放。
