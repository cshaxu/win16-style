# System Architecture

## 组件和四产物契约

机器可读映射为 [product-contract.json](../etc/product-contract.json)，修改时须与本表一致。

| 组件 | 输出 | 责任 | 不承担 |
| --- | --- | --- | --- |
| run16s-exe | run16s.exe / x64 | CLI、目标选择、首次挂起创建、匹配 DLL 注入、恢复与错误返回 | 绘图、驻留服务、DOS 重定向 |
| ntw16s-dll | ntw16s32.dll / x86；ntw16s64.dll / x64 | 进程内拦截、后代传播、窗口创建附着、唯一外观实现 | broker、WOW、应用业务逻辑 |
| demo-exe | demo.exe / x64 | 原生窗口、菜单与控件示例、正常消息循环 | 样式 API 调用、外观实现、测试开关 |

```text
run16s.exe (x64) -> 创建并注入目标 -> ntw16s{32,64}.dll
                                      | 外观附着
                                      + 创建子进程 -> 同位宽 DLL -> 后续子孙
demo.exe 直接启动 -> 仅 Windows 原生 UI
```

run16s 和 DLL 可在编译时复用 ntw16s-dll 所有的私有注入源码，不产生第五个
运行产物。系统已有的受控跨位宽 helper 可作为 OS 机制；其必要性、调用方式、
超时、错误与清理必须在导入 S 的活动包中明确，不能偷偷增加自带 helper EXE。

## 创建事务与生命周期

启动器和 DLL 都选择实际目标进程的位宽，不能使用父进程位宽代替。
DLL 从可信包目录按绝对路径定位。A/W CreateProcess 参数、环境、工作目录、
继承句柄、启动信息及原有 CREATE_SUSPENDED 语义须保留。安装成功后只恢复
本层为安装增加的挂起；调用者要求挂起时仍返回挂起进程。
失败时不发布半初始化进程；创建者仅可终止本事务新建且尚未交付的子进程，
并关闭自己拥有的句柄。禁止影响既有进程。

外观、字体、绘制、命中测试和资源只有 DLL 一个实现。窗口资源属于 GUI
线程；跨线程调用不得直接更换 HWND 状态。进程内拦截保留原函数 trampoline，
避免递归，窗口销毁释放恰好一次。DllMain 只做经过明确审计的有界引导，
不得加载样式库、枚举窗口、创建线程、等待、RPC 或创建 GUI/GDI 对象。
最终初始化及 hook 安装点必须先研究源实现再准入，不能把源项目行为直接视为合格。

## 来源差异

源项目 renderer 与 nthook 是两个 DLL 家族，本项目必须合并责任到两个 ntw16s
产物；不得将 nthook32/64.dll 当作额外包依赖。原始 run16 有服务依赖，
W16Style 会主动加载样式，均不是可直接复制的最终应用。
ABI 的保留/删除及导出 ordinal 在固定来源对照后逐项决定；不承诺二进制兼容
ntvdm64，也不修改它的消费者。具体文件与证据见[导入候选](../proposals/native-style-import.md)。

T1 已选独立化设计：[引导与注入事务](../etc/import/INDEPENDENCE.md)。
本项目核心 ABI 定为 3，原 scroll/theme binding 改为 DLL 私有，helper 使用
ordinal 1；普通无载荷加载为被动核心，带载荷的新目标才启用自动拦截。
DllMain 最小 Detours 地址改写/载荷验证与 renderer 延后初始化分离；保留 /MT
及静态 TLS 的线程通知。无 broker/句柄身份继承；载荷仅含固定宽头和包目录。
同宽直接更新 imports，跨宽使用有界 OS rundll32 安装事务，所有等待共享
10 秒截止时间。staged import 成功与目标实际初始化成功分别验证，调用者
原要求挂起时不能为等待 ready 提前恢复。
这些是已选设计，产品尚未实现；详细限制和后续证明责任由链接的设计记录承载。
