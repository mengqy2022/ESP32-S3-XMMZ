# 开发注意事项（血泪教训，开发中严禁重犯）

## LVGL 9 关键陷阱
1. **`lv_screen_load()` 不删除旧屏幕**（auto_del=false）！切换屏幕必须用 `ui_screen_show()`（内部 `lv_screen_load_anim(..., true)` + 先做分组清理）。否则每次切换泄漏一整屏对象 → 越来越卡 → "感觉有很多本书"。
2. **LVGL9 删除对象不会自动移出按键分组**！`ui_screen_show` 里必须先 `group_cleanup_obj(lv_scr_act())` 递归摘除旧屏所有对象，否则分组里留悬空指针 → 导航访问已释放内存 → 用 2-3 个功能后卡死。
3. **esp_lvgl_port 的 `full_refresh=1` 必须配整屏缓冲**（buffer_size == hres*vres），否则返回悬空 display 对象 → 空指针崩溃（StoreProhibited @0x0）。
4. **RGB565 字节序**：SPI 屏必须 `.flags.swap_bytes = 1`。
5. LVGL 渲染循环保持 **5ms 恒定节奏**，不要在按键等待里阻塞 20ms+。
6. 本机屏幕是 **320×240 横屏玻璃**（ST7789 控制器 240×320），逻辑屏 320×240，rotation swap_xy+mirror_y。

## 字体
7. 源码字符串是 **UTF-8**，渲染必须按 Unicode 码点查字库（font_cp_index 二分），不能按 GB2312 双字节解析。
8. `lv_font_conv` **不支持 TTC**（simsun.ttc 报错），用单文件 TTF（simhei.ttf）；大量字符用多个 `--symbols` 分块传。
9. 界面字库是**子集**（必须重新收集源码中文字符再生成），正文用全量 GB2312 书籍字库（bpp4）。任何新应用的中文字符都要进 ui_chars.txt 重新生成 14px 字体，否则 □ 乱码。

## 存储
10. **SPIFFS 深坑**：手动 spiffsgen 生成的镜像必须与驱动配置一致（--use-magic --use-magic-len 等），否则目录能读、数据页读取全 EIO（errno=5）；且 SPIFFS 的 fseek/ftell+fread 组合会返回 0 字节。**结论：内置数据直接编译进固件（books_data.c），不用 SPIFFS**。
11. SD 卡必须 FAT32（IDF fatfs 不支持 exFAT）；SD CS 用 GPIO10（模组 SD_CD 实为 CS）。

## 构建/配置
12. 改了 sdkconfig.defaults 后**必须删掉 sdkconfig 再编译**，否则默认值不生效（自定义分区表、BT、SPIRAM 等）。
13. 自定义分区表：`CONFIG_PARTITION_TABLE_CUSTOM=y` + filename。
14. **不要并发跑两个 ninja 构建**（会损坏 build 目录）。
15. 内部 RAM 只有 ~300KB：`CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=0`，LVGL 分配走 PSRAM。
16. xtensa 上**避免 GCC 嵌套函数**（trampoline 不稳定），用静态函数。
17. ESP32-S3 上 BLE 旧版广播 API（esp_ble_gap_start_advertising 等）需要 `CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y`。

