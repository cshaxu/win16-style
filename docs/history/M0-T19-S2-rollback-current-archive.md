# M0 T19 S2 — 撤回活动包归档

## M0 T19 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；用户要求撤回本S两次试用，恢复已发布T19 S1 P208 |
| Admission And Approval | 用户明确要求撤回全部字体试用并发布最初版本 |
| Objective | 还原T19 S1字体实现、S2新增测试与构建配置，并发布P208字节完全一致的四文件 |
| Non-goals | 不变更标题、菜单、原始图形资源、应用布局或应用自绘 |
| Reference Baseline | T19 S1 P208四文件和当前字体实现 |
| Candidate Proposal | [字体布局](../proposals/font-layout.md)，本S以用户新要求覆盖控件字形缩放策略 |
| Files And ABI Surface | 现有font.c及必要的C探针、测试和文档；不新增公共ABI或产品 |
| Applicable Rules | M0，纯C、MinGW、Ninja、ccache，原资源字节保持 |
| Verification | 与S1代码和测试逐字节对照；双宽干净构建、原始包实测及两处SHA核对 |
| Expected Markers | 字体、原图形资源及原始四文件与P208一致；Canon图标修复保留 |
| Asset Needs | 登记build/t19-s2-rollback及build/t19-package任务scratch；完成后提炼并清理 |
| Reporting Requirements | 撤回范围、原包SHA、双宽构建与实际原包测试、双发布和远端复核 |
| Stop Conditions | 原始包身份、占用或发布验证失败时停止并保留证据 |
| Exit Criteria | 重建并测试回退源码，实际核对原始P208四文件、发布及远端复核；仅关闭S2，保持T19开放 |
| Original Owner Request | 用户不满意字体试用，要求全部撤回并发布最初版本 |
| Similar-Issue Sweep | 区分菜单、标题、VB及标准控件字体获取路径，保留应用自绘 |
