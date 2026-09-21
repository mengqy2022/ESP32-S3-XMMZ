# 更新日志

本项目遵循 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/) 精神记录版本演进。

## [v1.0] - 视觉与工程重构（未发布）

### 新增
- 全新 “星夜暗色” 视觉主题：深空近黑背景、分层卡片、青蓝强调色、更高对比文字层级。
- 主页与系统设置改为 **彩色圆角应用图标徽标**（颜色自动压暗 30% 保证白字清晰）。
- 屏幕切换 **150 ms 淡入过渡**（原为无动画切换），进出应用更顺滑、更接近成熟系统手感。
- 主页音量 / 灯光快捷操作改为 **状态胶囊（toast）提示**，2 秒后自动淡出。
- 顶栏时间改为 14px 主字号，强化时间信息层级。
- 灯光 / 蓝牙 / WiFi 子页面统一为图标行，与主页、系统设置视觉一致。
- 虚拟键盘与屏保补上 **淡入/淡出过渡**。
- 重新启用屏保：主界面无按键 30 秒进入壁纸轮播（SD 卡壁纸优先，否则内置渐变/星空/波纹/几何/网格），任意键唤醒。
- 新增 `tools/check_balance.py`，用于提交前快速校验 C 源码括号 / 花括号配对。

### 改动
- `main/apps/lvgl_ui.h/.c`：主题调色板、尺寸与动效 token 集中管理，新增 `ui_app_icon()` 图标辅助。
- `main/apps/lvgl_menu.c`：主页应用图标 + 状态栏层级重构 + toast 提示，并接入屏保空闲检测。
- `main/apps/app_settings.c`、`app_led.c`、`app_ble.c`、`app_wifi.c`、`kbd.c`：统一图标行 / 淡入淡出。
- `main/services/screensaver.c`：屏保淡入 + 自动释放旧屏。
- `main/services/wallpaper.c`：移除不可靠的局域网 `192.168.1.8` 网络壁纸源，改为 SD 卡 > 内置兜底；修复扩展名大小写（TJPGD 敏感）。
- `main/CMakeLists.txt`：重新编译 `screensaver.c` / `wallpaper.c`。
- `main/version.h`：版本号 `v0.3` → `v1.0`；应用顶栏小字由占位 joke 改为版本号。

### 清理
- 移除未接线、未参与编译的死代码 `main/apps/app_login.c`（硬编码默认密码 `8888`）。
- 移除未参与编译的孤儿字体 `main/ui_font_lvgl_6.c`、`main/ui_font_lvgl_12.c`。
- 新增根目录 `LICENSE`（MIT）与 `CHANGELOG.md`；README 全面重写。

---

## 历史版本（归档）

> 以下内容整理自 `docs/` 下的历史发布说明，保留以便追溯。

### 2026-08-30（GitHub 版）
- 主界面右上角显示 `YYYY-MM-DD HH:MM`，仅分钟变化时刷新。
- 电子书移除 4 MB 整本载入限制，改 4 KB 流式 I/O + 动态页索引；支持 UTF-8/UTF-8 BOM/UTF-16 LE/BE/GBK/GB2312。
- 书签按书路径哈希后的 NVS key 独立保存，兼容旧版 `book/page` 迁移。
- 音乐完整曲库浏览（移除 5 首 / 24 首上限）、上一首 / 下一首、自然播完自动下一首并循环。
- 系统信息页实时查看空闲堆 / 最低空闲堆 / PSRAM。
- 新增 `CONFIG_IDF_TARGET="esp32s3"`，锁定 ESP32-S3，规避误配经典 ESP32。
- 修复 ESP-IDF 5.5.4 / GCC 14 下两处 `-Werror=format-truncation`。

### UI 重设计（2026）
- 统一 Apple Dark 视觉：深黑背景、分层灰卡片、系统蓝强调色、10px 页边距、8px 圆角。
- 主界面改为 4 项 / 页 + 顶部状态区 + 底部提示 + 页指示。
- 翻页动画 350ms → 180ms，并删除旧动画防止叠加。
- 修复 WiFi 多次扫描旧 AP 按钮无法删除、游戏大厅焦点误命中标题栏、BLE/设置/音乐列表末行重叠等问题。

### 音乐功能追加（2026）
- 曲库按实际曲目动态分配（优先 PSRAM），排序由冒泡改为 `qsort`。
- 5 行虚拟列表 + 底部 `6-10 / 87` 范围指示。
- 播放页左右键切歌、显示 `3 / 87` 序号、快速切歌竞态修复。

### 更早（硬件与稳定性关键修复，详见 docs/DEVNOTES.md）
- KEY6 根因定位：battery.c 将 GPIO 编号误当 ADC 通道号，导致 GPIO5 被配成模拟输入。
- LVGL 绘制缓冲改用内部 DMA 内存，修复 BLE/WiFi 后 SPI 刷新死锁。
- 蓝牙开关状态由 ADV_START/STOP 完成事件维护，开机仅初始化协议栈、绝不自动广播。
- SNTP 设置本地时区 `CST-8`，修复时间慢 8 小时。
- picolibc 下 64 位 `time_t` 越界修复（不再用 4 字节临时变量）。
- FATFS 打开 LFN + UTF-8，中文文件名不再乱码。
- 串口文件传输协议：小文件 PING/LIST/PUT/GET/DEL 直传，大文件改手动拷入 SD。