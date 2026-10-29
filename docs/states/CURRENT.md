# Project Status

## Current Work

M0 T20 S1准入：Win3.x文件夹选择与公共对话框调查，先确认真实接口、经典参考及语义边界。

## M0 T20 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；用户准入队列首位，T20 S1 |
| Admission And Approval | 用户要求T19收口并准入文件夹选择与公共对话框任务 |
| Objective | 识别真实应用的文件夹选择入口、窗口与数据语义，研究可保持语义的经典兼容路线 |
| Non-goals | 不分发旧系统DLL，不更改原图形资源、共享Shell宿主、系统设置或用户现有进程 |
| Reference Baseline | T19 S1 P208原始四文件，S2撤回，S3用户确认恢复 |
| Candidate Proposal | [Win3.x文件夹选择与公共对话框适配](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 纯C原生探针及必要的现有DLL模块，优先维持三组件四产物和公开ABI |
| Applicable Rules | M0，MinGW GCC、Ninja、ccache；原资源与来源声明逐字节保留 |
| Verification | 真实目标、自有有限实例、调用接口与参数、PIDL/路径/回调、原始经典参考及双宽对照 |
| Expected Markers | 明确调用路径与最小可实现范围，不以外观推断接口或以替换图标宣称功能完成 |
| Asset Needs | 登记build/t20-s1-dialog及build/t20-package scratch；成果提炼后清理 |
| Reporting Requirements | 实际证据、支持和回退矩阵、许可、不可兼容条件及可行路线 |
| Stop Conditions | 无法保证调用语义与资源许可时保留原对话框，不伪造结果 |
| Exit Criteria | 真实入口及可实施方案有证据；产品变更依规范构建、测试、双发布、推送复核 |
| Original Owner Request | 准入队列首位，处理Win3.x文件夹选择与公共对话框外观 |
| Similar-Issue Sweep | SHBrowseForFolder、GetOpenFileName、GetSaveFileName、IFileDialog及框架自绘 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 1
- Last delivery: M0 T19 S2 P214，撤回提交940a8d42bfda18da413577319e72970102d52156，实际远端源码和P208原始四文件已核对。
- Open task: T20 S1 文件夹选择和公共对话框接口及经典参考调查。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1初步原生双宽Shell探针和Win3.1原始模板只读调查已完成；旧式Shell仍是树，不等同Win3.1文件打开框，真实截图来源与动态接口尚待确认。详见[初步证据](../history/M0-T20-S1-preliminary.md)。

## Recent Closure

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。
