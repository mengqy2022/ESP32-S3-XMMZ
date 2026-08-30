#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
img2rgb565.py — 把动漫图片批量转成 ESP32 屏保壁纸 (320x240 RGB565)
用法 (推荐, 自动命名 wall1~wallN):
    python img2rgb565.py 图1.jpg 图2.jpg 图3.jpg ... 图10.jpg

输出: 当前目录 wall1.rgb565, wall2.rgb565 ... wallN.rgb565
(设备固件里的 URL 就是 wall1~wall10.rgb565, 无需手动改名)

依赖: pip install pillow
"""
import os
import sys

from PIL import Image

W, H = 320, 240


def convert(src, idx):
    img = Image.open(src).convert("RGB")
    # cover 模式: 等比缩放 + 居中裁剪到 320x240, 不变形
    iw, ih = img.size
    scale = max(W / iw, H / ih)
    nw, nh = int(iw * scale + 0.5), int(ih * scale + 0.5)
    img = img.resize((nw, nh), Image.LANCZOS)
    x0, y0 = (nw - W) // 2, (nh - H) // 2
    img = img.crop((x0, y0, x0 + W, y0 + H))
    out = f"wall{idx}.rgb565"
    with open(out, "wb") as f:
        for y in range(H):
            for x in range(W):
                r, g, b = img.getpixel((x, y))
                c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                f.write(bytes([c >> 8, c & 0xFF]))
    print(f"OK: {out}  <- {os.path.basename(src)}")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    if len(sys.argv) > 11:
        print(f"最多 10 张, 当前 {len(sys.argv) - 1} 张")
        sys.exit(1)
    for i, p in enumerate(sys.argv[1:], start=1):
        try:
            convert(p, i)
        except Exception as e:
            print(f"FAIL {p}: {e}")
    print("完成! 把 wall1~wall10.rgb565 放到 http 服务器目录即可")


if __name__ == "__main__":
    main()
