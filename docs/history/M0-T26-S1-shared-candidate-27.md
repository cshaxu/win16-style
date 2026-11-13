# T26 S1 第二十七批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布至 assets 与 O:\winnt，再测试。

VCL 面板和分组外框的严格 DC 坐标核验共用 `style_dc_origin_pixels`；调用方保留窗口身份、样式和实际绘制策略。

双位宽公共测试覆盖映射、窗口原点、视口原点和 RTL；正式包双位宽面板与分组外框夹具通过裁剪、重入、父窗口变更、状态改变和资源回收。其他风格尚未调用此严格核验，真实 Delphi 程序未验证。

导航焦点失败的跟进：测试以 SW_SHOWNA 创建窗口，临时显式调用 SetActiveWindow 后双位宽均未能使自有窗口成为活动窗口；只读探针同时读到前台窗口为零。临时试验已撤销，无法因此证明具体环境原因或产品焦点正确，导航焦点测试仍为未通过。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等先前未通过或未覆盖范围继续开放；S1 和 T26 尚未收口。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 3140854ca2fb947d5500c51e7b31da5c63f13b8e9bc471ca36d3a1850e42b82b |
| ntw16s32.dll | 2fbb9d3643b647023f492c40d5ee3a7e34469494d235fbe9fb244c36101d8e0a |
| ntw16s64.dll | 3f863759861ed337a22d905eed7fdef8c9cbc598e09c9ac41cd104431e741a25 |
| ntw2ks32.dll | 5d287662b83b1d24391a7c59aca389cad7ef2c78f1ea736eb24a2e80cac6b456 |
| ntw2ks64.dll | ebf28a27dd7f64032422f428c7afb66d1d29577642b69fb346afa26b755cdb8b |
| ntwshk32.dll | 70d2b99386f36fd4657a4e72a573e91e84106be5462e0f90892e5c39662e5757 |
| ntwshk64.dll | 45a310adf71af30a098a8ab7d3cc1fa712a594f6889d9ce8e7820041c5bde94e |
| demo.exe | a73bd7a1575540969828c2526de1423793262fc1fb59777f3515031d83c15b70 |

源码快照清单 SHA-256：`3fd7d5db2741fee4f4b7eefbf477d53d885473b93b8d0d3632c7cd95cf7d7ccf`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：5 个文本文件，新增 28 行、删除 11 行、净增 17 行，另有 8 个二进制文件。公共层拥有坐标前提核验；Win16 仍拥有窗口、状态和绘制策略，两处重复核验已删除。
