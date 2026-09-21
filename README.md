# 小孟蜜汁系统 · Xiaomeng Mizhi System

> 基于 **ESP32-S3** 的 320×240 掌机 / 多媒体固件。深色、克制的 “星夜” 视觉体系 + LVGL 9，集电子书、音乐、图库、NES、Wi-Fi、BLE、灯光、系统信息与内置小游戏于一体。

<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/Platform-ESP32--S3-0A84FF">
  <img alt="UI" src="https://img.shields.io/badge/UI-LVGL%209-3DA5FF">
  <img alt="Framework" src="https://img.shields.io/badge/Framework-ESP--IDF%205.5-00A3E0">
  <img alt="Language" src="https://img.shields.io/badge/Language-C%2FC%2B%2B-555555">
  <img alt="License" src="https://img.shields.io/badge/License-MIT-32D74B">
</p>

## 目录

- [功能特性](#功能特性)
- [界面预览](#界面预览)
- [硬件](#硬件)
- [实体按键](#实体按键)
- [SD 卡目录](#sd-卡目录)
- [编译与烧录](#编译与烧录)
- [项目结构](#项目结构)
- [设计体系](#设计体系)
- [性能与内存策略](#性能与内存策略)
- [仓库规范](#仓库规范)
- [更新日志](#更新日志)
- [许可与第三方](#许可与第三方)

## 功能特性

| 模块 | 说明 |
|---|---|
| **主界面** | 彩色应用图标、`YYYY-MM-DD HH:MM` 实时时间、电量、音量 / 灯光快捷操作与状态胶囊提示 |
| **电子书** | 扫描 `/sdcard/books` 下全部 TXT；支持 UTF-8、UTF-8 BOM、UTF-16 LE/BE、常见 GBK/GB2312 |
| **电子书大文件** | 4 KB 流式分页 + 页索引，不整本载入 PSRAM，摆脱旧版 4 MB 限制 |
| **阅读记忆** | 每本书独立保存阅读页码，兼容旧版书签 |
| **音乐** | 扫描 `/sdcard/music` 下全部 MP3/WAV；5 行虚拟列表；切歌 / 自然播完自动下一首并循环曲库 |
| **图库** | JPG/JPEG 浏览与自动播放 |
| **屏保** | 主界面无按键 30 秒进入壁纸轮播（SD 卡 `/sdcard/wallpaper` 壁纸优先，内置渐变 / 星空等兜底），任意键唤醒 |
| **NES** | 扫描 `/sdcard/games` 下 ROM；中文文件名 UTF-8 FATFS + GB2312 字库 |
| **游戏** | 内置贪吃蛇、飞机射击、俄罗斯方块、打砖块、入侵者 |
| **Wi-Fi** | 扫描 / 连接 / 断开；保存密码；SNTP 自动对时（UTC+8） |
| **BLE** | 广播 `XiaoMeng-SYS` + GATT 设备信息 / 电量读取与通知 |
| **系统设置** | 灯光、蓝牙、Wi-Fi、系统信息、按键测试 |
| **串口文件传输** | PC 工具经 CH340 直接读写 SD 卡，无需拔卡 |

## 界面预览

> 屏幕截图请放入 [`docs/screenshots/`](docs/screenshots/)（建议命名 `home.png`、`music.png`、`ebook.png`、`nes.png`、`settings.png` 等），
> 便于在 README 中直接引用展示。

## 硬件

| 外设 | 型号 / 说明 | 接口 / 关键参数 |
|---|---|---|
| MCU | ESP32-S3-WROOM-1-N16R8 | 16 MB Flash + 8 MB OPI PSRAM |
| 屏幕 | ST7789 240×320（横屏 320×240） | SPI, 80 MHz |
| SD 卡 | FAT32 | SDSPI（独立 SPI3 总线） |
| 音频 | MAX98357A | I2S |
| 状态灯 | WS2812B | RMT |
| 电池 | ADC 电压检测 | ADC1_CH3 (GPIO4) |

具体引脚以 [`main/board_config.h`](main/board_config.h) 为准（已内置 ESP32-S3 编译期保护）。

## 实体按键

设备按面板位置定义：

```text
┌──────────────────────────────┐
│ 左上: KEY1  KEY2    KEY3  KEY4 :右上 │
│                              │
│        五向开关 / 方向键          │
│                              │
│ 左下: SW2          KEY5 :右下上   │
│                    KEY6 :右下下   │
└──────────────────────────────┘
```

| 面板按键 | 固件逻辑 | GPIO | 常用角色 |
|---|---|---:|---|
| KEY1 | `KEY_MENU / KEY_HOME` | 18 | 主界面 / 完成 / 特殊功能 |
| KEY2 | `KEY_OPTION` | 8 | 选项；NES 中音量 + |
| KEY3 | `KEY_SELECT / KEY_BACK` | 16 | 返回；NES 中 SELECT |
| KEY4 | `KEY_START / KEY_CONFIRM` | 17 | 确定 / 打开；NES 中 START |
| SW2 | `KEY_BOOT` | 0 | 主页灯光快捷；NES 中音量 - |
| KEY5 | `KEY_A` | 15 | 增加 / 上一项；NES A |
| KEY6 | `KEY_B` | 5 | 减少 / 下一项；NES B |
| 五向左 | `KEY_LEFT` | 19 | 左移 / 上一首 / 上一张 |
| 五向右 | `KEY_RIGHT` | 6 | 右移 / 下一首 / 下一张 |
| 五向上 | `KEY_UP` | 7 | 上移 / 上翻 |
| 五向下 | `KEY_DOWN` | 20 | 下移 / 下翻 |

> SW2 在代码中沿用历史逻辑名 `KEY_BOOT`，它是正常的用户功能键；但上电 / 复位阶段按住 GPIO0 的行为仍由 ESP32-S3 ROM 决定。

各应用内的详细按键矩阵见 [`docs/DEVNOTES.md`](docs/DEVNOTES.md) 与 README 旧版说明。

## SD 卡目录

```text
/sdcard/
├── books/        # .txt 电子书
├── music/        # .mp3 / .wav
├── games/        # .nes ROM
├── gallery/      # .jpg / .jpeg
├── Pictures/     # 图库兼容目录
├── DCIM/         # 图库兼容目录
└── wallpaper/    # 壁纸 / 图库兼容目录
```

FATFS 启用 UTF-8 API 与长文件名（LFN），中文文件名可直接使用。

## 编译与烧录

推荐 **ESP-IDF 5.5.x**。项目仅支持 **ESP32-S3**，根目录 `CMakeLists.txt` 已将 target 固定为 `esp32s3`。

```bash
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

烧录与监视（Windows 示例，把 `COM7` 换成实际串口，Linux/macOS 用 `/dev/ttyUSB0` 等）：

```bash
idf.py -p COM7 flash monitor
```

> ESP-IDF Component Manager 会根据 [`main/idf_component.yml`](main/idf_component.yml) 与 `dependencies.lock`
> 自动恢复 LVGL / esp_lvgl_port 等托管依赖，无需上传 `managed_components/`。

### 如果曾错误编译成经典 ESP32

出现 `GPIO_NUM_41/45/47/48 undeclared` 且编译命令含 `xtensa-esp32-elf-gcc`，说明缓存成了经典 ESP32 target：

```bash
idf.py fullclean
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

必要时删除自动生成的 `sdkconfig`、`sdkconfig.old`、`build/` 后重试。**不要删除 `sdkconfig.defaults`。**

## 项目结构

```text
.
├── CMakeLists.txt            # 顶层构建（target 锁定 esp32s3）
├── sdkconfig.defaults        # 默认配置（BT / SPIRAM / 分区表 / FATFS LFN 等）
├── partitions.csv            # 自定义分区表
├── dependencies.lock         # 托管依赖锁文件
├── main/
│   ├── app_main.c            # 启动流程：自检 → LVGL → 主菜单
│   ├── board_config.h        # 引脚定义（硬件唯一事实来源）
│   ├── version.h             # 版本号 / 品牌名
│   ├── lvgl_port.c           # LVGL 9 + esp_lvgl_port 移植（单线程驱动）
│   ├── ui/                   # 旧版轻量 UI 引擎（帧缓冲绘制）
│   ├── apps/                 # 各应用 + 主页 + 公共主题 + 虚拟键盘
│   ├── services/             # LED / BLE / WiFi / 音乐 / 屏保 / 壁纸 / 设置 / 文件传输
│   ├── drivers/              # LCD / 按键 / 灯 / 电池 / 音频 / SD
│   ├── nes/                  # nofrendo NES 核心（含全部 mapper）
│   └── *.c                   # 内嵌字体 / GB2312 书籍字库 / 拼音表 / 内置书
├── components/mp3dec/        # Helix MP3 解码器（第三方）
├── tools/                    # 字体 / 拼音 / 图片 / 数据生成与检查脚本
├── storage_data/             # 内置示例数据
└── docs/                     # 开发笔记 / 设计体系 / 发布记录 / 截图
```

## 设计体系

- **主题**：深空近黑背景 `0x0A0A0E` + 分层卡片 + 高对比白字 + 青蓝强调色 `0x3DA5FF`。
- **图标**：彩色圆角徽标（颜色自动压暗 30%）+ 居中单字 / 字母，统一用于主页与系统设置。
- **动效**：屏幕切换 150 ms 淡入、主页翻页 180 ms 缓动，克制而流畅。
- **约束**：只使用纯色 / 细描边 / 超短动画，不使用大阴影、透明模糊与全屏渐变，保证 ESP32-S3 帧率。

完整设计规范见 [`docs/DESIGN.md`](docs/DESIGN.md)。

## 性能与内存策略

- 主菜单、应用页切换时自动释放旧 LVGL screen 对象树，避免 UI 对象累积泄漏。
- 音乐列表 / 电子书列表采用 5 行虚拟列表，曲目 / 书籍很多时不会创建大量按钮。
- 音乐曲库元数据优先放 PSRAM；解码任务大栈优先放 PSRAM，保护 Wi-Fi / LVGL 所需内部 SRAM。
- 音乐播放页只在状态 / 音量真正变化时更新标签，不做固定频率重复重绘。
- 音乐自然 EOF 用轻量完成计数通知 UI 自动下一首，不轮询文件位置。
- 电子书 4 KB 流式 I/O + 页索引，大型 TXT 不整本驻留 PSRAM。
- 主界面日期时间仅在分钟变化时刷新，电量低频刷新，减少无效重绘。
- NES 实时按键直接读取 GPIO，避免后台按键任务调度造成输入延迟。
- LVGL 绘制缓冲强制内部 DMA 内存（`buff_dma=true`），刷新零分配零拷贝，免疫 BLE/WiFi 挤占 DMA RAM。

## 仓库规范

**应提交：**

```text
CMakeLists.txt  main/  components/  storage_data/  tools/  docs/
partitions.csv  sdkconfig.defaults  dependencies.lock
README.md  LICENSE  CHANGELOG.md  CONTRIBUTING.md  .gitignore
```

**不应提交：**

```text
build/  managed_components/  sdkconfig  sdkconfig.old  本机日志 / 临时文件
```

## 更新日志

版本演进见 [`CHANGELOG.md`](CHANGELOG.md)。

## 参与贡献

欢迎提出 Issue / PR。请先阅读 [`docs/DESIGN.md`](docs/DESIGN.md) 与 [`docs/DEVNOTES.md`](docs/DEVNOTES.md)，并按 [`CONTRIBUTING.md`](CONTRIBUTING.md) 的提交检查清单自检。

## 许可与第三方

本项目采用 [MIT License](LICENSE)。

- [`components/mp3dec`](components/mp3dec) 内含其原始许可证文件（Helix MP3 解码器）。
- LVGL、esp_lvgl_port 等依赖由 ESP-IDF Component Manager 管理。
- NES 模拟核心来自 nofrendo（见 `main/nes/nofrendo/`）。