## 硬件（本板）
18. **KEY6(GPIO5) 一直"按下"的真正根因（2026-08-24 最终定案）：battery.c 把 GPIO 编号当 ADC 通道号传**。`adc_oneshot_config_channel/read` 第二参数是 `adc_channel_t`（通道号），不是 GPIO 编号！ESP32-S3 映射：ADC1_CH3=GPIO4（电池）、ADC1_CH4=GPIO5（KEY6）。早期代码传 `GPIO_NUM_4`（数值=4）→ 实际配置成 **ADC_CHANNEL_4=GPIO5**，把 KEY6 引脚改成 ADC 模拟输入 → 数字读取失效 → 按键测试 KEY6 永远"按下"且按了没反应；同时电池读数一直在读按键脚（电压全错，之前"电池校准"无从谈起）。**必须用 `ADC_CHANNEL_3` 对应 GPIO4**（battery.c 已修复）。教训：**ESP32-S3 的 ADC1_CH0-9 对应 GPIO1-10，通道号≠GPIO 编号，切勿把 GPIO_NUM_x 当通道号传**。
19. **KEY6 已恢复全部功能（2026-08-24）**：清除了 8 处 "BOOT 替代 KEY6" 的绕过逻辑（电子书翻页、音量±、灯光数值、设置数值、kbd 模式、BLE、WiFi）；**BOOT 键改为主页灯光模式快速切换**（循环 关闭/常亮/呼吸/闪烁/彩虹，顶栏提示 2 秒，自动保存 NVS，见 lvgl_menu.c + led_ctrl_mode_name()）。
20. 五向开关曾虚焊，现已修好；导航=五向-上/下，KEY5=上翻备用，KEY4=确定，KEY3=返回，KEY1=主界面，KEY2=系统信息。
21. 电池分压 150k/830k≈0.1807；修复 ADC 通道后读数才是真实电池电压，仍需按万用表校准 BAT_DIVIDER_RATIO。

## 蓝牙
21. 蓝牙默认关闭，广播由 s_adv_requested 门控，开机只做协议栈初始化（不再运行时懒启动）。
22. **运行时 BT 初始化（WiFi 已激活时）会死锁/挂起**（coexist 问题）：BT 必须在开机早期、WiFi 之前初始化。
23. **激活 BT 后空闲时 SPI 刷新挂死**（main 卡在 lv_refr.c 的 wait_for_flushing 自旋 → WDT 复位）。真根因（非中断争抢）：**LVGL 绘制缓冲在 PSRAM 时，esp_lcd 的 SPI panel io 每次刷新都要临时分配"内部 DMA 拷贝缓冲"（spi_master setup_dma_priv_buffer），而 BLE+WiFi 初始化恰好发生在显示初始化之后、第一次刷新之前，吃光了内部 DMA RAM → 分配失败 → DMA 从未启动 → on_color_trans_done 永不触发 → 永久自旋**。日志特征：`spi_master: setup_dma_priv_buffer: Failed to allocate priv TX buffer` + `lcd_panel.io.spi: spi transmit (queue) color failed`。**解法：LVGL 绘制缓冲必须用内部 DMA 内存（esp_lvgl_port 配置 `.buff_dma=true, .buff_spiram=false`）**，刷新零分配零拷贝；曾误判为 BT 中断饿死 SPI 回调并尝试 `CONFIG_BT_*_PINNED_TO_CORE_1`（无效，教训：先看日志找首个失败点，别猜中断时序）。
24. 广播名 XiaoMeng-SYS，GATT 0xFFF0（设备信息读 + 电量读/通知）。
25. **同样的 DMA 拷贝陷阱也存在于 lcd_fill 初始化整屏填充**：整屏 153KB PSRAM 缓冲 → 单次 153KB 私有 DMA 拷贝必然失败（日志同款 `setup_dma_priv_buffer` 报错）。必须用**小块（8KB）内部 DMA 缓冲分带填充**，直达 DMA 零拷贝。
26. **蓝牙开关状态必须由 ADV_START/STOP_COMPLETE_EVT 维护**（`ble_set_adv` 只发请求）：曾因 `s_adv_on` 默认 true 且从不更新 → 看起来"默认开启且无法关闭/无法切换"。开机只初始化协议栈、**绝不自动广播**（s_adv_requested 门控 + 完成事件才置 s_adv_on）。
27. **顶栏时钟不要用"编译时间+运行时长"的假时钟**（用户反感乱跳的假时间）：NTP 同步前固定显示"时间未同步"，同步后显示真实时间。WiFi 5 次重试失败后不能永久放弃：加 30 秒周期的后台自动重连任务，网络恢复后自动连上并 SNTP 同步。
28. **SNTP 同步的是 UTC，必须设置本地时区**（`setenv("TZ","CST-8",1); tzset();` 在 sntp_start 里，北京时间 UTC+8），否则 localtime_r 显示的时间比北京时间慢 8 小时。
29. **ESP-IDF v5.5 (picolibc) 的 time_t 是 64 位**（`_TIME_T_ = __int_least64_t`）！严禁 `uint32_t now = (uint32_t)time(NULL); localtime_r((const time_t *)&now, ...)` —— 4 字节变量被当 8 字节读，越界 4 字节读到栈垃圾 → 时间戳随机（症状：日期乱跳如 09-32、分钟乱变）。必须 `time_t now = time(NULL); localtime_r(&now, &ti);`。

