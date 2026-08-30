# 小孟蜜汁系统

基于 **ESP32-S3-WROOM-1-N16R8** 的 320×240 掌机 / 多媒体固件。界面使用 LVGL，支持电子书、音乐、图库、NES、Wi-Fi、BLE、灯光、系统信息和内置小游戏。

当前版本保持轻量 Apple Dark 风格，并持续针对 ESP32-S3 的内部 SRAM、PSRAM、SD 卡 I/O、音频解码和 LVGL 刷新做性能优化。

## 主要功能

- 主界面：应用中心、`YYYY-MM-DD HH:MM` 日期时间、电量、音量和灯光快捷操作
- 音乐：扫描 `/sdcard/music` 下全部 MP3/WAV；5 行虚拟列表；上一首/下一首；**歌曲自然播放结束后自动播放下一首并循环曲库**
- 电子书：扫描 `/sdcard/books` 下全部 TXT；支持 UTF-8、UTF-8 BOM、UTF-16 LE/BE、常见 GBK/GB2312
- 电子书大文件：流式分页，不把整本小说一次性载入 PSRAM；不再受旧版 4 MB 限制
- 阅读记忆：每本书独立保存阅读页码，并兼容旧版书签
- NES：扫描 `/sdcard/games` 下 ROM；中文文件名使用 UTF-8 FATFS + GB2312 字库显示
- 图库：JPG/JPEG 浏览与自动播放
- 系统设置：灯光、蓝牙、Wi-Fi、系统信息、按键测试

## 实体按键位置与逻辑名称

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

> SW2 在代码中沿用历史逻辑名 `KEY_BOOT`，它是正常的用户功能键；但上电/复位阶段按住 GPIO0 的行为仍由 ESP32-S3 ROM 决定。

## 各界面的按键功能

| 使用场景 | 五向开关 | KEY1 | KEY2 | KEY3 | KEY4 | SW2 | KEY5 | KEY6 |
|---|---|---|---|---|---|---|---|---|
| **主界面** | 浏览应用 | 已在主页，无额外动作 | 当前保留 | 无额外动作 | 打开选中应用 | 循环切换灯光模式 | 音量 +5% | 音量 -5% |
| **普通功能列表 / 系统设置** | 移动焦点 | 返回上一层/主页 | 由具体功能决定 | 返回 | 进入/确定 | 通常无动作 | 由具体功能决定 | 由具体功能决定 |
| **电子书书单** | 上下左右选书 | 返回并退出电子书 | 无 | 返回 | 打开选中书籍 | 无 | 无 | 无 |
| **电子书阅读** | 上/左=上一页；下/右=下一页 | 保存进度并退出 | 无 | 保存进度并返回 | 无 | 无 | 上一页 | 下一页 |
| **音乐列表** | 浏览全部歌曲 | 返回主页 | 无 | 返回 | 播放选中歌曲 | 无 | 无 | 无 |
| **音乐播放** | 左=上一首；右=下一首 | 停止并返回 | 无 | 停止并返回 | 播放/暂停 | 无 | 音量 +5% | 音量 -5% |
| **图库** | 左=上一张；右=下一张 | 自动播放开/关 | 无 | 返回 | 无 | 无 | 上一张 | 下一张 |
| **Wi-Fi** | 移动焦点 | 返回 | 无 | 返回 | 选择 AP / 连接 | 无 | 扫描；已连接行可断开 | 无 |
| **BLE** | 移动到广播/扫描行 | 返回 | 无 | 返回 | 无 | 无 | 开启/切换当前行 | 关闭/切换当前行 |
| **灯光设置** | 选择模式/颜色/亮度行 | 返回 | 无 | 返回 | 无 | 无 | 当前值增加 | 当前值减少 |
| **系统信息** | 无特殊动作 | 返回 | 无 | 返回 | 无 | 无 | 重新检测 SD 卡 | 无 |
| **虚拟键盘** | 移动光标 | 完成输入 | 无 | 退格 | 输入当前键 / 选字 | 无 | 下一输入模式 | 上一输入模式 |
| **NES ROM 列表** | 上下选择 ROM | 返回 | 无 | 返回 | 启动 ROM | 无 | 无 | 无 |
| **NES 游戏中** | NES 十字方向键 | **长按约 1 秒退出 NES** | 音量 +5% | NES SELECT | NES START | 音量 -5% | NES A | NES B |

### 音乐自动续播

当前歌曲**自然播放结束**后会自动切换到下一首；最后一首结束后回到第一首。手动上一首/下一首、暂停、停止返回不会被误判成“播放完毕”。

### 内置小游戏

- **贪吃蛇**：五向控制方向；KEY4 开始/暂停；KEY3 退出。
- **飞机射击**：左/右移动，上键射击；KEY4 暂停；KEY3 退出。
- **俄罗斯方块**：左/右移动，上键旋转，下键加速下落；KEY4 暂停；KEY3 退出。
- **打砖块**：左/右移动挡板；KEY4 发球/暂停；KEY3 退出。
- **入侵者**：左/右移动（KEY5/KEY6 也可移动），KEY4/上键开火；KEY1 暂停/继续；KEY3 退出。

## SD 卡目录

```text
/sdcard/
├── books/        # .txt 电子书
├── music/        # .mp3 / .wav
├── games/        # .nes ROM
├── gallery/      # .jpg / .jpeg
├── Pictures/     # 图库兼容目录
├── DCIM/         # 图库兼容目录
└── wallpaper/    # 壁纸/图库兼容目录
```

