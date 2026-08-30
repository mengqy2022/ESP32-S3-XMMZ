/*
 * ble.c — BLE GATT 服务 (Bluedroid)
 * 广播名: XiaoMeng-SYS
 * 服务 0xFFF0: 0xFFF1 设备信息(读)  0xFFF2 电量%(读+通知)
 */
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "battery.h"
#include "ble.h"
#include "version.h"
#include "safe_string.h"

static const char *TAG = "ble";

#define BLE_NAME               "XiaoMeng-SYS"
#define SVC_INST_ID            0

static uint16_t s_conn_id = 0xFFFF;
static uint16_t s_batt_val_handle = 0;
static esp_gatt_if_t s_gatts_if = 0;
static bool s_active = false;
static bool s_inited = false;          /* 蓝牙是否已初始化 (默认关闭, 首次使用才启动) */
static bool s_adv_requested = false;   /* 初始化完成后要广播 */
static bool s_scan_requested = false;  /* 初始化完成后要扫描 */
static volatile bool s_init_running = false;

static void ble_ensure(void);   /* 懒启动: 确保蓝牙已初始化 (异步) */

/* ---- 服务表 ---- */
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t char_decl_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t char_cfg_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;

static const uint16_t svc_uuid = 0xFFF0;
static const uint16_t devinfo_uuid = 0xFFF1;
static const uint16_t battery_uuid = 0xFFF2;

static char s_dev_info[24];
static uint8_t s_battery_level = 0;

static esp_gatts_attr_db_t s_gatt_db[] = {
    /* 服务声明 */
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
                            sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&svc_uuid} },
    /* 特性: 设备信息 (读) */
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_decl_uuid, ESP_GATT_PERM_READ,
                            1, 1, (uint8_t *)&char_prop_read} },
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&devinfo_uuid, ESP_GATT_PERM_READ,
                            sizeof(s_dev_info), sizeof(s_dev_info), (uint8_t *)s_dev_info} },
    /* 特性: 电量 (读+通知) */
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_decl_uuid, ESP_GATT_PERM_READ,
                            1, 1, (uint8_t *)&char_prop_read_notify} },
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&battery_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                            sizeof(uint8_t), sizeof(uint8_t), &s_battery_level} },
    { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_cfg_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                            sizeof(uint16_t), sizeof(uint16_t), (uint8_t[]){0, 0} } },
};

static esp_ble_adv_params_t s_adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t s_adv_data[31];
static uint8_t s_adv_len = 0;

static void build_adv_data(void)
{
    s_adv_data[0] = 0x02; s_adv_data[1] = 0x01; s_adv_data[2] = 0x06;   /* Flags */
    int nlen = strlen(BLE_NAME);
    if (nlen > 20) nlen = 20;
    s_adv_data[3] = nlen + 1; s_adv_data[4] = 0x09;                      /* Complete name */
    memcpy(&s_adv_data[5], BLE_NAME, nlen);
    s_adv_len = 5 + nlen;
}

static void gatts_event(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        s_gatts_if = gatts_if;
        ESP_LOGI(TAG, "registered, create attr tab");
        esp_ble_gatts_create_attr_tab(s_gatt_db, gatts_if,
                                      sizeof(s_gatt_db) / sizeof(esp_gatts_attr_db_t), SVC_INST_ID);
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
        esp_ble_gatts_cb_param_t *p = param;
        ESP_LOGI(TAG, "attr tab created, num=%d", p->add_attr_tab.num_handle);
        for (int i = 0; i < p->add_attr_tab.num_handle; i++) {
            if (p->add_attr_tab.handles[i]) {
                ESP_LOGI(TAG, "handle[%d]=0x%04x", i, p->add_attr_tab.handles[i]);
                if (i == 4) s_batt_val_handle = p->add_attr_tab.handles[i];  /* 电量值句柄 */
            }
        }
        esp_ble_gatts_start_service(p->add_attr_tab.handles[0]);
        break;
    }

    case ESP_GATTS_CONNECT_EVT:
        s_conn_id = param->connect.conn_id;
        s_active = true;
        ESP_LOGI(TAG, "connected conn=%d", s_conn_id);
        break;

    case ESP_GATTS_DISCONNECT_EVT:
        s_conn_id = 0xFFFF;
        s_active = false;
        ESP_LOGI(TAG, "disconnected");
        if (s_adv_requested) esp_ble_gap_start_advertising(&s_adv_params);
        break;

    default:
        break;
    }
}

