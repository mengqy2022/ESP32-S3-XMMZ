/*
 * music_list.h — 音乐播放列表 (局域网 mp3 在线播放)
 *
 * 用法:
 *   1. 在自己电脑上放几个 mp3, 然后在该目录打开命令行运行:
 *        python -m http.server 8000
 *   2. 把下面的 URL 改成 电脑IP:端口/文件名 (ipconfig 查电脑 IP)
 *   3. 设备连上同一 WiFi, 在「音乐」应用里选曲播放
 *
 * 提示: 在线电台流 (http 直接返回 mp3 流) 也可以放进来
 */
#pragma once

typedef struct {
    const char *name;   /* 曲目名 (显示用) */
    const char *url;    /* http 地址 */
} music_track_t;

static const music_track_t s_music_tracks[] = {
    { "Test1", "http://192.168.1.8:8000/Test1.mp3" },
    { "Test2", "http://192.168.1.8:8000/Test2.mp3" },
    /* 在这里加你的曲目, 然后重新编译烧录 */
};

#define MUSIC_TRACK_COUNT (sizeof(s_music_tracks) / sizeof(s_music_tracks[0]))
