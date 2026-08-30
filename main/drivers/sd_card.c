/*
 * sd_card.c — SD 卡驱动 (sdspi)
 * 原理图: CMD=GPIO11, CLK=GPIO13, DATA=GPIO9, SD_CD=GPIO10
 * 注: 该模组 SD_CD 大概率是 CS 片选脚而非检测开关, 因此:
 *   - 不做"CD 高=无卡"的前置拦截, 始终尝试挂载
 *   - CS 探测顺序: GPIO10(模组 CS) -> GPIO45(哑引脚, 若模组 CS 已硬接地)
 */
#include <string.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sd_card.h"
#include "board_config.h"

static const char *TAG = "sd_card";

static sdmmc_card_t *s_card = NULL;
static bool s_mounted = false;
static gpio_num_t s_active_cs = GPIO_NUM_NC;

bool sd_card_present(void)
{
    return s_mounted;
}

static esp_err_t try_mount(gpio_num_t cs_pin, uint32_t freq_khz)
{
    esp_err_t err;

    spi_bus_config_t bus = {
        .sclk_io_num = SD_PIN_CLK,
        .mosi_io_num = SD_PIN_CMD,
        .miso_io_num = SD_PIN_DATA,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    err = spi_bus_initialize(SD_SPI_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "spi bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    /* SD 规范要求 CMD/DAT(含 CLK/CS) 线有 10k-100k 上拉; 若 LCD 底座无外部上拉,
     * 卡完全不应答 → sdmmc_card_init 超时 (0x107=ESP_ERR_TIMEOUT).
     * 补内部上拉 (pad 级电阻, 与 SPI 功能不冲突). */
    gpio_pullup_en(SD_PIN_CLK);
    gpio_pullup_en(SD_PIN_CMD);
    gpio_pullup_en(SD_PIN_DATA);
    gpio_pullup_en(cs_pin);

    sdspi_device_config_t dev = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev.host_id = SD_SPI_HOST;
    dev.gpio_cs = cs_pin;
    dev.gpio_cd = SDSPI_SLOT_NO_CD;      /* 不依赖 CD 脚 */
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = freq_khz;

    esp_vfs_fat_sdmmc_mount_config_t mcfg = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024,
    };

    /* 挂载 (内部会打印 sdmmc_card_init 详细错误) */
    err = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &dev, &mcfg, &s_card);
    if (err == ESP_OK) {
        s_active_cs = cs_pin;
        s_mounted = true;
        ESP_LOGI(TAG, "SD 挂载成功: CS=GPIO%d %dkHz, size=%lluMB",
                 cs_pin, freq_khz,
                 ((uint64_t)s_card->csd.capacity * s_card->csd.sector_size) / (1024 * 1024));
    } else {
        /* 常见错误: 13(FR_NO_FILESYSTEM)=非 FAT32 格式 */
        ESP_LOGW(TAG, "SD 挂载失败: CS=GPIO%d %dkHz err=%s (卡若非 FAT32 需格式化)",
                 cs_pin, freq_khz, esp_err_to_name(err));
        spi_bus_free(SD_SPI_HOST);
    }
    return err;
}

/* CS 候选探测: GPIO10(模组CS?) -> GPIO45(哑脚), 40MHz / 20MHz / 5MHz / 400kHz 各试一次
 * 40MHz 优先 (电子书/音乐读取快一半); 失败自动回退低速.
 * 400kHz 是 SD 协议标准初始化频率, 部分卡/劣质布线在高速下不应答 */
static esp_err_t do_mount_probe(void)
{
    const gpio_num_t candidates[] = { SD_PIN_CS_PROBE_B /*GPIO10*/, SD_PIN_CS_PROBE_A /*GPIO45*/ };
    const uint32_t freqs[] = { 40000, 20000, 5000, 400 };
    for (int i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        for (int j = 0; j < sizeof(freqs) / sizeof(freqs[0]); j++) {
            if (try_mount(candidates[i], freqs[j]) == ESP_OK) return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t sd_card_init(void)
{
    /* CD 脚电平仅记录参考 (可能实为 CS 或未接) */
    gpio_config_t cd_cfg = {
        .pin_bit_mask = (1ULL << SD_PIN_CD),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cd_cfg);
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_LOGI(TAG, "CD(GPIO10) level=%d (参考, 不作拦截)", gpio_get_level(SD_PIN_CD));

    /* 诊断: 插入并上电的 SD 卡会通过卡内上拉把 DAT0(MISO=GPIO9) 拉高.
     * 若 DATA(MISO)=0 → 卡未供电或 MISO 线未连通 (查 FPC/卡座/3V3 供电). */
    gpio_config_t diag_cfg = {
        .pin_bit_mask = (1ULL << SD_PIN_CLK) | (1ULL << SD_PIN_CMD) | (1ULL << SD_PIN_DATA),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&diag_cfg);
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "SD 线电平(上拉后): CLK=%d CMD=%d DATA(MISO)=%d",
             gpio_get_level(SD_PIN_CLK), gpio_get_level(SD_PIN_CMD),
             gpio_get_level(SD_PIN_DATA));

    if (do_mount_probe() != ESP_OK) {
        ESP_LOGE(TAG, "SD 挂载失败: 检查卡是否 FAT32、接触/接线 (详见上面日志)");
        return ESP_FAIL;
    }

    /* 确保应用所需目录存在 (用户手动拷入文件的目标目录), 首次使用自动创建 */
    mkdir("/sdcard/music", 0755);                 /* 音乐 mp3 / wav(PCM16) */
    mkdir("/sdcard/gallery", 0755);               /* 图库 jpg/jpeg */
    mkdir("/sdcard/wallpaper", 0755);             /* 壁纸 jpg/png/bmp */
    mkdir("/sdcard/books", 0755);                 /* 电子书 txt */
    mkdir("/sdcard/warehouse", 0755);
    mkdir("/sdcard/warehouse/orders", 0755);      /* 仓库订单 json */
    mkdir("/sdcard/扫码", 0755);                   /* 扫码文件 (inventory_tool 查看/上传) */
    mkdir("/sdcard/games", 0755);                  /* ROM: 老街机游戏 (NES *.nes / CHIP-8 *.c8) */
    ESP_LOGI(TAG, "SD 目录就绪: /music /gallery /wallpaper /books /warehouse/orders /扫码 /games");
    return ESP_OK;
}

esp_err_t sd_card_remount(void)
{
    sd_card_unmount();
    ESP_LOGI(TAG, "运行中重新检测 SD...");
    if (do_mount_probe() == ESP_OK) {
        ESP_LOGI(TAG, "重新挂载成功!");
        return ESP_OK;
    }
    ESP_LOGE(TAG, "重新挂载失败");
    return ESP_FAIL;
}

void sd_card_unmount(void)
{
    if (s_mounted) {
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, s_card);
        s_mounted = false;
        s_card = NULL;
    }
}
