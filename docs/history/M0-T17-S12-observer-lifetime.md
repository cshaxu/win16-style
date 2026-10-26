# M0 T17 S12：观察器与资源所有权审计

本 S 只诊断 S9 首轮总 GDI 跳变和 S11 临时图形 IAT 观察器偶发安装失败；没有
修改 renderer、标题、字体、菜单、滚动条、ABI、原 glyph 或来源声明。

## 实测结论

自编写的 VB5/VB6 PictureBox 夹具在 x86 与 x64 各做两次完整裸/注入生命周期：
裸路径首轮 `0->2`、第二轮 `2->2`，样式路径首轮 `0->4`、第二轮 `4->4`。
每个样式循环都精确检查两份不同私有字体在 callback retirement 后 `GetObjectW=0`，
同时保留源字体、原生输入、图像、几何、RTL 回退、hook、mapping 和 event 的清理。
因此此夹具中的首轮跳变是有界缓存，不是逐轮私有字体泄漏；不能外推为任意真实
VB 应用或全进程 GDI 零增长。

图形观察器仍拒绝未知 import hook。诊断现在记录 malformed-image、目录、实际/预期
地址及模块所有者、页面保护、CAS race、容量和还原阶段；断言没有放宽。自编写图形
运行时在候选四文件包上完成 x86/x64、VB5/VB6、裸/注入对照；API/像素/状态、后期创建
和精确私有字体释放全部通过。一次自有 x64 双实例并发对照也通过，未复现 S11 偶发项。

两个可选真实 VB6 样本在本机非输入桌面于 `CreateProcessW` 前失败为 623，未安装观察器；
这不是 IAT 根因。自有探针还记录 `OpenInputDesktop`/`CreateDesktopW`/`CreateWindowStationW`
均为 5、`WTSQueryUserToken` 为 1314；未触碰用户输入桌面、窗口站、服务或既有进程。
完整日志摘要及限制见 [S12 证据](../etc/evidence/vb-observer-lifetime.json)。

## 构建与包验证

双位宽 GCC/Ninja 重新编译新诊断与生命周期夹具；source manifest、第三方来源自检、
治理 13 项负例和 `git diff --check` 全部通过。候选四文件包经 `check_package.py` 核对
架构、导入、版本与每 DLL 18 个原资源表；在该包上运行上述八个 x86/x64 VB5/VB6
容器/图形原生用例均通过。

本 S 改动仅为 CTest/原生探针与证据；生产源码没有差异，因而不产生新的产品行为或
二进制发布。标准私有桌面全包 UI 矩阵在这个执行会话无创建桌面权限，明确保留为
环境限制，未使用当前桌面 fallback 把菜单跟踪结果伪装成回归通过。

## 收口

P1 提交及远端复核后关闭 S12，并按用户已授权的自动顺序准入 S13：真实图形
CheckBox/OptionButton 的身份、默认绘制和有限适配审计。IAT 原始真实样本复现、
复杂图片、DriveListBox 与最终 148 单位审计仍按 TODO/后续 S 接收；M0/T17 保持开放。
