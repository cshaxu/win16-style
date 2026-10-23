# Product package

当前批次以[发布清单](../docs/etc/release.json)为准：M0 T7 S1 P1 / 0.7.0.0。
三组件四产物：run16s.exe(x64)、ntw16s32.dll(x86)、ntw16s64.dll(x64)、demo.exe(x64)。
README是仓库说明，不是第五产品。运行时将四文件放在同一目录，从自身目录定位DLL。

修复鼠标切换菜单后取消仍残留键盘菜单模式的问题。Esc、外部点击、同项二次点击、
CancelMode和失活后恢复闲置，悬停不再自行打开；保留键盘及子菜单Escape层级。
原生私有桌面探针覆盖双宽15场景，直接加载被测发布DLL，不使用computer-use。
构建、相关回归、实际包测试及已发布文件身份见发布清单和
[菜单修复审计](../docs/history/M0-T7-menu-cancellation.md)。探针不冒称物理输入。

滚动条17逻辑像素；横/纵端钮及固定滑块按实际横截面保持正方形。
五份原图形头及原声明逐字节保留，两DLL各18表，不替换图形。
启动器保持Console，控制台目标默认等待并返回实际退出码，GUI默认异步，--wait均等待。

二进制变化的每个S执行同批四文件测试、assets根目录及O:/winnt/system32双发布、
旧文件备份和摘要核验，详见[执行规范](../docs/rules/EXECUTION.md)和
[构建/使用](../docs/etc/operations/BUILD.md)。历史T6验收见
[完成审计](../docs/history/M0-T6-completion.md)，不能代替本次修复验证。
