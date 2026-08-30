/*
 * file_proto.h — 串口文件传输协议服务 (PC ↔ 设备 SD 卡)
 * 协议 (文本行, 115200 8N1, 与控制台同口):
 *   FILE:PING              -> FILE:PONG
 *   FILE:LIST|<dir>        -> FILE:ENTRY|<name>|<size>|<type> ... FILE:DONE
 *   FILE:PUT|<path>|<size> -> FILE:READY, 接收 size 字节原始数据, -> FILE:OK
 *   FILE:GET|<path>        -> FILE:READY|<size>, 发送 size 字节原始数据, -> FILE:DONE
 *   FILE:DEL|<path>        -> FILE:OK / FILE:ERR|<原因>
 * 日志行 (形如 I (123) tag: ...) 由 PC 端自动过滤; 路径限 /sdcard 下.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t file_proto_start(void);   /* 启动协议任务 (需在 SD 初始化后调用) */

/* 是否有文件传输 (PUT/GET) 在进行; 主菜单用它跳过屏保, 避免壁纸抢占 SD/内存 */
bool file_proto_active(void);
