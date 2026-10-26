# M0 T17 S17：Microsoft 扩展 OCX 承载审计

S17 审计冻结的 35 个模块、66 个 Microsoft 扩展控件行。只读检查本机
SysWOW64/System32 后，列出的 `comctl32.ocx`、`mscomctl.ocx`、`comdlg32.ocx`、
`msmask32.ocx`、grid/data/media/legacy 等模块均不存在；因此没有可认证的版本、许可、
wrapper HWND、子控件或事件生命周期。

现有 renderer 对原生公共控件的有限支持仍保留，但它不能被外推为 OCX wrapper 支持：
OCX 可以改变承载、字体、图像、通知和 COM 生命周期。S17 不安装/注册组件、不猜测
class alias，也不让缺少 UAC/桌面成为理由。完整清单处置见
[S17 证据](../etc/evidence/vb-extension-ocx-audit.json)。

本 S 没有产品改动。推送复核后关闭，顺序准入 S18 审计仍可自包含验证的 intrinsic
图形选择 default/disabled/picture 变体；最终 148 单位验收会将本次缺样本逐行列为
未覆盖，而非误称完成。
