# M0 T17 S16：特殊内建宿主审计

S16 审计 Data、OLE、MDIForm 和 UserControl。没有产品改动：它们不能只按类名当作
普通可重绘控件。固定 VB6 Data 样本已经带有绝对数据库路径、ODBC/Access 与绑定字段；
固定 MDI 样本则在启动代码中使用 registry helper，并具有删除文件、ShellExecute/URL
路径。OLE 和 UserControl 在登记样本中也没有独立、惰性且公开协议可验证的实例。

因此 DLL 继续只接纳已验证的标准子窗口；不会主动访问数据、启动嵌入对象、接管 MDI
客户区或猜测 UserControl。窗口宿主本身及应用内容保留原生，防止把业务或对象生命周期
误认为绘制缺口。完整样本/安全边界见
[S16 证据](../etc/evidence/vb-special-hosts-audit.json)。

S16 关闭后，顺序推进到 S17，对官方 Microsoft 扩展 OCX 目录做版本、可见性、公共
native 承载和已存在 renderer 路径审计；M0/T17保持开放。
