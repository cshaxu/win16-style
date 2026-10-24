# Build and test efficiency

M0 Td S9按用户授权实施。参考softpc的持久Ninja/ccache、按目标构建、测试标签、
独立工作目录及资源限制，参考只读，不新增必需外部依赖。

先固定ABI/staging/font-layout/menu-cancel及x64 cmd的修前串行有限基线；修后以
同集合/同断言并行对照，不用历史CTest成本当实际本轮耗时。统一项目持久ccache，
干净源码树显式共享原项目缓存，绝不使用用户全局缓存；toolchain置于持久构建图。
增加快速目标/入口，完整集合保持；每case独有scratch目录，限制重UI并发，保留
原超时和skip。日志记录构建/测试阶段耗时及cache前后计数，验证清理后no-work。

最终只做一次冻结源码的双宽全构建/完整回归和实际包测试；按规则比较四文件，
有binary change则双目录发布。所有产品C及原资源字节保持，T8开放。

Td S9已交付收口；实际耗时、完整回归、发布和远端/清理证据见
[完成记录](../history/M0-Td-S9-test-efficiency.md)，当前状态由CURRENT管理。
