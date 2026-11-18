# M0 T27 S14 VB、VCL 与高频复合控件对照

从 [S1 冻结范围](../etc/evidence/t27-s1-controls-reference.json) 的 8 个运行库组
出发。先记录每组 Win3.x 实际入口、Win2000 当前结果、共享身份入口、
应归还的应用角色及试验证据。旧 [VB 逐项台账](../etc/evidence/vb-final-audit-ledger.json)
的 148 行与 [VCL 计数](M0-T18-S64-control-inventory.md) 的 74 家族作为来源，
各代已有部分范围分别保留。原版 Win2000 的边线、图标、状态由 S2–S13 的
原始源码和已核实图片支持。运行库外壳与其公开原生子控件是两个观察单位。

| 固定组 | 首轮核对 |
| --- | --- |
| vb-native | 同一 MSVBVM50/60 模块、准确类名、标准按钮/选择/编辑/列表/组合协议。 |
| vb-containers-and-owner-roles | 原子窗及内容所有者、VB 窗口化和无窗口绘制，核对传统 Frame 和自绘列表边界。 |
| vcl-native-wrappers | 已有原生身份、受证实的真实转发和控件消息，先看标准按钮、编辑和真实子窗。 |
| vcl-text-and-containers | 共享身份中标签、分组、框架及滚动边缘，保留应用内容。 |
| vcl-grid-file-inplace | 原生非客户边缘、滚动和子窗，与专用数据/文件角色分别登记。 |
| vcl-data-navigation-media | 数据、命令和位图由应用持有，核对是否有已证实的相同原生前端。 |
| vcl-special-composites | 日期、色盒、工具/状态和富编辑，分别核对窗口及已证实的协议。 |
| ocx-and-unsupported | 同模块、公开协议已证实的 OCX 及其真实原生子窗，其他角色记具体限制。 |

优先复用 application/control/control_class.h 里的运行库模块识别以及
control_protocol 的同线程已验证角色。Win2000 只持
本年代绘制政策，版本上下文留在公共应用层。避免按任意名字识别，保留
Win3.x 独有运行库观察及第三方内容。现有外部只读样本运行前核对其 SHA
及安全场景，自有 C 测试必须能在没有样本的情况下运行。旧样本报告只能作为
身份和行为旁证，不充当本轮实际验证。实际有二进制改动以一批正式包验收。

已只读重算实际样本与运行库摘要，尚未在 S14 启动。DoubleKiller 和
MD5summer 与 Delphi 历史样本相同，Actor 和系统 VB6 运行库也与
记录一致。系统目录没有 VB5 运行库。有必要时可复用 T17 已固定的原作者
样例及官方分发包做仅应用目录的试验，按原摘要解包，不运行安装器或注册。
先完成无需外部样本的原生双宽模型，再在固定样本上观察特点，分开报告。

| 样本 | SHA-256 |
| --- | --- |
| D:/programs/misc/DoubleKiller.exe | cdd83ab909db749992d11ace2dff671493c0568d7a14697f8bef4b06d8647ca8 |
| D:/programs/misc/md5summer.exe | 4646ce4f638e7d7ec820e7540b39bb185234e1f151d780c734ca557d6dba7ed9 |
| O:/winnt/actor.exe | df4887ba302322cd3fd8c3d542015f492e4de2619ee62532d66eb465bfb51500 |
| C:/Windows/SysWOW64/msvbvm60.dll | 898288bd3b21d0e7d5f406df2e0b69a5bbfa4f241baf29a2cdf8a3cf4d4619f2 |

[VB 原作者样例的固定签收记录](../etc/evidence/vb-intrinsic-controls.json)
含 VB5/6 档案路径、摘要及可观察的只读启动流程。
