#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_book_font.py — 生成全量 GB2312 书籍字体 (LVGL 格式, 16px, 1bpp)
覆盖: ASCII + GB2312 全部 7445 字符 (含中文标点)
输出: ../main/book_font_lvgl.c
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "main", "book_font_lvgl.c")
FONT = r"C:\Windows\Fonts\simhei.ttf"

chars = set()
for q in range(1, 95):
    for w in range(1, 95):
        try:
            chars.add(bytes([0xA0 + q, 0xA0 + w]).decode("gb2312"))
        except UnicodeDecodeError:
            pass
print("GB2312 chars:", len(chars))

lst = sorted(chars)
# npx 绝对路径 (子进程不带 shell PATH)
NPX = r"D:\software\Programming\node-v23.8.0-win-x64\npx.cmd"
if not os.path.exists(NPX):
    NPX = "npx"
cmd = [
    NPX, "--yes", "lv_font_conv@1.5.3",
    "--font", FONT,
    "--size", "16",
    "--bpp", "4",
    "--format", "lvgl",
    "--no-compress", "--no-prefilter", "--no-kerning",
    "--force-fast-kern-format",
    "--range", "0x20-0x7E",
]
CHUNK = 700
for i in range(0, len(lst), CHUNK):
    cmd += ["--symbols", "".join(lst[i:i + CHUNK])]
cmd += ["-o", OUT]

print("running lv_font_conv...")
r = subprocess.run(cmd, capture_output=True, text=True)
print(r.stdout[-2000:] if r.stdout else "")
if r.returncode != 0:
    print("STDERR:", r.stderr[-2000:])
    sys.exit(1)
sz = os.path.getsize(OUT) if os.path.exists(OUT) else 0
print("OK:", OUT, sz, "bytes")