## 串口文件传输 (services/file_proto.c + tools/inventory_tool.py)
30. **控制台 UART0 默认不装 uart 驱动**：esp_vfs_console 走"直接寄存器"VFS（uart_rx_char/uart_tx_char）。因此：
    - `uart_read_bytes()` 等驱动 API **不可用**（无驱动实例），必须 `read(0,...)` 走 VFS；
    - **非阻塞读必须先 `fcntl(0, F_SETFL, O_NONBLOCK)`**，否则 read 永久忙等；
    - **原始数据发送用 `fwrite/printf + fflush`（newlib→VFS→直接寄存器），勿用 `write(1,...)`**——实测在 O_NONBLOCK 置位后 write(1,...) 持续返回 -1 卡死任务（fwrite 正常）。
31. **控制台 UART 默认做 \r\n 行结束转换**，会改写二进制中的 0x0A/0x0D！文件传输前必须：
    `uart_vfs_dev_port_set_rx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_LF); uart_vfs_dev_port_set_tx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_LF);`
    （include "driver/uart_vfs.h" + "driver/uart.h"，REQUIRES 加 esp_driver_uart）
32. **fatfs 默认 LFN 关闭（CONFIG_FATFS_LFN_NONE）+ CP437**：中文文件名在 readdir 里变短名乱码（如 `测试.txt`→`\xe6\xb5I\xe8\xafO.TXT`，且名对得上但显示错）。**sdkconfig.defaults 必须开 `CONFIG_FATFS_LFN_HEAP=y` + `CONFIG_FATFS_MAX_LFN=255`**（FF_LFN_UNICODE=2=UTF-8），改后删 sdkconfig 全量重建（注意保 target=esp32s3）。
33. 协议行读取不能"读到哪算哪"：必须**等 \n 或空闲超时**再整行处理，否则 115200 下命令被分片（`FILE:PUT`→BAD CMD、`FILE:DE`→UNKNOWN）。
34. **大文件串口直传不可行（2026-08-26 定案）**：内部 DMA RAM 仅 ~190KB（heap_init: 130+21+32+7 KiB）。给控制台 UART0 `uart_driver_install` 装驱动（环形缓冲）后，**sdmmc 的 DMA 缓冲被挤爆 → 连 LIST 都失败（`sdmmc_read_sectors: not enough mem, err=0x101`）**。结论：**不装 UART 驱动**（小文件 PING/LIST/PUT/GET/DEL 正常），**大文件（音乐/壁纸/书）改用手动拷入 SD**。
35. **SD 卡目录（应用读取目标，开机自动创建，见 sd_card.c）**：`/sdcard/music`(mp3)、`/sdcard/wallpaper`(jpg/png/bmp, 文件名≤40)、`/sdcard/books`(txt)、`/sdcard/warehouse/orders`(json)。
36. **栈上大数组会栈溢出**：fp_task 栈 4096 时，局部 `char chunk[4096]` 直接撑爆 → panic 重启（`rst:0xc`）。传输块缓冲必须 static；`sizeof(指针)`=4 陷阱——用 `sizeof(数组)` 取大小。