/* ---- 扫描与广播开关 ---- */
static ble_dev_t s_devs[BLE_SCAN_MAX_DEV];
static int s_dev_count = 0;
static bool s_scanning = false;
static bool s_adv_on = false;   /* 广播默认关闭! 由 ADV start/stop 完成事件维护真实状态 */

static esp_ble_scan_params_t s_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
};

bool ble_adv_on(void) { return s_adv_on; }

esp_err_t ble_set_adv(bool on)
{
    s_adv_requested = on;
    if (!s_inited) {
        ble_ensure();   /* 异步初始化, 完成后按 s_adv_requested 广播 */
        return ESP_OK;
    }
    if (on) {
        esp_ble_gap_start_advertising(&s_adv_params);
        ESP_LOGI(TAG, "adv start requested");
    } else {
        esp_ble_gap_stop_advertising();
        ESP_LOGI(TAG, "adv stop requested");
    }
    /* 真实状态 (s_adv_on) 由 ADV_START/STOP_COMPLETE_EVT 更新 */
    return ESP_OK;
}

static void scan_add_device(const uint8_t *addr, const uint8_t *adv, uint8_t adv_len, int rssi)
{
    /* 提取名字 */
    uint8_t nlen = 0;
    uint8_t *name = esp_ble_resolve_adv_data_by_type((uint8_t *)adv, adv_len,
                                                     ESP_BLE_AD_TYPE_NAME_CMPL, &nlen);
    if (!name) {
        name = esp_ble_resolve_adv_data_by_type((uint8_t *)adv, adv_len,
                                                ESP_BLE_AD_TYPE_NAME_SHORT, &nlen);
    }
    char namebuf[20] = "未知设备";
    bool has_name = false;
    if (name && nlen > 0) {
        if (nlen > 19) nlen = 19;
        memcpy(namebuf, name, nlen);
        namebuf[nlen] = 0;
        has_name = true;
    }
    /* 查重/更新 */
    for (int i = 0; i < s_dev_count; i++) {
        if (memcmp(s_devs[i].addr, addr, 6) == 0) {
            s_devs[i].rssi = rssi;
            if (has_name) { xm_strlcpy(s_devs[i].name, namebuf, sizeof(s_devs[i].name)); }
            return;
        }
    }
    if (s_dev_count < BLE_SCAN_MAX_DEV) {
        ble_dev_t *d = &s_devs[s_dev_count++];
        memcpy(d->addr, addr, 6);
        d->rssi = rssi;
        d->has_name = has_name;
        xm_strlcpy(d->name, namebuf, sizeof(d->name));
        ESP_LOGI(TAG, "found: %s rssi=%d", d->name, rssi);
    }
}

void ble_scan_start(uint32_t duration_ms)
{
    if (s_scanning) return;
    if (!s_inited) {
        s_scan_requested = true;
        ble_ensure();   /* 异步初始化, 完成后自动开始扫描 */
        return;
    }
    s_dev_count = 0;
    esp_ble_gap_set_scan_params(&s_scan_params);
    esp_ble_gap_start_scanning(duration_ms);
    s_scanning = true;
    ESP_LOGI(TAG, "scan start %lu ms", (unsigned long)duration_ms);
}

void ble_scan_stop(void)
{
    if (!s_scanning) return;
    esp_ble_gap_stop_scanning();
    s_scanning = false;
    ESP_LOGI(TAG, "scan stop, %d devices", s_dev_count);
}

bool ble_scanning(void) { return s_scanning; }
int ble_scan_count(void) { return s_dev_count; }
const ble_dev_t *ble_scan_get(int idx)
{
    return (idx >= 0 && idx < s_dev_count) ? &s_devs[idx] : NULL;
}

static void gap_event(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        /* 只在用户请求广播时才自动开始 (开机初始化不广播) */
        if (s_adv_requested) esp_ble_gap_start_advertising(&s_adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            s_adv_on = true;
            ESP_LOGI(TAG, "adv on");
        } else {
            s_adv_on = false;
            ESP_LOGW(TAG, "adv start failed status=%d", param->adv_start_cmpl.status);
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        s_adv_on = false;
        ESP_LOGI(TAG, "adv off");
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
            scan_add_device(param->scan_rst.bda, param->scan_rst.ble_adv,
                            param->scan_rst.adv_data_len, param->scan_rst.rssi);
        } else if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
            s_scanning = false;
            ESP_LOGI(TAG, "scan complete, %d devices", s_dev_count);
        }
        break;
    /* ---- 配对/安全事件 (PC/手机连接 BLE 必走, 必须响应!) ---- */
    case ESP_GAP_BLE_SEC_REQ_EVT:
        ESP_LOGI(TAG, "pairing req, accept (Just Works)");
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "pairing %s (fail_reason=%d)",
                 param->ble_security.auth_cmpl.success ? "OK" : "FAIL",
                 (int)param->ble_security.auth_cmpl.fail_reason);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
        ESP_LOGI(TAG, "passkey: %06lu", (unsigned long)param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_PASSKEY_REQ_EVT:
        /* 主机请求输入 PIN: 回固定 123456 (IO=NONE 时一般不会触发) */
        esp_ble_passkey_reply(param->ble_security.ble_req.bd_addr, true, 123456);
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* 数字比较确认: 直接接受 */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
        break;
    default:
        break;
    }
}

