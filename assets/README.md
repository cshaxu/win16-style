# Product package

当前批次以[发布清单](../docs/etc/release.json)为准：M0 T6 S5 P1 / 0.6.0.0。
三组件四产物：run16s.exe(x64)、ntw16s32.dll(x86)、ntw16s64.dll(x64)、demo.exe(x64)。
README是仓库说明，不是第五产品。运行时将四文件放在同一目录，从自身目录定位DLL。

滚动条恢复17逻辑像素厚度；横/纵端钮及固定滑块随实际横截面保持正方形，绘制与
命中测试共用几何。五份原图形头及原声明逐字节保留，两DLL各18表；不替换图形。
启动器保持Console，控制台目标默认等待并返回实际退出码，GUI默认异步，--wait均等待。

本S干净双宽全构建，124输入与S4相同。S4完整矩阵58通过/2隔离剪贴板权限跳过、
0失败，注册/执行一致；本S未重跑全矩阵，
实际同包10/10。两路demo原生探针验证Tab焦点、控件数据/通知、菜单/About、下拉、
滚动捕获、移动与窗口状态。本机DPI144/150%，滚动条厚26物理像素。原资源外观一致
已由用户确认；探针不宣称物理键鼠，隔离clipboard无法创建station（错误5）不计通过。

同批四文件已发布assets根目录及O:/winnt/system32，均与被测包SHA一致。
二进制变化的每个S都执行双位置发布、旧文件备份和摘要核验，详见
[执行规范](../docs/rules/EXECUTION.md)和[构建/使用](../docs/etc/operations/BUILD.md)。
交互修复记录见[S4探针与修复](../docs/history/M0-T6-S4-acceptance.md)。本S最终审计见
[完成审计](../docs/history/M0-T6-completion.md)，已完成远端复核及收口。
