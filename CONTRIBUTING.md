# 参与贡献

感谢你关注「小孟蜜汁系统」！这是一个为 **ESP32-S3-WROOM-1-N16R8**（320×240 横屏）设计的掌机 / 多媒体固件。

## 贡献前须知

- 项目只支持 **ESP32-S3**，根 `CMakeLists.txt` 已锁定 target，请勿改成经典 ESP32。
- 界面基于 **LVGL 9 + esp_lvgl_port**，单线程驱动（`lvgl_menu_run` 是唯一 UI 主循环）。
- 设计规范见 [`docs/DESIGN.md`](docs/DESIGN.md)，开发血泪教训见 [`docs/DEVNOTES.md`](docs/DEVNOTES.md)——**先读这两份再动代码**。
- 字体是**子集字库**：任何新增中文都要先确认存在于 `tools/ui_chars.txt` 并重新生成 14px / 10px 字体，否则会显示缺字方框；需要全量字符时改用 `book_font_lvgl`。

## 开发流程

```bash
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
idf.py -p <串口> flash monitor     # 例: COM7 或 /dev/ttyUSB0
```

若曾在同目录按经典 ESP32 编译过：

```bash
idf.py fullclean
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

## 提交检查清单

在提交 PR 前，请确认：

1. 新增中文都在 `tools/ui_chars.txt` 内（或改用 `book_font_lvgl`）。
2. 运行 `python tools/check_balance.py`（或等效）校验括号 / 花括号配对。
3. 没有引入阴影 / 透明模糊 / 大面积渐变 / 高频重绘。
4. 屏幕切换走 `ui_screen_show()`，不要裸用 `lv_screen_load`（会泄漏整屏对象）。
5. 不要提交 `build/`、`managed_components/`、`sdkconfig`、`sdkconfig.old`、日志与临时文件。
6. 上板验证：连续翻页 / 进出应用 10 次以上，free heap 不持续下降。

## 风格约定

- 中文注释、UTF-8 编码、4 空格缩进。
- 界面颜色 / 尺寸 / 动效一律引用 `lvgl_ui.h` 里的 token，不要写死魔法色值。
- 提交信息建议中文、语义清晰（如 `fix(ui): 修复列表末行与底部提示重叠`）。

## 反馈与提问

- Bug 请附上 `idf.py monitor` 的日志片段、复现步骤，以及是否有 SD 卡 / 是否连网等环境信息。
- 功能建议请说明使用场景与期望的按键交互。