# M0 T2 S2 — actual DLL runtime verification

范围：在 S1 的两个 ABI 3 核心 DLL 上建立有限运行验证集，不实现注入或产品 demo。
七个夹具从 T1 固定 blob 导入；适配本地测试注册，保留既有动态 DLL 路径参数，
并为字体夹具添加私有桌面。
新增 runtime、frames、isolated_process 三文件；53 回执保留来源、声明与实际摘要。
本 S 未修改 renderer 或公开 ABI，未复制受限制的原图。

## Frozen coverage

每架构恰为以下 13 项，以实际 CTest 注册名为单位；全部为本仓库构建与运行。

| Test | 有限契约及证据类型 |
| --- | --- |
| core/abi | LoadLibrary/GetProcAddress；ABI 3、31 public、private binding 不导出 |
| core/runtime | 实际 DLL；root+9 children 创建附着、六控件、原生状态/通知、wrong thread、menu 96/144/192、字体租用、移除与销毁、20 轮资源 |
| core/runtime-native | 同布局/原生状态/通知与销毁对照；不安装样式 |
| core/frames | 六 window forms、client screen 保持、hit test、title/palette、与原生对照的 max/min/restore、三排除 exstyle |
| core/font | 实际 DLL；CP1252 224 glyph/39100 pixel、系统字体比例与资源 |
| core/font-dpi96 | 同一字体夹具的 96 DPI 配置 |
| core/tabs | 实际 DLL；list tabs、font 更新/拒绝/移除与内容原点 |
| core/lifetime | 实际 DLL；控件重入移除、销毁与嵌套安装 |
| core/default | 实际 DLL；默认按钮、Enter、focus、pressed、border、资源及卸载 |
| core/terminal | 实际 DLL；七 family、中和 procedure、重入、foreign thread、重复移除与卸载后销毁 |
| core/sizebox-native | 原生 sizebox 对照 |
| core/sizebox-styled | 实际 DLL 的 sizebox 状态/边界 |
| core/geometry | 编译生产 scroll_layout/scroll_track；数学检查，不能替代 DLL 验证 |

## Failure disposition

首轮 s2-runtime：10/11 通过，组合夹具以冷启动全局资源数作基线失败。
s2-runtime-diagnostic 显示早期增长后稳定；s2-cleanup-proof 的直接 HWND/HFONT
销毁及预热后资源检查通过，但 GUI thread 切回桌面返回 170，原生对照亦有延后
分配。最终组合夹具由父进程拥有私有桌面、等待同程序 worker 退出后关闭桌面；
不用销毁系统 helper window。样式路径仍逐轮要求 GDI/USER 不超过预热基线，
同时检查所有已知窗口失效及最后字体引用释放；没有添加资源容差。
原生对照计数只作观察，其窗口销毁仍有断言。

s2-frames 首轮把“从最小化恢复此前最大化窗口”错误预期为正常状态；改为真实
原生窗口逐步对照 IsIconic/IsZoomed，而后明确 SW_SHOWNORMAL。未改生产实现。
s2-full 的 x86 13/13 通过，x64 组合路径通过后 180 秒总预算耗尽；本轮不算完整
通过。按两架构实测耗时把已登记总预算改为 360 秒，s2-complete 重新运行全集。

## Final run

s2-complete（owned worker PID 40364，360 秒）exit 0；x86 13/13，101.56 秒；
x64 13/13，95.05 秒。上表各行两架构均通过，PE 校验及 DLL 摘要与 S1 一致。
两宽 styled 20 轮 GDI=43/43、USER=38/37；每轮不超过预热基线，全部已知 HWND
及末次 HFONT 释放断言通过。creation scope 记录 installed=10、unsupported=1，
root+9 个自建 HWND 均确认附着；不声称内部附带窗口亦被支持。
原生对照 GDI=23/43，两宽 USER 分别为 15/36、17/38，仅观察其延后分配。

人工查看两宽 WM_PRINT 样式图及原生对照，标题/菜单/六控件无明显裁切或重叠。
两宽样式 BMP 均 840×521、SHA256 0906781c968065d66cf43b62cf06decf7c8e185b608b58d132639e69fde3fd2f；
原生均 850×540、51a11929508ff9e6722e865791314a3876a6bd05fe4e6e9ed2846a61cd3f9384。
图像相等是本次有限布局观察，不是原 Win3.1 像素承诺、任意 DPI 或物理输入验收。

新增测试 C/H +895/-0，根测试注册 +35/-0；其中新编 runtime/frames/isolation
252 行，其余 643 行为已审夹具适配。生产源码变更 0，旧状态与清理仍由唯一
renderer/原生控件所有，没有第二绘制实现。来源回执 53 项及 23 负例、治理
13 负例、五几何头一致、正负 native exit guard、diff --check 通过。

本 S 已验证，P1 推送及实际提交复核后才关闭并进入 S3 整体审计。
