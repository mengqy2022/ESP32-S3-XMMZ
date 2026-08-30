# -*- coding: utf-8 -*-
"""在各应用循环的按键 switch 中恢复方向键导航 (队列驱动)"""
import re

nav_case = """            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                ui_nav_key(evt.key);
                break;
"""

targets = {
    r"F:\01_ESP32-S3-soft\firmware\main\apps\lvgl_menu.c":
        "            case KEY_CONFIRM:   /* 物理 KEY4: 确定 */",
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_ebook.c":
        "            case KEY_CONFIRM: {",
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_led.c":
        "            case KEY_A:   /* KEY5: 值 +1 */",
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_ble.c":
        "            case KEY_A:",
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_wifi.c":
        "            case KEY_A: {   /* KEY5: 焦点行对应动作 */",
}
for f, anchor in targets.items():
    t = open(f, encoding="utf-8").read()
    if "ui_nav_key(evt.key)" in t:
        print("already has nav:", f)
        continue
    if anchor not in t:
        print("ANCHOR NOT FOUND:", f, "->", anchor[:40])
        continue
    n = t.replace(anchor, nav_case + anchor, 1)
    open(f, "w", encoding="utf-8").write(n)
    print("nav restored:", f)
