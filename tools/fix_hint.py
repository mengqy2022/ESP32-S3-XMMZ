# -*- coding: utf-8 -*-
import io

edits = [
    (r"F:\01_ESP32-S3-soft\firmware\main\apps\app_led.c",
     'ui_screen_new("灯光设置")', 'ui_screen_new_ex("灯光设置", false)'),
    (r"F:\01_ESP32-S3-soft\firmware\main\apps\app_ble.c",
     'ui_screen_new("蓝牙")', 'ui_screen_new_ex("蓝牙", false)'),
]
for f, old, new in edits:
    t = open(f, encoding="utf-8").read()
    if old in t:
        open(f, "w", encoding="utf-8").write(t.replace(old, new))
        print("OK:", f, "->", new)
    else:
        print("NOT FOUND in", f)