FATFS 启用 UTF-8 API 与长文件名（LFN），中文文件名可直接使用。

## 硬件

| 外设 | 型号/说明 | 接口/关键参数 |
|---|---|---|
| MCU | ESP32-S3-WROOM-1-N16R8 | 16 MB Flash + 8 MB OPI PSRAM |
| 屏幕 | ST7789 240×320（横屏 320×240） | SPI |
| SD 卡 | FAT32 | SDSPI |
| 音频 | MAX98357A | I2S |
| 状态灯 | WS2812B | RMT |
| 电池 | ADC 电压检测 | ADC1 |

具体引脚以 `main/board_config.h` 为准。

## 编译环境

推荐 **ESP-IDF 5.5.x**。项目只支持 **ESP32-S3**，根目录 `CMakeLists.txt` 已将 target 固定为 `esp32s3`，`sdkconfig.defaults` 同时包含：

```text
CONFIG_IDF_TARGET="esp32s3"
CONFIG_IDF_TARGET_ESP32S3=y
```

首次编译：

```bash
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

烧录与监视：

```bash
idf.py -p COM7 flash monitor
```

Linux/macOS 把 `COM7` 替换为实际串口，例如 `/dev/ttyUSB0`。

### 如果之前错误编译成 ESP32

如果日志出现：

```text
GPIO_NUM_41 undeclared
GPIO_NUM_45 undeclared
GPIO_NUM_47 undeclared
GPIO_NUM_48 undeclared
```

并且编译命令中出现 `xtensa-esp32-elf-gcc` / `components/.../esp32/`，说明旧目录缓存成了经典 ESP32 target，而不是 ESP32-S3。

建议执行：

```bash
idf.py fullclean
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

如果仍提示 target 冲突，删除自动生成的 `sdkconfig`、`sdkconfig.old` 和 `build/` 后重新执行。**不要删除 `sdkconfig.defaults`。**

本版本同时修复 ESP-IDF 5.5.4 / GCC 14 下两处 `-Werror=format-truncation`：日期格式化和电子书超长文件名错误提示。

## 流畅度 / 内存策略

- 主菜单、应用页切换时自动释放旧 LVGL screen 对象树，避免反复进入应用造成 UI 对象累积。
- 音乐列表和电子书列表采用固定 5 行虚拟列表，曲目/书籍很多时不会创建大量按钮。
- 音乐曲库元数据优先放 PSRAM；解码任务大栈优先放 PSRAM，保护 Wi-Fi/LVGL 所需内部 SRAM。
- 音乐播放页只在“状态或音量真正变化”时更新标签，不再固定频率重复让 LVGL 重绘。
- 音乐自然 EOF 使用轻量完成计数通知 UI 自动下一首，不轮询文件位置、不增加额外播放任务。
- 音乐暂停轮询由 50 ms 调整为 20 ms，停止等待检查粒度由 50 ms 调整为 10 ms，在不缩短最大安全等待时间的前提下提高切歌/返回响应。
- 电子书使用 4 KB 流式 I/O + 页索引，大型 TXT 不整本驻留 PSRAM。
- 主界面日期时间仅在分钟变化时刷新，电量低频刷新，减少无效重绘。
- NES 实时按键直接读取 GPIO，避免后台按键任务调度造成输入延迟。

## 电子书编码说明

阅读器自动识别：

- UTF-8
- UTF-8 BOM
- UTF-16 LE / UTF-16 BE
- 常见 GBK/GB2312 文本

正文最终转换为 LVGL 可显示的 UTF-8。固件内置 GB2312 中文字库，因此极少数 GBK 扩展汉字/生僻字不在字库中时会显示占位字符，但常用简体中文可正常显示。

## GitHub 仓库说明

建议提交：

```text
CMakeLists.txt
components/
main/
storage_data/
tools/
docs/
partitions.csv
sdkconfig.defaults
dependencies.lock
README.md
.gitignore
```

不要提交：

```text
build/
managed_components/
sdkconfig
sdkconfig.old
本机日志/临时文件
```

`main/idf_component.yml` + `dependencies.lock` 用于 ESP-IDF Component Manager 恢复 LVGL / esp_lvgl_port 等托管依赖，因此无需把 `managed_components/` 上传 GitHub。

## 本次补充修改

1. README 补齐 KEY1~KEY6、SW2、五向键的物理位置、GPIO、全局/各应用功能。
2. 音乐播放增加自然结束自动下一首，曲库末尾自动循环。
3. UI 中“NES 模拟器”统一改名为“NES”。
4. 应用公共顶栏原 `v0.3` 小标签替换为 **“不要狗叫”**；真实版本号仍保留在版本常量和设备信息中。
5. 音乐播放页减少重复 LVGL 文本更新，并提高暂停/停止响应粒度。
6. 工程 target 固定为 `esp32s3`，修复误配置成经典 ESP32 后 GPIO40~48 无法编译的问题。
7. 修复 GCC 14 的日期/电子书字符串截断构建错误。

## 许可与第三方组件

`components/mp3dec` 内含其原始许可证文件；LVGL、esp_lvgl_port 等依赖由 ESP-IDF Component Manager 管理。公开发布仓库前请按你的项目许可策略补充根目录 `LICENSE`。
