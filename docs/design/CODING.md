# Source Layout

导入任务按需建立目录；当前不放置虚假实现或空构建目标。

```text
src/run16s-exe/       x64 CLI / 首次启动
src/ntw16s-dll/       双位宽 renderer / interception / injection
src/demo-exe/         x64 原生示例
tests/run16s-exe/     启动契约测试
tests/ntw16s-dll/     双位宽 DLL 单元及生命周期测试
tests/demo-exe/       原生 demo 边界测试
tests/integration/    真实注入、跨位宽链及 UI 观察
tools/               构建、打包、检查与观测工具
assets/binary/       四产物的发布包
build/               全部生成物（忽略）
```

组件不引入第四个通用产品层；共享注入实现归 ntw16s-dll，启动器只消费其明确的
内部构建接口。demo 只依赖 Windows API。产品 ABI 与私有头分离，消费者不得
通过包含 renderer 私有头绕过责任边界。

目标构建基线：纯 C17、MinGW-w64 GCC + Ninja + ccache，警告作为错误，
x86/x64 使用独立工具链与稳定 build/.ninja/x86、build/.ninja/x64；CMake 可作
Ninja 生成器。MSYS2 是可选构建环境，发布物不依赖其运行时。T6 S1已完成C/GCC迁移、干净重建及
同包聚焦测试；完整回归和M0验收以CURRENT为准。旧MSVC结果不是新基线证据。不沿用 ntvdm64 的个人
路径、历史 T 编号脚本、服务发布目录或外部生成图。构建工具实际可用性由首个
实现任务验证。T2 已完成双位宽核心 DLL 构建，T3已有x64启动器；T5已有纯原生x64 demo。

build/.ninja/ 与可选 build/.ccache/ 可复用；其他 scratch 必须在活动包登记
精确目录、用途与回收责任。证据先提炼到跟踪文档，任务结束后回收不再需要的
scratch；不得清理用户目录、系统目录、其他任务目录或跟随 reparse point。
产品包四文件均在同一目录，构建输出与正式发布相互区分。

T1曾以独立MSVC C17/C++17、/MT双架构探针验证旧基线；该探针及旧入口
已在T6 S1退休，历史可从T1提交复取。当前工具链直接构建实际产品和C测试，
入口为tools/Build-Core.ps1，不能用历史探针结果替代当前GCC整包验证。
本机双架构 SDK/DLL/TLS 实测已通过，结果及证明局限见
[工具链记录](../etc/import/TOOLCHAIN.md)，不等于产品构建成功。
