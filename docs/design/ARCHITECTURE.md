# System Architecture

## 产品与边界

本项目是运行时 Windows 外观框架。接入、样式应用和年代呈现独立组合；当前能力与实际应用覆盖由 [CURRENT](../states/CURRENT.md) 及其证据说明。三层实现不能等同于所有框架或主题已经支持。

[product-contract.json](../etc/product-contract.json) 固定七件正式文件。公共源码组件静态链接到中性运行时，风格数据由两对外部主题提供。

| 产品组件 | 产物 | 职责 |
| --- | --- | --- |
| run16s-exe | run16s.exe，x64 | CLI、托盘业务、风格与目标选择、提权 UX 和最终错误报告 |
| ntwshk-dll | ntwshk32.dll、ntwshk64.dll | 共享运行时装配、进程内接入和公共状态导出 |
| themes | Win16、Win2K 各一份 .theme 和 .msstyles | 外部风格数据与原图资源 |

Win16、Win2K 的兼容 DLL 和原生 demo 留在构建树用于验证。

## 三层结构

所有样式能力归 `src/lib-win32-style`；无样式含义的 Windows 基础能力归 `src/lib-win32`。

| 层 | 目录 | 职责 |
| --- | --- | --- |
| 接入 access | startup、runtime、global | 启动接入、运行中接入、全局发现与会话协调 |
| 样式应用 application | lifecycle、window、menu、control、dialog、graphics、adapters | 共同状态、协议、输入、附着/移除、资源、重绘与框架适配 |
| 风格呈现 appearance | win3x、win2000 | 年代度量、布局、绘制、配色、字体/资源与支持策略 |

Luna、Aero 是后续呈现实现，不建立空目录宣称支持。

```text
产品装配：选择风格、目标、协议和接入方式
    access/{startup,runtime,global} → application
    appearance/{win3x,win2000,...}  → application 定义的契约
    access 与 application          → lib-win32 基础能力
```

application 定义契约，appearance 实现契约。access/application 不包含年代或产品私有头；各年代不互相依赖。框架适配器接受实际风格的几何和呈现操作，不为每种风格复制宿主生命周期。

标题栏绘制接收已解析的标题、图标、字体、按钮状态和目标 DC。Win32、UWP 和 WinUI 的适配入口负责窗口查询、回调重入、坐标及最终提交。风格绘制不得向 HWND 查询标题、图标或窗口状态。Win32 的非客户区 DC 和现代宿主的合成表面可有不同的 DPI 与提交方式，验收须核对最终屏幕输出。

公共组件拥有共同状态与完整事务，产品或年代提供真实策略。不得接收 Win16 私有结构后用大量回调逐字段读取，也不为每次 SaveDC 或颜色选择制造空包装。局部绘制资源由实际绘制操作就地结算。

## 接入与生命周期

- `access/startup`：载荷/路径、模块验证、安装、helper 及初始化/传播装配；唯一挂钩后端是 `lib-win32/process-hook`。产品提供包和模块身份、策略、API 集合及终端行为。
- `access/runtime`：同位宽映射身份与入口核验、临时线程 hook/请求/结果、晚期批次和回滚。产品提供消息、导出名、动作和实际样式操作。
- `access/global`：session/control/start/worker/late/attach/manager/receiver/owner/host/trace 的状态及停止协调；复用 `process-global` 的目标发现、身份、去重和生命周期。产品提供目标资格、协议、包选择、风格能力和最终报告。
- `application/lifecycle/instance`：GUI 根、身份、重入引用、子类安装/解除和最后引用退场；`install`、`retire`、`creation`、`creation_call` 拥有现有发布、退役和创建事务。
- `theme_state`、`theme_track`：主题观察、所有权、抑制/恢复及临时 open/close 作用域；年代或产品提供实际原生 API 与支持策略。

普通启动、运行时及全局接入组合相同的应用机制。全局切换使用同一协调器：停止当前风格并尽力恢复，再应用选定风格。六条转换不各建一套路径；托盘菜单业务留在产品。

传播停用、线程 hook 解除、窗口恢复、晚期 patch 和 DLL 物理卸载是不同能力。晚期批次和已固定模块维持原有进程生命周期；没有安全物理卸载能力时不能把停用称为卸载。保留既有权限、目标、会话及 UX，不改变注册表、目标文件或系统安全设置。

