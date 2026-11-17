# Project Status

## Current Work

M0固定。T26按已批准的[三层架构与14个S规划](../proposals/style-library-reorganization.md)完成既有能力重组与有限终审；S1–S14已交付，当前无活动任务包，不自动准入新T。这个结论不表示原有绘制/恢复缺陷或所有第三方框架兼容问题已经解决。

[最终审计](../etc/evidence/t26-s14-final-audit.json)逐项记录473个当前样式/产品C/H归属、实际公共根/入口/消费者和原路径处置。443项沿用字节一致的阶段证据，另30项核对runtime/CLI/ABI叶子；1170生产/测试/工具输入与最终干净构建快照完全相同。全T生产C/H按Git numstat含rename检测净+3508行，不宣称总footprint减少。

最终产品由S13 P1 `1b1b33d3`同快照MinGW GCC 16.1/C17、Ninja/ccache双位宽干净构建，x86约268秒、x64约299秒；八文件先发布assets根与O:/winnt再测试。S14未改变二进制输入，复核同摘要包，未重复构建或虚构新发布。[S14 P1实际远端](../etc/evidence/t26-s14-remote.json)已fetch核对9个变更blob、1170冻结输入及八文件。S13[交付](../etc/evidence/t26-s13-renderer-delivery.json)及[实际远端](../etc/evidence/t26-s13-renderer-remote.json)保留完整构建/包身份和原始结果。

S13最后120调用119过/1未过，x86 v5 header回退24像素由完整S12包同条件复现；随后一次S13对照过但首失败保留。S14首94调用86过/8未过；两次dialog-link补齐其既定临时目录环境后通过，原始失败保留。最终94项按正确前置条件88通过、6未过、0skip，总96调用不称首轮全过。两宽公共六条转换、实际窗体/子控件切换与移除、无样式目标/传播、资源/失败/重入、12个dialog scope和独立modern provider通过；具体范围见[实施记录](../history/M0-T26-S14-final-audit.md)。

6个最终未过仍为双宽Win2000 DWM恢复restored=0及普通/composed原生thread hook126；直接入口/表面通过不能替代。原static exception0xC000041D、header间歇24像素、实际计算器1460/宿主关系与其他未覆盖面保留；后续兼容工作见[TODO](TODO.md)，不改权限、安全策略或扩框架来掩盖限制。此前dialog激活NULL本次未复现，不声称代码修复了桌面环境。

S13/S14正式交付scratch/发布备份合计370141812字节已先归档证据后核验清理，S13十批开发34129391字节也已清理；见[S13清理](../etc/evidence/t26-s13-renderer-cleanup.json)、[S14清理](../etc/evidence/t26-s14-cleanup.json)。无本任务worker或测试副本；稳定.ninja/.ccache和正式八文件保留。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 26
- Last allocated Td S: 21
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 8
- Last allocated T26 S: 14
- Last verified remote delivery: M0 T26 S14 P1，`9d5ea6ea`；9个变更blob、1170冻结输入及八文件实际复核。
- Open task: none；T26按既有能力三层重组范围收口，不准入新T。

## Technical Baseline

- 产品为[契约](../etc/product-contract.json)固定五组件八文件：run16s.exe、demo.exe、ntw16s32/64.dll、ntw2ks32/64.dll、ntwshk32/64.dll；正式文件在assets根与O:/winnt，摘要见最终审计。
- 样式统一src/lib-win32-style的access/application/appearance；通用无样式Windows能力在src/lib-win32。当前结构和接口见[ARCHITECTURE](../design/ARCHITECTURE.md)及[CODING](../design/CODING.md)。
- 三接入、共同生命周期/窗口/菜单/控件协议/对话框/现代宿主/图形根已有真实消费者；年代保留度量、绘制、字体、资源与支持策略。产品保留CLI/托盘业务、目标/会话选择、原API trampoline、兼容ABI和装配。Win2000控件、其他框架与Luna/Aero不因公共根存在即得到完整支持。
- 来源1077回执、五原资源、包内原始栅格表和ABI/PE门禁通过；两宽各23公共样式归档与4进程归档无产品/年代反向依赖。所有当前构建/运行/测试不依赖或修改兄弟仓库。
- propagation停用、窗口恢复、thread hook解除与物理DLL/late patch卸载分别报告；原有进程存活期固定/保留能力不改称安全卸载。
- 禁止computer-use、创建或切换私有桌面；只用自有有界当前桌面探针。用户已永久授权非computer-use验证及按身份处理正式文件实际占用者；不重复请求同一授权，不改系统安全设置。
- 历史逐文件来源及迁移见[原顺序台账](../history/M0-T26-S1-ordered-audit.md)、各S证据与T26规划；早期lib-style/lib-win32/style路径是历史路径，当前旧生产目录已删除。
