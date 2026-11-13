# T26 S1 顺序第 12 项：像素网格

组合框箭头、滚动条箭头与选择控件原本各自进行按比例像素网格绘制。通用索引色、透明像素、行间距与独立横纵比例绘制迁入 `lib-style/common/pixel_grid.h`；Win16 仍提供原图案、颜色和缩放参数。原始图形资源与来源声明没有修改。第 14 项 `choice_paint.c` 仅更新调用及摘要，仍须按顺序审计。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过，先发布到 assets 和 O:\winnt，再运行公共网格测试及正式包的双位宽组合框、滚动条、复选框和分组控件夹具，均通过；公共测试覆盖透明、颜色、当前画刷、行间距和非整数缩放。测试使用直接工作入口，没有创建或切换桌面。真实第三方程序、全部 DPI 及其他风格尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | e785978db9b9cde2229e719e9a3629fef66f6f382b3a3b7dddee8766dc32f64c |
| ntw16s32.dll | c5951e143907d5897e8ee03fa004f4654140d2cf8bb9820102f966524f6d66f4 |
| ntw16s64.dll | d17726f7f4369f46731f798e3379c1362d33941ac88f1573ae9329a5aa8e951f |
| ntw2ks32.dll | e3ca2eddf977f420b91eda5b7cbbffa9ccfdd91063feb0e6c1b1cdc1e7dcc2f2 |
| ntw2ks64.dll | 15dd66f745817de8c23e9c3c598103b80bf8b0bded317603e8c55a3337ad187d |
| ntwshk32.dll | 10b1244da4e5c6a935d0e5d6ecdf1482fdd6d6a2511144f858271f9d7a4e6012 |
| ntwshk64.dll | b0bb3a1eb9878cad538f7a1a7c8b9675c9a0bf13fc0f713d87126646e108df39 |
| run16s.exe | ba5a9847a2676f3ae97d669eab46013333de5d5ac86521836c6f2eaa624267c1 |

既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