## 窗口、菜单与控件

`application/window` 拥有 native_frame、install、messages 和 caption_button 的原生状态、事务与捕获；年代提供几何、命中与绘制。`graphics/font_pair` 拥有字体对及 DPI 的提交和失败保留，provider 提供字体配方与租约操作。

`application/menu` 拥有 popup/tracking、嵌套 owner、bar_state、bar_snapshot 和 bar_controller：临时项、原生查询/回调、键盘/焦点、跟踪与 NEXTMENU 引用交接。Win3.x、Win2000 和现代外框消费相同机制，保留实际度量和导航差异；共享运行时不因状态管理静态引入 GUI 依赖。

`application/control` 拥有 control_state、字体/协议/重绘深度、tabstop、子窗口绑定、快照、通知、原生回退与退役。滚动条的资源、拖拽/通知/epoch、捕获/计时器、查询/设置及 surface 坐标归共同组件；年代提供布局、配色、字形、命中/延时和支持条件。

`application/adapters/win32` 拥有 VB/VCL 等运行时的身份、父子关系、消息与绘制关联协议。Win3.x 的具体字体/颜色/绘制支持条件归 appearance。既有 Win2000 控件能力仍有限，不因公共协议已经存在就宣称拥有对应全部 renderer。

## 对话框与现代宿主

`application/dialog` 拥有 classic_file_controller、classic_com_controller、proxy/events、结果回写、print 和 observation 的共同参数及资源事务。产品提供转换选择、会话资格、样式 scope、原生 ABI 和既有实验配置；年代保留 chrome/布局。Win3.x 的预创建与 Win2000 的 post-create observer 使用各自已有合同，不强制改成同一附着时点。

COM 配置字符串、回调代码和模块引用覆盖 factory/proxy/hook 存活期；应用原 hook/template、多选与扩展参数按既有回退策略处理。控制器复用调用者提供的 patch 后端，不再创建第二套实现。

`application/adapters/frame` 嵌入 host/instance，拥有 bitmap、输入/刷新/激活、menu owner 和完整 action 事务。每个年代在 appearance 中定义一套标题栏度量、按钮布局和绘制。Win32 与现代宿主提供有效 DPI、状态和坐标；宿主预留的客户区位置只用于裁剪，不重新决定年代标题栏高度或按钮宽度。

UWP/WinUI 的宿主、表面和标题适配在 application；加载/请求和全局协调在 access。调用者提供 HWND，在所属 GUI 线程操作并持有模块直到所有回调返回。标题动作复制数组、借用标签和上下文；UIA 查询与独立 MTA 调用保持既有线程合同。客户区 XAML、其他框架及真实宿主覆盖以实际验证为准。

## 图形资源与错误

`application/graphics` 拥有 DC/裁剪/bitmap/buffer、font_pair、来源字体 cache、glyph map、快照借还、内存字体引用、variant pool、临时 glyph-text 和完整 text-fit 事务。年代提供原字形/FNT 数据、字体配方、key 与编码/支持策略；原图形资源及来源声明保持。

present/print 同步借用完整 instance 与 renderer；native renderer 通过共同 control state 借还临时字体。frame measurement 借用完整 instance/font_pair，原生菜单/标题回调后读取当前状态，保持重入、替换和退役语义。

错误在最内层结算本层资源并向上传递，最终入口报告；中间层不重复补救、重试或吞错。同步原生调用可以重入/销毁/替换对象，资源所有者必须覆盖调用。保留既有 native unwind；正常返回合同不因此获得新的异常恢复能力。

## 验证

测试随源码组件映射，产品组合测试另有明确标记；一个真实集成测试可覆盖多个文件，不复制测试凑数量。检查实际所有者/入口/消费者、旧路径删除、源及归档依赖、原资源、ABI、双位宽、发布包及恢复行为。

[T26 任务设计](../proposals/style-library-reorganization.md) 保存按完整能力合批的迁移规划；[顺序台账](../history/M0-T26-S1-ordered-audit.md) 和各 S 记录保存审计证据。架构文档不重复进度、通过数或未解决缺陷，当前事实由 CURRENT 与 TODO 管理。
