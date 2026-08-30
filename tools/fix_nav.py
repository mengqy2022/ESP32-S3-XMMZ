# -*- coding: utf-8 -*-
"""移除应用循环中重复的 KEY_UP/KEY_DOWN 处理 (导航交给 LVGL indev 单一路径)"""
import re

edits = {
    r"F:\01_ESP32-S3-soft\firmware\main\apps\lvgl_menu.c": [
        ("            case KEY_UP:\n                lv_group_focus_prev(lv_group_get_default());\n                break;\n            case KEY_DOWN:\n                lv_group_focus_next(lv_group_get_default());\n                break;\n", ""),
    ],
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_ebook.c": [
        ("            case KEY_UP:\n                lv_group_focus_prev(lv_group_get_default());\n                break;\n            case KEY_DOWN:\n                lv_group_focus_next(lv_group_get_default());\n                break;\n", ""),
    ],
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_led.c": [
        ("            case KEY_UP:\n                lv_group_focus_prev(lv_group_get_default());\n                break;\n            case KEY_DOWN:\n                lv_group_focus_next(lv_group_get_default());\n                break;\n", ""),
    ],
    r"F:\01_ESP32-S3-soft\firmware\main\apps\app_ble.c": [
        ("            case KEY_UP:\n                lv_group_focus_prev(lv_group_get_default());\n                break;\n            case KEY_DOWN:\n                lv_group_focus_next(lv_group_get_default());\n                break;\n", ""),
    ],
}
for f, subs in edits.items():
    t = open(f, encoding="utf-8").read()
    n = t
    for old, new in subs:
        n = n.replace(old, new)
    if n != t:
        open(f, "w", encoding="utf-8").write(n)
        print("cleaned:", f)
    else:
        print("no match:", f)
