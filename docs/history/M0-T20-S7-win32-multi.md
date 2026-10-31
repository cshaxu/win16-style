# M0 T20 S7：普通 Win32 文件多选契约调查

## 证据和范围

针对 `GetOpenFileNameA/W` 的 `OFN_ALLOWMULTISELECT`：系统 Explorer 布局以
NUL 分隔目录及文件名，旧布局以空格分隔并对带空格的文件名依赖短名；参见
[微软 Open and Save As 文档](https://learn.microsoft.com/en-us/windows/win32/dlgbox/open-and-save-as-dialog-boxes)
及 [OPENFILENAME](https://learn.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamea)。

自有探针在 O: 下各自创建目录和文件、私有桌面运行、退出后删除文件及目录。
双位宽 A/W 的16组系统对照：旧式短名各4组列出并选中两项，返回空格分隔；
旧式带空格长名各4组未按长名找到，Explorer 短名及长名各4组均实际返回
两项NUL分隔结果。随后 x86 Unicode `FindFirstFileW` 确认两个长名的
`cAlternateFileName` 均为空，旧式列表为空，Explorer 返回两项真实长名。
该现象只证明本机O:夹具，不推定所有卷均没有8.3别名。

Explorer x86 Unicode 小缓冲区对照返回 `FNERR_BUFFERTOOSMALL` (12291)，
缓冲区记录所需长度80；已发布包同一情况也返回12291和80。早期对照因探针
混用路径斜杠导致窗口无法退出，限时终止后只清理本探针打印的自有目录；
修正为 `GetFullPathNameW` 后成功。旧式实际长名列表未显示及旧/新返回格式
不同，故不能仅清除 OFN_EXPLORER；按用户要求保持结果和二八优先级，本S
不以丢失文件、伪造结果或调整全局短名设置换取旧布局。该多选仍为系统
Explorer布局，未实现用户希望的统一旧布局；若要完成需另行研究可靠的
旧式界面长名枚举、多选及返回格式转换，包含导航后不同卷、ANSI、取消、
缓冲不足和hook语义，不能根据启动目录单次检查断言安全。

新增纯C原生探针检查目录、两项文件、末尾双NUL、offset、缓冲错误及自身
清理，测试涵盖系统旧式短名、系统新式长名、已发布路径长名及小缓冲。未覆盖
真实鼠标输入、所有文件系统、非ASCII、应用自定义模板/hook和任意路径长度。

## S7 交付验证

`s7-multi-final` GCC/Ninja/ccache 双宽干净构建，来源、ABI、原资源和治理
门禁通过；同批 `s7-multi-focus` 24项通过、无跳过（约44秒），其中16项为
双宽 A/W：系统旧式短名、系统 Explorer 长名、样式启动的 Explorer 长名和
小缓冲 `FNERR_BUFFERTOOSMALL`。测试检查路径、两个名称、双NUL、offset、
错误及自有文件清理；余项为demo及基本首次注入/传播。物理输入未测试。
无产品源码变更或新发布；项目assets与O:/winnt四文件摘要逐项一致。

重建包 SHA-256：run16s `8b59387f0f8a263cf01d3d0f947869dabce4f0a400a572a676d86c34930f2393`；
ntw16s32 `9dc67083dab12d5dc9d4132c34ab508a05765812f7931f9409e0e3b929380146`；
ntw16s64 `287d83d47b2e8860d3d1b75421e1a5d8a391cc6de727f761c3e19bbe34d7bea3`；
demo `cc3b051e82db67e2cff7568ec1429ca10013205f43b6c8a1e49c57b02be5ba40`。
这些重建字节与已发布包不同，不用其覆盖已发布文件。测试文件为独立原生
探针，产品负责现有回退，未改变所有者或生产路径。
