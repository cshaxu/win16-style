# Source Layout

导入任务按需建立目录；当前不放置虚假实现或空构建目标。

样式库与对应测试统一使用下列三层根。组件接口与所有权以ARCHITECTURE为准；实际覆盖及迁移证据由CURRENT和各S记录管理，不从目录存在推导能力已支持。

```text
src/lib-win32/                         无风格含义的进程、句柄、托盘基础能力
src/lib-win32-style/access/
  startup/ runtime/ global/            三种接入方式
src/lib-win32-style/application/
  lifecycle/ window/ menu/ control/    公共状态、协议、输入与生命周期
  dialog/ graphics/                   对话框机制、绘图资源
  adapters/win32/ uwp/ winui/          应用框架适配
src/lib-win32-style/appearance/
  win3x/ win2000/                      各自 common、window、menu、control、dialog、resources
  luna/ aero/                         未来规划，按实际实现建立
src/run16s-exe/                        CLI 与托盘业务装配
src/ntwshk-dll/                        双位宽共享运行时装配
src/ntw16s-dll/ src/ntw2ks-dll/         合入中性运行时的年代装配源码
src/demo-exe/                         原生示例
 tests/                              按源码组件对应目录，产品组合测试另列 integration
 tools/                              构建、检查及观测
 assets/                             三件运行文件和四件主题文件
 build/                              生成物，保留 .ninja 和 .ccache
```

接口和依赖边界由 [ARCHITECTURE](ARCHITECTURE.md) 统一规定。公共源码组件可静态链接到多个现有产品，不自动新增 DLL。demo 仅依赖 Windows API；公共组件不引用产品私有头，各风格不互相引用。

目标构建基线：纯 C17、MinGW-w64 GCC + Ninja + ccache，警告作为错误，
x86/x64 使用独立工具链与稳定 build/.ninja/x86、build/.ninja/x64；CMake 可作
Ninja 生成器。MSYS2 是可选构建环境，发布物不依赖其运行时。T6 S1已完成C/GCC迁移、干净重建及
同包聚焦测试；完整回归和M0验收以CURRENT为准。旧MSVC结果不是新基线证据。不沿用 ntvdm64 的个人
路径、历史 T 编号脚本、服务发布目录或外部生成图。构建工具实际可用性由首个
实现任务验证。T2 已完成双位宽核心 DLL 构建，T3已有x64启动器；T5已有纯原生x64 demo。

build/.ninja/ 与可选 build/.ccache/ 可复用；其他 scratch 必须在活动包登记
精确目录、用途与回收责任。证据先提炼到跟踪文档，任务结束后回收不再需要的
scratch；每次测试完成即按执行规则清理已不再需要的临时构建及副本，
仅为未完成的后续验证或发布暂留有明确用途的最小集合。不得清理用户目录、
系统目录、其他任务正在使用的内容或跟随 reparse point。
正式包根目录为启动器和双宽中性 DLL，两对主题位于同级的 Win16、Win2K 目录。

T1曾以独立MSVC C17/C++17、/MT双架构探针验证旧基线；该探针及旧入口
已在T6 S1退休，历史可从T1提交复取。当前工具链直接构建实际产品和C测试，
入口为tools/Build-Core.ps1，不能用历史探针结果替代当前GCC整包验证。
本机双架构 SDK/DLL/TLS 实测已通过，结果及证明局限见
[工具链记录](../etc/import/TOOLCHAIN.md)，不等于产品构建成功。

## 现代外框的当前源码位置与迁移边界

公共宿主、输入、表面和重绘机制位于 `src/lib-win32-style/application/adapters/`；
原生标题和目标观察位于其 `uwp/` 子目录。全客户区的共享几何、命中计算
及宿主调色位于 `application/window`。现代全客户区适配器位于
`appearance/win3x/window/modern_frame{,_layout}.{c,h}`，提供宿主边界与呈现数据，
由公共 `adapters/frame` 对象维护菜单、输入和表面状态。
对应测试按相同组件放在 `tests/lib-win32-style/`；实际产品组合验证保留明确标记，不能把它们冒称为独立公共库测试。

接入协调位于 `access/runtime/uwp_dispatch` 和 `access/global/uwp_global`；产品仍有导出、
消息协议和装配；接入协调、宿主状态/消息/表面和图形资源使用共同根；appearance保留年代几何、命中和呈现。不能为 Win2000 或未来风格复制宿主，
应由公共适配器接受各风格几何、资源和绘制策略。

现有契约保持：调用者提供根窗口及原生标题 HWND，在所属 GUI 线程应用或移除，
并持有模块直到移除后全部回调返回。标题动作复制数组，借用标签和上下文；临时菜单命令
先检查冲突、按身份清理，再调用动作。原生标题动作的 UIA 查询在协调线程进行，
菜单调用使用独立 MTA worker 及模块引用；OLEAUT32 是系统依赖，不增加产品文件。

启动器已有 calc.exe 身份核验及包激活路径，保留参数、等待和复用实例语义；
普通同名程序不扩展为全局扫描。全局协调跟踪选定的 CoreWindow/宿主对，
停止时尽力移除仍存活的适配，已固定 DLL 不物理卸载。客户区 XAML 控件、
间歇激活失败及实际宿主覆盖仍以 CURRENT 的证据和限制为准。
