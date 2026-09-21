# 设计体系 · DESIGN SYSTEM

本文档是 “小孟蜜汁系统” 界面的单一事实来源，任何 UI 改动都应先从这里对齐风格，再改代码。

## 1. 目标

在 **320×240 横屏 + ESP32-S3 + ST7789 + LVGL 9** 的硬件约束下，实现：

- **高级**：深色、分层、克制的现代视觉，而不是五颜六色或大阴影堆砌。
- **流畅**：所有动效短而克制，渲染成本极低，长时间运行不掉帧。
- **一致**：不同应用复用同一套 token、样式与辅助函数。

## 2. 调色板

定义于 [`main/apps/lvgl_ui.h`](../main/apps/lvgl_ui.h)。核心：

| Token | 色值 | 用途 |
|---|---|---|
| `UI_THEME_BG` | `0x0A0A0E` | 全局背景（近黑，带极淡冷调） |
| `UI_THEME_BAR` | `0x121218` | 顶栏 / 底栏 |
| `UI_THEME_CARD` | `0x1B1B23` | 卡片 / 按钮 |
| `UI_THEME_CARD_FOC` | `0x262633` | 聚焦卡片（轻微提亮） |
| `UI_THEME_ACCENT` | `0x3DA5FF` | 系统强调色（青蓝） |
| `UI_THEME_ACCENT_DIM` | `0x17406B` | 强调色深底 |
| `UI_THEME_TEXT` | `0xF7F7FA` | 主文字 |
| `UI_THEME_DIM` | `0x8E8E9A` | 次要文字 / 提示 |
| `UI_THEME_SEPARATOR` | `0x2A2A34` | 分隔线 |
| `UI_THEME_SUCCESS` | `0x34D05A` | 成功 / 在线 |
| `UI_THEME_WARNING` | `0xFFB020` | 警告 |
| `UI_THEME_DANGER` | `0xFF4D42` | 危险 / 错误 |

**规则**

- 强调色只用一处（焦点环 / 关键数值 / 状态胶囊），不要整屏铺满。
- 层级通过“背景 → 卡片 → 聚焦卡片”三步亮度完成，不靠阴影。

## 3. 字体

只使用已内嵌的 3 套，**不要**再引入未生成的字体：

| 声明 | 字号 | 字符集 | 用途 |
|---|---|---|---|
| `ui_font_lvgl` | 14 px | 界面子集（见 `tools/ui_chars.txt`） | 标题 / 正文 / 应用名 |
| `ui_font_lvgl_10` | 10 px | 同上子集 | 小标签 / 提示 / 状态胶囊 |
| `book_font_lvgl` | 16 px | 全量 GB2312 | 品牌名 / 电子书正文 |

> **重要**：任何新增中文必须确认已存在于 `tools/ui_chars.txt` 并重新生成 14px/10px 字体，
> 否则会显示为缺字方框。品牌名等需要全量字符时改用 `book_font_lvgl`。

## 4. 尺寸与间距

| Token | 值 | 用途 |
|---|---|---|
| `UI_PAGE_MARGIN` | 10 | 页面左右边距 |
| `UI_TOPBAR_H` | 38 | 顶栏高度 |
| `UI_FOOTER_H` | 20 | 底栏高度 |
| `UI_CARD_RADIUS` | 10 | 卡片圆角 |
| 胶囊圆角 | 高度的一半 | 状态胶囊 / 页指示 |

## 5. 动效

| Token | 值 | 用途 |
|---|---|---|
| `UI_TRANSITION_MS` | 150 | 屏幕切换淡入 |
| `UI_SCROLL_MS` | 180 | 主页翻页 |

**规则**

- 用 `lv_screen_load_anim(..., LV_SCREEN_LOAD_ANIM_FADE_ON, ...)` 做淡入，禁用大幅位移 / 缩放。
- 翻页用 `lv_anim_path_ease_out`。
- **禁止**透明模糊、全屏渐变、大阴影、高帧率逐帧动画——都会显著抬高渲染成本。

## 6. 组件用法

- `ui_screen_new_ex()` — 标准应用根屏（顶栏 + 可选底部返回提示）。
- `ui_style_list_button()` — 列表 / 菜单按钮（聚焦 = 强调色 2px 描边 + 卡片提亮）。
- `ui_style_compact_button()` — 键盘 / 紧致按钮。
- `ui_style_card()` — 不可聚焦卡片面板。
- `ui_app_icon()` — 应用图标徽标（圆角方形 + 单字 / 字母，颜色自动压暗 30%）。
- `ui_screen_show()` — 淡入加载新屏并自动删除旧屏（**必须**用它，代替裸 `lv_screen_load`）。

## 7. 检查清单

提交 UI 改动前确认：

1. 新中文都在 `tools/ui_chars.txt` 内（或改用 `book_font_lvgl`）。
2. 使用 `tools/check_balance.py`（或等效）校验括号 / 花括号配对。
3. 没有引入阴影 / 透明模糊 / 大面积渐变 / 高频重绘。
4. 屏幕切换走 `ui_screen_show()`，没有泄漏旧 screen。
5. 上板验证：连续翻页 / 进出应用 10 次以上，free heap 不持续下降。