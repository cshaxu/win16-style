# T26 S1 Win16 公共样式目录审计

范围为 `src/lib-style/win16-common` 当前全部七个生产文件；此范围不代表其他样式目录已审完。

| 文件 | 职责与处置 | 调用、验证与限制 |
| --- | --- | --- |
| `layout.c`、`layout.h` | 描述 Win3.1 的具体边框、标题按钮尺寸及命中规则，保留在 Win16；通用工作区、坐标恢复和命中光标已分别放在 `common/frame.h`。 | 窗口与 UWP 外框调用 `win16_style_measure`/`win16_style_hit`。不能把 19 像素按钮、不同边界或 Win16 的命中顺序作为其他年代的默认策略。 |
| `palette.c`、`palette.h` | 负责 Win16 配色结构、预设与高对比度映射。预设和字段归风格；高对比度系统颜色查询可由未来样式复用，但不同主题的颜色角色须由调用方定义，当前没有可直接共用的无风格数据契约。 | `win16_style_default_palette` 被外框和控件使用；`tests/lib-style/win16-common/common_link.c` 验证库链接与预设，不能据此证明其他风格配色。 |
| `font.c`、`font.h` | 合成 Win16 System 位图、菜单和控件字体，线程内持有字体资源及引用。栅格化所用临时位图已复用 `common/bitmap`；字体字形、尺寸、菜单策略及 FNT 数据不能当成 Win2000 或未来风格定义。 | 外框与控件共用字体租约；`core-status-font-isolation` 验证共享、独立租约及资源释放。其纯 C x86/x64 在当前普通桌面通过，不等于全部字体和 DPI 已验证。 |
| `control_api.h` | 仅定义现有 `NTW16S_API` 导入导出宏，与 Win16 产品 ABI 绑定；保留在产品风格侧，不放入无产品依赖的公共库。 | 具体 ABI 和导出检查由 `tests/ntw16s-dll/abi.c` 和构建包检查承担；此项不新增公共 ABI。 |

本次复核运行 `core-win16-common-link` 和 `core-status-font-isolation`，x86/x64 均通过。误将 `core-font-layout` 默认入口作为普通测试启动，该入口尝试创建私有桌面但两次均被系统以错误 5 拒绝；没有创建或切换桌面，测试未运行，不计通过。后续在启动任何图形测试前必须检查入口，只使用不创建或切换桌面的路径。

核对入口后，使用 `--private-worker` 直接运行普通桌面菜单子集，并加载 `O:\winnt` 已发布 DLL；x86/x64 的 96 与 144 DPI 菜单字体测试均通过。此参数直接执行 worker，没有创建或切换桌面；该结果只涵盖菜单子集，不代替完整控件字体矩阵。
