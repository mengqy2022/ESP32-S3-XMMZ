/*
 * screensaver.h — 屏保 (壁纸轮播)
 */
#pragma once

/* 运行屏保: 阻塞直到任意按键按下, 返回后调用者恢复界面 */
void screensaver_run(void);

/* 屏保壁纸切换间隔 ms */
#define SCREENSAVER_INTERVAL_MS  5000