static void ble_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        if (s_conn_id != 0xFFFF && s_batt_val_handle) {
            s_battery_level = battery_percent();
            esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_batt_val_handle,
                                        1, &s_battery_level, false);
        }
    }
}

static esp_err_t ble_init_internal(void)
{
    snprintf(s_dev_info, sizeof(s_dev_info), "XiaoMeng %.20s", SYS_VERSION_STR);
    s_battery_level = battery_percent();

    ESP_LOGI(TAG, "init: mem release");
    esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "mem release: %s", esp_err_to_name(ret)); return ret; }

    ESP_LOGI(TAG, "init: controller");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) { ESP_LOGE(TAG, "ctrl init: %s", esp_err_to_name(ret)); return ret; }
    ESP_LOGI(TAG, "init: controller enable");
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) { ESP_LOGE(TAG, "ctrl enable: %s", esp_err_to_name(ret)); return ret; }
    ESP_LOGI(TAG, "init: bluedroid");
    ret = esp_bluedroid_init();
    if (ret) { ESP_LOGE(TAG, "bluedroid init: %s", esp_err_to_name(ret)); return ret; }
    ret = esp_bluedroid_enable();
    if (ret) { ESP_LOGE(TAG, "bluedroid enable: %s", esp_err_to_name(ret)); return ret; }

    esp_ble_gatts_register_callback(gatts_event);
    esp_ble_gap_register_callback(gap_event);
    esp_ble_gatts_app_register(0);

    /* 安全参数: NoInputNoOutput + 允许绑定 → Just Works 配对
     * (PC/手机可直接连接, 无需输入 PIN; 不配这个 Windows 配对会卡住/失败) */
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(iocap));
    esp_ble_auth_req_t auth = ESP_LE_AUTH_BOND;   /* 仅绑定, 无 MITM → Just Works, 任何主机可连 */
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth, sizeof(auth));

    build_adv_data();
    esp_ble_gap_config_adv_data_raw(s_adv_data, s_adv_len);

    xTaskCreate(ble_task, "ble", 3072, NULL, 4, NULL);
    s_inited = true;
    s_init_running = false;
    ESP_LOGI(TAG, "BLE init done: %s", BLE_NAME);
    return ESP_OK;
}

/* BLE 初始化任务 (异步, 不在主任务里做, 避免卡死 UI) */
static void ble_init_task(void *arg)
{
    ESP_LOGI(TAG, "init task start");
    if (ble_init_internal() == ESP_OK) {
        if (s_adv_requested) {
            ESP_LOGI(TAG, "init task: start adv");
            esp_ble_gap_start_advertising(&s_adv_params);
        }
        if (s_scan_requested) {
            ESP_LOGI(TAG, "init task: start scan");
            s_dev_count = 0;
            esp_ble_gap_set_scan_params(&s_scan_params);
            esp_ble_gap_start_scanning(10000);
            s_scanning = true;
        }
    } else {
        s_init_running = false;
        ESP_LOGE(TAG, "BLE init FAILED");
    }
    vTaskDelete(NULL);
}

/* 确保蓝牙已初始化 (异步懒启动) */
static void ble_ensure(void)
{
    if (s_inited) return;
    if (s_init_running) return;
    s_init_running = true;
    xTaskCreate(ble_init_task, "ble_init", 6144, NULL, 4, NULL);
}

esp_err_t ble_start(void)
{
    /* 开机同步初始化 (此时 WiFi 尚未运行, 不会共存死锁);
     * 注意: 只初始化, 不广播 — 广播由「蓝牙」功能栏控制 */
    if (s_inited) return ESP_OK;
    return ble_init_internal();
}

bool ble_inited(void) { return s_inited; }

bool ble_is_active(void)
{
    return s_active;
}
