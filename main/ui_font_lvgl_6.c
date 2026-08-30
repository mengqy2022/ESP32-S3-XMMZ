/*******************************************************************************
 * Size: 6 px
 * Bpp: 1
 * Opts: --font C:\Windows\Fonts\simhei.ttf --size 6 --bpp 1 --format lvgl --no-compress --no-prefilter --force-fast-kern-format --range 0x20-0x7E --symbols 一万三上下不与个中串为主久义书事二于互五产亮仅从仓他代件任优会但位低体作使供依侧保信修候值假偏做停像充先光免入全公共关其具兼内册写冲准减出分切列创初删到制刷前功加务动助包化区升半单占卡印即压原参双反发取口句只可台右号各合同名后向否启吸呼哑唯嘉器回因固图在地场坏堆塞填声处备复外大失头如始子字存孟定实容宽寸对导小尝尺局层屏嵌工左己已布帧常幕平并广序库应底度建开异式引当录彩径待循心必忆志态性总恢息戏成或截户才打扫找把抖拉拟拦择持挂指按损换据掌探接控推描提插播操擎支收放效数整文斥断新方无日旧时明是显景暂更替有服期未本机来极析果柄查标栏校样核根格检植概模横次止正此步段每比永汉池沿泄注流测浏消深游源滑漏灯点烁热焊焦爆片版牙物特状独率环现玻珠理璃生用由电畅界留白的相知矩短码硬确示秒积称移程稳空窗立符第等签简算管箭籍系素索累红约线组细终经结绘络统续综绿缓编缩网置翻考而聚背能脚自致航色节芯若菜蓝虹行表衰见览角解触认记设访证诊译试询该详语误说读调败赖起跑路跳转轮软轻载辅辑输边过运返这进远连退送适选通逻道避部采释里重量针钟错键镜长闪闭问闲间阅队阵阶阻除需静非靠面音页顶项顺须预题颜首驱驻高黄默齐 -o ..\main\ui_font_lvgl_6.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef UI_FONT_LVGL_6
#define UI_FONT_LVGL_6 1
#endif

#if UI_FONT_LVGL_6

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xf0,

    /* U+0022 "\"" */
    0x0,

    /* U+0023 "#" */
    0x1f, 0x80,

    /* U+0024 "$" */
    0x4d, 0x36, 0x80,

    /* U+0025 "%" */
    0xc9, 0xb0,

    /* U+0026 "&" */
    0xcb, 0xf0,

    /* U+0027 "'" */
    0x80,

    /* U+0028 "(" */
    0x6a, 0x0,

    /* U+0029 ")" */
    0xa5, 0x0,

    /* U+002A "*" */
    0x49, 0x0,

    /* U+002B "+" */
    0x5d, 0x0,

    /* U+002C "," */
    0x80,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x29, 0x48,

    /* U+0030 "0" */
    0x56, 0xd4,

    /* U+0031 "1" */
    0x55,

    /* U+0032 "2" */
    0xc5, 0x60,

    /* U+0033 "3" */
    0xc8, 0xe0,

    /* U+0034 "4" */
    0x2f, 0x90,

    /* U+0035 "5" */
    0xd8, 0xe0,

    /* U+0036 "6" */
    0x4a, 0xf0,

    /* U+0037 "7" */
    0x69, 0x20,

    /* U+0038 "8" */
    0xea, 0xf0,

    /* U+0039 "9" */
    0x75, 0xa0,

    /* U+003A ":" */
    0xa0,

    /* U+003B ";" */
    0xa0,

    /* U+003C "<" */
    0x2a, 0x46,

    /* U+003D "=" */
    0xe3, 0x80,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0xc8, 0x20,

    /* U+0040 "@" */
    0xdf, 0x40,

    /* U+0041 "A" */
    0x5b, 0x50,

    /* U+0042 "B" */
    0xfa, 0xf0,

    /* U+0043 "C" */
    0x52, 0xa0,

    /* U+0044 "D" */
    0xd6, 0xe0,

    /* U+0045 "E" */
    0xda, 0x60,

    /* U+0046 "F" */
    0xda, 0x40,

    /* U+0047 "G" */
    0x56, 0xb0,

    /* U+0048 "H" */
    0xbe, 0xd0,

    /* U+0049 "I" */
    0xf0,

    /* U+004A "J" */
    0x26, 0xf0,

    /* U+004B "K" */
    0xdb, 0x50,

    /* U+004C "L" */
    0x92, 0x60,

    /* U+004D "M" */
    0xb6, 0xf0,

    /* U+004E "N" */
    0xbf, 0xd0,

    /* U+004F "O" */
    0xd6, 0xe0,

    /* U+0050 "P" */
    0xd7, 0x40,

    /* U+0051 "Q" */
    0xd6, 0xe0,

    /* U+0052 "R" */
    0xfb, 0x50,

    /* U+0053 "S" */
    0xd1, 0x60,

    /* U+0054 "T" */
    0x49, 0x20,

    /* U+0055 "U" */
    0xb6, 0xf0,

    /* U+0056 "V" */
    0xbb, 0x20,

    /* U+0057 "W" */
    0xf6, 0xe0,

    /* U+0058 "X" */
    0xcb, 0x50,

    /* U+0059 "Y" */
    0xb9, 0x20,

    /* U+005A "Z" */
    0x4a, 0x60,

    /* U+005B "[" */
    0xea, 0xb0,

    /* U+005C "\\" */
    0x11, 0x20,

    /* U+005D "]" */
    0xd5, 0x70,

    /* U+005E "^" */
    0xc0,

    /* U+005F "_" */
    0xe0,

    /* U+0060 "`" */
    0x80,

    /* U+0061 "a" */
    0xdf, 0x80,

    /* U+0062 "b" */
    0x9a, 0xe0,

    /* U+0063 "c" */
    0xd3, 0x0,

    /* U+0064 "d" */
    0x3e, 0xf0,

    /* U+0065 "e" */
    0xfc,

    /* U+0066 "f" */
    0x49, 0x20,

    /* U+0067 "g" */
    0xda, 0x60,

    /* U+0068 "h" */
    0x9e, 0xd0,

    /* U+0069 "i" */
    0xf0,

    /* U+006A "j" */
    0x55, 0xc0,

    /* U+006B "k" */
    0x9b, 0x60,

    /* U+006C "l" */
    0xf0,

    /* U+006D "m" */
    0xff, 0x80,

    /* U+006E "n" */
    0xf6, 0x80,

    /* U+006F "o" */
    0xd7, 0x0,

    /* U+0070 "p" */
    0xd7, 0x40,

    /* U+0071 "q" */
    0xf6, 0xb0,

    /* U+0072 "r" */
    0xe8,

    /* U+0073 "s" */
    0xfc,

    /* U+0074 "t" */
    0x49, 0x20,

    /* U+0075 "u" */
    0xb7, 0x80,

    /* U+0076 "v" */
    0xb9, 0x0,

    /* U+0077 "w" */
    0xf7, 0x0,

    /* U+0078 "x" */
    0xcb, 0x0,

    /* U+0079 "y" */
    0xb9, 0x60,

    /* U+007A "z" */
    0x68,

    /* U+007B "{" */
    0xfc,

    /* U+007C "|" */
    0xfe,

    /* U+007D "}" */
    0xfc,

    /* U+007E "~" */
    0x38,

    /* U+4E00 "一" */
    0xf8,

    /* U+4E07 "万" */
    0xfc, 0x83, 0x8a, 0x4a, 0x60,

    /* U+4E09 "三" */
    0xf8, 0x1c, 0xf, 0x80,

    /* U+4E0A "上" */
    0x20, 0x83, 0x88, 0x23, 0xf0,

    /* U+4E0B "下" */
    0xfc, 0x83, 0xa, 0x20,

    /* U+4E0D "不" */
    0xf9, 0x1c, 0x52, 0x0,

    /* U+4E0E "与" */
    0x43, 0xdf, 0xf0, 0x80,

    /* U+4E2A "个" */
    0x21, 0x4a, 0x88, 0x20, 0x80,

    /* U+4E2D "中" */
    0x27, 0xeb, 0xf2, 0x10,

    /* U+4E32 "串" */
    0x27, 0xff, 0xff, 0x80,

    /* U+4E3A "为" */
    0x21, 0x1e, 0x54, 0xc4,

    /* U+4E3B "主" */
    0x27, 0xc9, 0xf2, 0x7c,

    /* U+4E45 "久" */
    0x20, 0xc5, 0x4, 0x2b, 0x10,

    /* U+4E49 "义" */
    0x21, 0x25, 0xc, 0x31, 0x20,

    /* U+4E66 "书" */
    0x2b, 0xe2, 0xbe, 0x20,

    /* U+4E8B "事" */
    0xfd, 0xe7, 0x8a, 0x78, 0x80,

    /* U+4E8C "二" */
    0x70, 0x1, 0xf0,

    /* U+4E8E "于" */
    0xf9, 0x3e, 0x46, 0x0,

    /* U+4E92 "互" */
    0xfa, 0x1c, 0xe1, 0x7c,

    /* U+4E94 "五" */
    0x7c, 0x87, 0x8a, 0xfc,

    /* U+4EA7 "产" */
    0x78, 0x9f, 0x8, 0x40,

    /* U+4EAE "亮" */
    0xfd, 0xe0, 0x3f, 0x5c,

    /* U+4EC5 "仅" */
    0x1, 0x76, 0xb6, 0x49, 0x66, 0x40,

    /* U+4ECE "从" */
    0x51, 0x45, 0x2a, 0xa8,

    /* U+4ED3 "仓" */
    0x62, 0xbe, 0xa4, 0xbc,

    /* U+4ED6 "他" */
    0x51, 0xc7, 0x9e, 0x64, 0x60,

    /* U+4EE3 "代" */
    0x51, 0x7d, 0x14, 0x44,

    /* U+4EF6 "件" */
    0x28, 0xbb, 0xa3, 0xf4, 0x80,

    /* U+4EFB "任" */
    0x0, 0xb1, 0x27, 0xf4, 0x8b, 0x90, 0x0,

    /* U+4F18 "优" */
    0x11, 0x47, 0xf6, 0x59, 0xb0,

    /* U+4F1A "会" */
    0x22, 0xab, 0xf5, 0x78,

    /* U+4F46 "但" */
    0x40, 0xfb, 0x73, 0x25, 0x8f, 0xc0,

    /* U+4F4D "位" */
    0x41, 0x7c, 0x96, 0x7d, 0x0,

    /* U+4F4E "低" */
    0x5c, 0xd1, 0x73, 0x45, 0x8a, 0x80,

    /* U+4F53 "体" */
    0x51, 0xf5, 0x9e, 0x7c,

    /* U+4F5C "作" */
    0x61, 0xed, 0xd4, 0x5d, 0x40,

    /* U+4F7F "使" */
    0x51, 0xff, 0x9e, 0x4c,

    /* U+4F9B "供" */
    0x9, 0xfe, 0x9f, 0x64,

    /* U+4F9D "依" */
    0x41, 0xf5, 0x96, 0x54,

    /* U+4FA7 "侧" */
    0x45, 0x7d, 0xd7, 0x75, 0x90,

    /* U+4FDD "保" */
    0x39, 0x2f, 0x1f, 0x50,

    /* U+4FE1 "信" */
    0x1, 0xf5, 0xf7, 0x5d, 0x74, 0x0,

    /* U+4FEE "修" */
    0x4d, 0xe7, 0x9a, 0x48,

    /* U+5019 "候" */
    0x59, 0xff, 0x9f, 0x54,

    /* U+503C "值" */
    0x51, 0xfc, 0x9e, 0x78, 0xf0,

    /* U+5047 "假" */
    0x79, 0xee, 0x9e, 0x78,

    /* U+504F "偏" */
    0x10, 0xa7, 0xbf, 0x7d, 0xf6, 0x40,

    /* U+505A "做" */
    0x29, 0xef, 0x96, 0x78,

    /* U+505C "停" */
    0x5e, 0xbb, 0x73, 0xf4, 0x80,

    /* U+50CF "像" */
    0x59, 0xdd, 0xde, 0x44,

    /* U+5145 "充" */
    0xfa, 0xbe, 0xc9, 0x80,

    /* U+5148 "先" */
    0x62, 0xcf, 0xd4, 0x9c,

    /* U+5149 "光" */
    0x11, 0x6f, 0xcc, 0x37, 0x30,

    /* U+514D "免" */
    0x20, 0xe5, 0x1a, 0x79, 0x70,

    /* U+5165 "入" */
    0x0, 0x83, 0x14, 0x48,

    /* U+5168 "全" */
    0x31, 0x33, 0x9f, 0x7c,

    /* U+516C "公" */
    0x31, 0x4a, 0x88, 0x48, 0xc0,

    /* U+5171 "共" */
    0x21, 0xe3, 0x3f, 0x21, 0x30,

    /* U+5173 "关" */
    0x57, 0xc9, 0xf2, 0x4c,

    /* U+5176 "其" */
    0x57, 0xd5, 0xf5, 0x0,

    /* U+5177 "具" */
    0x79, 0xe4, 0xbf, 0x78,

    /* U+517C "兼" */
    0x3, 0xe3, 0x9e, 0x72, 0xf0,

    /* U+5185 "内" */
    0x27, 0xeb, 0xb8, 0x80,

    /* U+518C "册" */
    0x79, 0xef, 0xde, 0x78, 0x20,

    /* U+5199 "写" */
    0xfe, 0xc, 0xfe, 0x84,

    /* U+51B2 "冲" */
    0x2, 0xf3, 0x6f, 0x80,

    /* U+51C6 "准" */
    0xa8, 0xc7, 0xae, 0xbc, 0x80,

    /* U+51CF "减" */
    0x13, 0xc7, 0xba, 0x6a, 0x70,

    /* U+51FA "出" */
    0x25, 0x7f, 0x5f, 0x84,

    /* U+5206 "分" */
    0x51, 0x2f, 0xca, 0x50,

    /* U+5207 "切" */
    0x7a, 0xd6, 0xb2, 0x80,

    /* U+5217 "列" */
    0xec, 0xfe, 0xb4, 0x80,

    /* U+521B "创" */
    0x21, 0x7e, 0xd3, 0x55, 0xd0,

    /* U+521D "初" */
    0x7e, 0xd7, 0xb6, 0xb4,

    /* U+5220 "删" */
    0x7a, 0xfd, 0xfb, 0xf7, 0xa0,

    /* U+5230 "到" */
    0xeb, 0xf6, 0xfe, 0x80,

    /* U+5236 "制" */
    0xc7, 0xd5, 0x7d, 0xe4,

    /* U+5237 "刷" */
    0x75, 0x76, 0xd7, 0x94, 0x10,

    /* U+524D "前" */
    0x11, 0xaf, 0x1e, 0xaa, 0xa0,

    /* U+529F "功" */
    0xf1, 0xf5, 0x55, 0x94, 0x80,

    /* U+52A0 "加" */
    0x47, 0xde, 0xfb, 0x80,

    /* U+52A1 "务" */
    0x0, 0xe7, 0x33, 0x78, 0xa4, 0x80,

    /* U+52A8 "动" */
    0x1, 0x7f, 0x65, 0xb5, 0xb0,

    /* U+52A9 "助" */
    0xd7, 0xf7, 0xbd, 0x94,

    /* U+5305 "包" */
    0x41, 0xef, 0x1d, 0x7c,

    /* U+5316 "化" */
    0x51, 0x5d, 0x9c, 0x50, 0x70,

    /* U+533A "区" */
    0xfa, 0x2b, 0x2a, 0xfc,

    /* U+5347 "升" */
    0x38, 0xaf, 0xca, 0x48,

    /* U+534A "半" */
    0xa9, 0x1d, 0xf2, 0x0,

    /* U+5355 "单" */
    0x31, 0xa7, 0x9e, 0xfc, 0x0,

    /* U+5360 "占" */
    0x21, 0xc9, 0xbf, 0x80,

    /* U+5361 "卡" */
    0x20, 0xcf, 0xc8, 0x38, 0x80,

    /* U+5370 "印" */
    0xde, 0x5f, 0x65, 0xd0,

    /* U+5373 "即" */
    0xff, 0x7b, 0x5e, 0x80,

    /* U+538B "压" */
    0x7d, 0x47, 0xd4, 0x51, 0xf0,

    /* U+539F "原" */
    0x7d, 0xf7, 0xdf, 0x72, 0x50,

    /* U+53C2 "参" */
    0x23, 0xbe, 0xe2, 0xb0,

    /* U+53CC "双" */
    0xf8, 0xe5, 0x9a, 0x98, 0x80,

    /* U+53CD "反" */
    0x9, 0xc7, 0x9a, 0x52, 0xa0,

    /* U+53D1 "发" */
    0x1, 0xa3, 0xe, 0x72, 0x46, 0x80,

    /* U+53D6 "取" */
    0x79, 0x67, 0x9e, 0x68, 0x50,

    /* U+53E3 "口" */
    0xfc, 0x63, 0xf0,

    /* U+53E5 "句" */
    0x41, 0xff, 0x55, 0x78, 0x20,

    /* U+53EA "只" */
    0x79, 0x24, 0x9e, 0x11, 0x20,

    /* U+53EF "可" */
    0xfc, 0x26, 0x9a, 0x8, 0x40,

    /* U+53F0 "台" */
    0x44, 0x99, 0xef, 0x0,

    /* U+53F3 "右" */
    0x27, 0xd1, 0xe7, 0x20,

    /* U+53F7 "号" */
    0x74, 0xbe, 0xe1, 0x18,

    /* U+5404 "各" */
    0x21, 0xe2, 0x3e, 0x48, 0xc0,

    /* U+5408 "合" */
    0x21, 0x6b, 0x52, 0x79, 0x0,

    /* U+540C "同" */
    0xff, 0xef, 0xbb, 0xc4,

    /* U+540D "名" */
    0x43, 0xa9, 0xb7, 0xa4,

    /* U+540E "后" */
    0x5, 0xe7, 0xd9, 0xbc,

    /* U+5411 "向" */
    0x7, 0xef, 0x78, 0x80,

    /* U+5426 "否" */
    0xf9, 0x1f, 0xaf, 0x0,

    /* U+542F "启" */
    0x3, 0xde, 0xf7, 0xc4,

    /* U+5438 "吸" */
    0xfb, 0xce, 0xbe, 0xd1, 0xa0,

    /* U+547C "呼" */
    0xba, 0x69, 0x2e, 0x90, 0x80,

    /* U+54D1 "哑" */
    0xfb, 0x6f, 0xb6, 0xbc,

    /* U+552F "唯" */
    0xeb, 0xef, 0xfa, 0xfc, 0x80,

    /* U+5609 "嘉" */
    0xf0, 0x3d, 0x7, 0x0,

    /* U+5668 "器" */
    0xde, 0xff, 0xbd, 0x80,

    /* U+56DE "回" */
    0xfd, 0xf7, 0x5f, 0xc0,

    /* U+56E0 "因" */
    0xfd, 0x7f, 0xbf, 0x80,

    /* U+56FA "固" */
    0xfd, 0x6b, 0xff, 0xb8,

    /* U+56FE "图" */
    0xfd, 0x7b, 0xfa, 0xfc,

    /* U+5728 "在" */
    0x47, 0xe9, 0xea, 0x7c,

    /* U+5730 "地" */
    0x40, 0x93, 0xb3, 0xe6, 0xd4, 0x47, 0x0,

    /* U+573A "场" */
    0x59, 0x45, 0xd7, 0x74, 0x70,

    /* U+574F "坏" */
    0x5f, 0xd1, 0x33, 0xea, 0x81, 0x0,

    /* U+5806 "堆" */
    0x49, 0xe7, 0xd7, 0x1c, 0x0,

    /* U+585E "塞" */
    0x10, 0xf9, 0xf3, 0xe3, 0x8f, 0xc0,

    /* U+586B "填" */
    0x5d, 0xc9, 0x72, 0xe5, 0xe2, 0x80,

    /* U+58F0 "声" */
    0x7d, 0xe5, 0x5f, 0x40, 0x0,

    /* U+5904 "处" */
    0x48, 0xd1, 0xb1, 0x45, 0xc0,

    /* U+5907 "备" */
    0x20, 0x71, 0xe1, 0x4f, 0xcf, 0x9f, 0x0,

    /* U+590D "复" */
    0x3f, 0xe4, 0x8e, 0xf3, 0x30,

    /* U+5916 "外" */
    0x91, 0xcd, 0x95, 0x90,

    /* U+5927 "大" */
    0x10, 0x47, 0xc4, 0x29, 0x10,

    /* U+5931 "失" */
    0x63, 0xe9, 0xf2, 0x6c,

    /* U+5934 "头" */
    0x23, 0x19, 0xf2, 0x2c,

    /* U+5982 "如" */
    0x43, 0xfb, 0x7d, 0x6e, 0x0,

    /* U+59CB "始" */
    0x48, 0xeb, 0x76, 0xe7, 0xd2, 0x0,

    /* U+5B50 "子" */
    0x78, 0x4f, 0xc8, 0x20, 0x80,

    /* U+5B57 "字" */
    0x7, 0xe3, 0xf2, 0x10,

    /* U+5B58 "存" */
    0x47, 0xe4, 0xf1, 0x18,

    /* U+5B5F "孟" */
    0x38, 0x87, 0x9e, 0x7b, 0xf0,

    /* U+5B9A "定" */
    0x11, 0xf4, 0x47, 0x78,

    /* U+5B9E "实" */
    0x27, 0xe7, 0xf3, 0x60,

    /* U+5BB9 "容" */
    0x7, 0xf2, 0xe5, 0x30,

    /* U+5BBD "宽" */
    0x3, 0xf7, 0x96, 0xfc,

    /* U+5BF8 "寸" */
    0x17, 0xc5, 0x21, 0x10,

    /* U+5BF9 "对" */
    0x9, 0xfc, 0x9a, 0x88, 0x60,

    /* U+5BFC "导" */
    0x79, 0xe0, 0xbf, 0x48, 0x60,

    /* U+5C0F "小" */
    0x21, 0x2b, 0x42, 0x0,

    /* U+5C1D "尝" */
    0x37, 0xc1, 0xf5, 0x38,

    /* U+5C3A "尺" */
    0x79, 0x27, 0x98, 0x52, 0x20,

    /* U+5C40 "局" */
    0x7d, 0xf7, 0xdf, 0xb8,

    /* U+5C42 "层" */
    0x7d, 0xf5, 0xdf, 0x6a, 0xf0,

    /* U+5C4F "屏" */
    0x7d, 0xf7, 0xdf, 0xa8,

    /* U+5D4C "嵌" */
    0x45, 0xef, 0xda, 0x74,

    /* U+5DE5 "工" */
    0xf8, 0x82, 0x8, 0xfc,

    /* U+5DE6 "左" */
    0x47, 0xd1, 0xef, 0x80,

    /* U+5DF1 "己" */
    0xf8, 0x2f, 0xa0, 0xf8,

    /* U+5DF2 "已" */
    0xfa, 0x2f, 0xa0, 0xf8,

    /* U+5E03 "布" */
    0x43, 0x9f, 0xd6, 0x0,

    /* U+5E27 "帧" */
    0x4f, 0xff, 0x7d, 0xd8,

    /* U+5E38 "常" */
    0x6b, 0xf8, 0x9f, 0x24, 0x80,

    /* U+5E55 "幕" */
    0x7a, 0x5e, 0xaf, 0xa4,

    /* U+5E73 "平" */
    0xfd, 0x4d, 0xf2, 0x10,

    /* U+5E76 "并" */
    0x29, 0xe3, 0x3f, 0x20,

    /* U+5E7F "广" */
    0x11, 0xf4, 0x10, 0x42, 0x0,

    /* U+5E8F "序" */
    0x1, 0xf7, 0xd6, 0x7c, 0x20,

    /* U+5E93 "库" */
    0x0, 0xf9, 0xfb, 0xe7, 0xf1, 0x0,

    /* U+5E94 "应" */
    0x11, 0xf4, 0x1d, 0x4a, 0xf0,

    /* U+5E95 "底" */
    0x11, 0xf4, 0x9e, 0x59, 0xab, 0x40,

    /* U+5EA6 "度" */
    0x1, 0xf4, 0x96, 0x79, 0xca, 0xc0,

    /* U+5EFA "建" */
    0xba, 0xff, 0xae, 0xbd, 0x40,

    /* U+5F00 "开" */
    0x78, 0xcf, 0xcc, 0x40,

    /* U+5F02 "异" */
    0xff, 0xd5, 0xf9, 0x0,

    /* U+5F0F "式" */
    0x11, 0xe7, 0xc, 0x39, 0x10,

    /* U+5F15 "引" */
    0xe9, 0x73, 0xd2, 0xb0,

    /* U+5F53 "当" */
    0x2d, 0x3f, 0xff, 0x84,

    /* U+5F55 "录" */
    0x7d, 0xe7, 0xd5, 0x39, 0xd0,

    /* U+5F69 "彩" */
    0x10, 0x78, 0x93, 0xc7, 0x15, 0x80,

    /* U+5F84 "径" */
    0xfb, 0x46, 0xfe, 0x51, 0xf0,

    /* U+5F85 "待" */
    0x50, 0xe7, 0xff, 0x69, 0x60,

    /* U+5FAA "循" */
    0x7e, 0xf7, 0xdd, 0x5c,

    /* U+5FC3 "心" */
    0x10, 0xa, 0x68, 0x20, 0xe0,

    /* U+5FC5 "必" */
    0x0, 0x6a, 0xec, 0x60, 0xe0,

    /* U+5FC6 "忆" */
    0x5f, 0xad, 0x19, 0x5c,

    /* U+5FD7 "志" */
    0x11, 0xe7, 0xab, 0x38,

    /* U+6001 "态" */
    0x27, 0xeb, 0x97, 0x0,

    /* U+6027 "性" */
    0x5b, 0xfc, 0x97, 0x49, 0x70,

    /* U+603B "总" */
    0x31, 0x27, 0xa1, 0x30,

    /* U+6062 "恢" */
    0x51, 0xfe, 0x9f, 0x54,

    /* U+606F "息" */
    0x79, 0xe7, 0xbf, 0x30,

    /* U+620F "戏" */
    0x13, 0x65, 0x57, 0x4a, 0x70,

    /* U+6210 "成" */
    0x11, 0xe7, 0x9a, 0x6c, 0x50,

    /* U+6216 "或" */
    0x13, 0xf5, 0x5e, 0x29, 0xd0,

    /* U+622A "截" */
    0xf0, 0x67, 0x32, 0xf4,

    /* U+6237 "户" */
    0x11, 0xf4, 0x5f, 0x42, 0x0,

    /* U+624D "才" */
    0x13, 0xf1, 0x1c, 0x90, 0x80,

    /* U+6253 "打" */
    0x5f, 0xa4, 0xb2, 0x4b, 0x60,

    /* U+626B "扫" */
    0x5f, 0x95, 0xf1, 0x5c,

    /* U+627E "找" */
    0x53, 0x57, 0xb6, 0x5f, 0x80,

    /* U+628A "把" */
    0x5d, 0xd9, 0xb2, 0xed, 0xc0,

    /* U+6296 "抖" */
    0x4b, 0xa4, 0xb2, 0x7b, 0x20,

    /* U+62C9 "拉" */
    0x43, 0xf4, 0x76, 0x49, 0xf0,

    /* U+62DF "拟" */
    0x45, 0xe9, 0x97, 0x25, 0xda, 0x40,

    /* U+62E6 "拦" */
    0x5b, 0xf4, 0x37, 0x43, 0xf0,

    /* U+62E9 "择" */
    0x5d, 0xe9, 0x7a, 0x47, 0xf9, 0x0,

    /* U+6301 "持" */
    0x53, 0xc7, 0xff, 0x6b, 0x60,

    /* U+6302 "挂" */
    0x49, 0xd1, 0x32, 0xe4, 0x9f, 0xc0,

    /* U+6307 "指" */
    0x47, 0xe5, 0xd9, 0x5c, 0x0,

    /* U+6309 "按" */
    0x43, 0xb7, 0xf6, 0x48, 0x50,

    /* U+635F "损" */
    0x5d, 0xf9, 0x77, 0xa5, 0x5a, 0x80,

    /* U+6362 "换" */
    0x53, 0xa7, 0xff, 0x5b, 0x90,

    /* U+636E "据" */
    0x5f, 0xf7, 0xd7, 0x56, 0xa0,

    /* U+638C "掌" */
    0x73, 0xf7, 0x18, 0xfc, 0x80,

    /* U+63A2 "探" */
    0x5f, 0xed, 0x22, 0xf5, 0xcd, 0x40,

    /* U+63A5 "接" */
    0x5f, 0xe5, 0xf7, 0x5b, 0x50,

    /* U+63A7 "控" */
    0x5f, 0xed, 0x6, 0xe4, 0x8f, 0xc0,

    /* U+63A8 "推" */
    0x5b, 0xa5, 0xf7, 0x5f, 0x0,

    /* U+63CF "描" */
    0x53, 0xe6, 0xdf, 0x6f, 0xf0,

    /* U+63D0 "提" */
    0x5d, 0xc9, 0xfb, 0x6f, 0xe0,

    /* U+63D2 "插" */
    0x5c, 0xfd, 0xb3, 0x65, 0xc8, 0x80,

    /* U+64AD "播" */
    0x5d, 0xf9, 0x77, 0xf5, 0xcb, 0x80,

    /* U+64CD "操" */
    0x5b, 0x85, 0xd7, 0x5b, 0x90,

    /* U+64CE "擎" */
    0xff, 0xe7, 0x44, 0xfc, 0x40,

    /* U+652F "支" */
    0x23, 0xbc, 0xa2, 0x6c,

    /* U+6536 "收" */
    0x26, 0xed, 0xa2, 0x80,

    /* U+653E "放" */
    0x1, 0x77, 0x5a, 0x6a, 0x50,

    /* U+6548 "效" */
    0x53, 0xfa, 0x86, 0x52, 0xa0,

    /* U+6570 "数" */
    0xf3, 0xe6, 0xbe, 0xb3, 0xa0,

    /* U+6574 "整" */
    0x43, 0xee, 0x9c, 0xfc,

    /* U+6587 "文" */
    0x27, 0xd4, 0xc2, 0x6c,

    /* U+65A5 "斥" */
    0x9, 0xc7, 0xdc, 0x98,

    /* U+65AD "断" */
    0x4b, 0x4f, 0xf6, 0xf0,

    /* U+65B0 "新" */
    0xf9, 0x45, 0xbe, 0xf9, 0xa0,

    /* U+65B9 "方" */
    0x10, 0x7, 0x8f, 0x29, 0x20, 0x0,

    /* U+65E0 "无" */
    0x78, 0x87, 0xcc, 0x55, 0x60,

    /* U+65E5 "日" */
    0xf9, 0xf9, 0xf0,

    /* U+65E7 "旧" */
    0xfe, 0x7f, 0x9f, 0x80,

    /* U+65F6 "时" */
    0xca, 0xfe, 0xb2, 0x88, 0x40,

    /* U+660E "明" */
    0xdf, 0x5d, 0xf7, 0xa4, 0x80,

    /* U+662F "是" */
    0x79, 0xe7, 0xbf, 0x7c,

    /* U+663E "显" */
    0x79, 0xe7, 0x9e, 0xfc,

    /* U+666F "景" */
    0x73, 0x80, 0xe7, 0x54,

    /* U+6682 "暂" */
    0x5b, 0xe0, 0x9e, 0x79, 0xc0,

    /* U+66F4 "更" */
    0x79, 0xe7, 0x9e, 0x61, 0x70,

    /* U+66FF "替" */
    0xf7, 0xfc, 0xe7, 0x30,

    /* U+6709 "有" */
    0x3, 0x9d, 0xe7, 0x0,

    /* U+670D "服" */
    0x7c, 0xe9, 0xf3, 0xe7, 0xc0,

    /* U+671F "期" */
    0x4d, 0xb6, 0xfb, 0x54, 0x0,

    /* U+672A "未" */
    0x21, 0xcf, 0x9c, 0x28,

    /* U+672C "本" */
    0x11, 0xf3, 0x96, 0x7c, 0x40,

    /* U+673A "机" */
    0x5b, 0xa6, 0xba, 0x4c,

    /* U+6765 "来" */
    0x21, 0xcf, 0x9c, 0xa8,

    /* U+6781 "极" */
    0x7b, 0xa6, 0xbe, 0x68,

    /* U+6790 "析" */
    0x5f, 0xc5, 0xfe, 0x60,

    /* U+679C "果" */
    0x79, 0xe7, 0xbf, 0x48,

    /* U+67C4 "柄" */
    0x7e, 0xb9, 0xb6, 0xa4, 0x40,

    /* U+67E5 "查" */
    0x7, 0xbf, 0xef, 0x80,

    /* U+6807 "标" */
    0x5d, 0xc1, 0x77, 0xe6, 0x80,

    /* U+680F "栏" */
    0x5b, 0xf4, 0x32, 0x5d, 0x0,

    /* U+6821 "校" */
    0x4b, 0x85, 0x76, 0x58,

    /* U+6837 "样" */
    0x59, 0x25, 0xfa, 0xdd, 0x20,

    /* U+6838 "核" */
    0x49, 0x7, 0xb1, 0x5c,

    /* U+6839 "根" */
    0x5b, 0xe7, 0xba, 0x50,

    /* U+683C "格" */
    0x63, 0xe5, 0x3a, 0x58,

    /* U+68C0 "检" */
    0x53, 0xe5, 0xf9, 0x59, 0xf0,

    /* U+690D "植" */
    0x53, 0xe5, 0xb6, 0x58, 0xf0,

    /* U+6982 "概" */
    0x59, 0xb1, 0x7e, 0xc4, 0x60,

    /* U+6A21 "模" */
    0x5f, 0xe5, 0xbe, 0x54,

    /* U+6A2A "横" */
    0x59, 0xf7, 0xbe, 0x79, 0x40,

    /* U+6B21 "次" */
    0x22, 0xe6, 0x14, 0xb0, 0xb0,

    /* U+6B62 "止" */
    0x11, 0x45, 0x94, 0xfc,

    /* U+6B63 "正" */
    0x31, 0x65, 0x14, 0xfc,

    /* U+6B64 "此" */
    0x11, 0x4d, 0x6e, 0xd2, 0x70,

    /* U+6B65 "步" */
    0x23, 0xbe, 0x4b, 0xf0,

    /* U+6BB5 "段" */
    0x3c, 0xa9, 0xb, 0xe5, 0x4b, 0x80,

    /* U+6BCF "每" */
    0x1, 0xfb, 0x9a, 0x59, 0xf1, 0x80,

    /* U+6BD4 "比" */
    0xa5, 0x7d, 0x4a, 0x7c,

    /* U+6C38 "永" */
    0x0, 0x43, 0x5e, 0x59, 0x53, 0x0,

    /* U+6C49 "汉" */
    0x78, 0xa6, 0x94, 0xa8, 0x0,

    /* U+6C60 "池" */
    0x21, 0x78, 0xd5, 0xb, 0xc0,

    /* U+6CBF "沿" */
    0x98, 0xaa, 0xee, 0xb8, 0x80,

    /* U+6CC4 "泄" */
    0xb8, 0xef, 0xce, 0xa2, 0xf0,

    /* U+6CE8 "注" */
    0x90, 0xe9, 0xe, 0xfc,

    /* U+6D41 "流" */
    0x7c, 0xa7, 0xdc, 0x34,

    /* U+6D4B "测" */
    0x4, 0xf7, 0xdf, 0xac,

    /* U+6D4F "浏" */
    0x24, 0xf1, 0x4d, 0xe4,

    /* U+6D88 "消" */
    0xb0, 0x2a, 0xf1, 0xeb, 0xd4, 0x80,

    /* U+6DF1 "深" */
    0x3c, 0xf2, 0xf, 0x6a, 0x10,

    /* U+6E38 "游" */
    0x8, 0x79, 0xdd, 0x7b, 0x5b, 0x80,

    /* U+6E90 "源" */
    0xbc, 0xf3, 0xcf, 0xbb, 0xb0,

    /* U+6ED1 "滑" */
    0xb9, 0x67, 0xee, 0xb8, 0x20,

    /* U+6F0F "漏" */
    0x3c, 0x83, 0xdd, 0xc4,

    /* U+706F "灯" */
    0x7f, 0xa4, 0x92, 0xa8, 0x40,

    /* U+70B9 "点" */
    0x0, 0x64, 0x9e, 0x62, 0xd0,

    /* U+70C1 "烁" */
    0xbb, 0x8b, 0xd4, 0xb6, 0x40,

    /* U+70ED "热" */
    0x53, 0x65, 0x95, 0x22, 0x50,

    /* U+710A "焊" */
    0x7b, 0xef, 0x9e, 0xbe, 0x40,

    /* U+7126 "焦" */
    0x51, 0xfd, 0x1f, 0x26, 0x80,

    /* U+7206 "爆" */
    0x7b, 0xef, 0xdf, 0x66, 0xc0,

    /* U+7247 "片" */
    0x52, 0x98, 0xe5, 0x48,

    /* U+7248 "版" */
    0xbd, 0x3f, 0xfb, 0x0,

    /* U+7259 "牙" */
    0xf8, 0x4f, 0xcc, 0x52, 0xc0,

    /* U+7269 "物" */
    0x53, 0xb5, 0xdb, 0x55, 0x30,

    /* U+7279 "特" */
    0x7b, 0xc4, 0x97, 0xe9, 0x20,

    /* U+72B6 "状" */
    0x51, 0x47, 0xd4, 0xe9, 0x90,

    /* U+72EC "独" */
    0x52, 0xe7, 0xbe, 0x52, 0xf0,

    /* U+7387 "率" */
    0x11, 0xf6, 0x44, 0x1b, 0xf1, 0x0,

    /* U+73AF "环" */
    0xfe, 0x93, 0xb2, 0x6d, 0x81, 0x0,

    /* U+73B0 "现" */
    0x1c, 0xdb, 0xb2, 0x49, 0xe0,

    /* U+73BB "玻" */
    0x8, 0xf3, 0xf3, 0xa6, 0x86, 0x80,

    /* U+73E0 "珠" */
    0xf8, 0x91, 0x7a, 0xc8, 0xe0,

    /* U+7406 "理" */
    0x1c, 0xdb, 0xf2, 0xcf, 0xe0,

    /* U+7483 "璃" */
    0x3, 0xf4, 0xd7, 0x5d, 0xb0, 0x40,

    /* U+751F "生" */
    0x51, 0xf1, 0x1f, 0x11, 0xf0,

    /* U+7528 "用" */
    0x7d, 0x57, 0xdf, 0x56, 0x70,

    /* U+7531 "由" */
    0x27, 0xeb, 0xff, 0x80,

    /* U+7535 "电" */
    0x23, 0xef, 0xbe, 0xa0, 0xf0,

    /* U+7545 "畅" */
    0x5f, 0xaf, 0xff, 0xf5, 0x70,

    /* U+754C "界" */
    0x7c, 0xf9, 0xf1, 0x86, 0xc9, 0x0,

    /* U+7559 "留" */
    0xfe, 0xc0, 0xf7, 0xc4,

    /* U+767D "白" */
    0x47, 0xe3, 0xff, 0x80,

    /* U+7684 "的" */
    0x57, 0xeb, 0xde, 0xd4,

    /* U+76F8 "相" */
    0x5d, 0x95, 0xf7, 0x5c,

    /* U+77E5 "知" */
    0x87, 0xd6, 0xb9, 0x80,

    /* U+77E9 "矩" */
    0x9f, 0xc5, 0xd7, 0xb2, 0x70,

    /* U+77ED "短" */
    0x9d, 0x75, 0x57, 0x9c,

    /* U+7801 "码" */
    0xf5, 0xbd, 0x9c, 0x88,

    /* U+786C "硬" */
    0xfd, 0xff, 0x7f, 0x0,

    /* U+786E "确" */
    0xf4, 0xbf, 0xff, 0x84,

    /* U+793A "示" */
    0x78, 0xf, 0xc8, 0x68, 0x80,

    /* U+79D2 "秒" */
    0x51, 0x6f, 0x32, 0x51, 0x80,

    /* U+79EF "积" */
    0xd9, 0xa6, 0x9e, 0xe1, 0x20,

    /* U+79F0 "称" */
    0x30, 0xbb, 0xb2, 0x4c, 0xc9, 0x0,

    /* U+79FB "移" */
    0xc9, 0x65, 0xbb, 0x58,

    /* U+7A0B "程" */
    0xfd, 0x95, 0xf7, 0x5d, 0x0,

    /* U+7A33 "稳" */
    0x11, 0xa5, 0xd7, 0x7d, 0x64, 0x0,

    /* U+7A7A "空" */
    0x23, 0xed, 0xbe, 0xfc,

    /* U+7A97 "窗" */
    0x27, 0xf6, 0xfb, 0xd5, 0xe0,

    /* U+7ACB "立" */
    0x27, 0xc0, 0xa2, 0x7c,

    /* U+7B26 "符" */
    0x13, 0xa4, 0x9f, 0xc9, 0x25, 0x80,

    /* U+7B2C "第" */
    0x9, 0xf0, 0x1f, 0x7c, 0xd5, 0x0,

    /* U+7B49 "等" */
    0x40, 0xfd, 0x57, 0xf3, 0xc0, 0x0,

    /* U+7B7E "签" */
    0x17, 0xd8, 0xbf, 0xfc,

    /* U+7B80 "简" */
    0x13, 0xa7, 0xa6, 0xfa, 0x20,

    /* U+7B97 "算" */
    0x1, 0xb0, 0x1f, 0x47, 0xf4, 0x80,

    /* U+7BA1 "管" */
    0x41, 0xf8, 0x3f, 0x79, 0xe7, 0x80,

    /* U+7BAD "箭" */
    0x3, 0xe1, 0xfa, 0x7b, 0x40,

    /* U+7C4D "籍" */
    0x1, 0xbd, 0x37, 0xfb, 0xe8, 0x80,

    /* U+7CFB "系" */
    0x78, 0xa3, 0x1f, 0x13, 0x50,

    /* U+7D20 "素" */
    0xff, 0xd1, 0xf2, 0x4,

    /* U+7D22 "索" */
    0xfb, 0xfc, 0x7e, 0x28, 0x80,

    /* U+7D2F "累" */
    0xff, 0xd5, 0xf7, 0x4,

    /* U+7EA2 "红" */
    0x41, 0x7a, 0x92, 0x9, 0x70,

    /* U+7EA6 "约" */
    0x52, 0xfc, 0x55, 0x5, 0x10,

    /* U+7EBF "线" */
    0x52, 0x5f, 0xae, 0x8a, 0xf0,

    /* U+7EC4 "组" */
    0x5d, 0x29, 0x73, 0xe3, 0xe0,

    /* U+7EC6 "细" */
    0xbd, 0xef, 0x7f, 0x80,

    /* U+7EC8 "终" */
    0x52, 0x6b, 0x96, 0xf9, 0x60,

    /* U+7ECF "经" */
    0x5e, 0xa5, 0x5f, 0x3c,

    /* U+7ED3 "结" */
    0x4a, 0x2d, 0xf6, 0xd8,

    /* U+7ED8 "绘" */
    0x97, 0x61, 0x72, 0x1c,

    /* U+7EDC "络" */
    0x51, 0x3b, 0x22, 0xe5, 0xc2, 0x80,

    /* U+7EDF "统" */
    0x15, 0xf9, 0x73, 0x14,

    /* U+7EED "续" */
    0x7f, 0xe4, 0x7e, 0x80,

    /* U+7EFC "综" */
    0x2, 0xfe, 0x6f, 0x12, 0x90,

    /* U+7EFF "绿" */
    0x9a, 0x6f, 0xe4, 0x18, 0x40,

    /* U+7F13 "缓" */
    0x97, 0xef, 0xb7, 0x14,

    /* U+7F16 "编" */
    0x12, 0xff, 0xef, 0xbf, 0xf2, 0x40,

    /* U+7F29 "缩" */
    0x2, 0xff, 0x2b, 0xf4, 0xfa, 0xc0,

    /* U+7F51 "网" */
    0xfc, 0xf7, 0xdf, 0xcc,

    /* U+7F6E "置" */
    0xfb, 0xe7, 0x1c, 0x93, 0xf0,

    /* U+7FFB "翻" */
    0x7d, 0xb2, 0xdd, 0x74, 0xa0,

    /* U+8003 "考" */
    0x78, 0xc7, 0xae, 0x8,

    /* U+800C "而" */
    0x77, 0xfb, 0xd8, 0x80,

    /* U+805A "聚" */
    0x7d, 0xd5, 0xdd, 0xdc,

    /* U+80CC "背" */
    0x5b, 0x77, 0x9e, 0x79, 0x20,

    /* U+80FD "能" */
    0x83, 0xfe, 0xaa, 0xea, 0x80,

    /* U+811A "脚" */
    0xaf, 0xfe, 0xf3, 0xf8,

    /* U+81EA "自" */
    0x45, 0xff, 0xff, 0x80,

    /* U+81F4 "致" */
    0xe3, 0xfa, 0xba, 0x90, 0xb0,

    /* U+822A "航" */
    0x3, 0xff, 0xba, 0xeb, 0xbe, 0xc0,

    /* U+8272 "色" */
    0x20, 0xcf, 0xdf, 0x7c,

    /* U+8282 "节" */
    0x21, 0xe7, 0xc9, 0x24, 0x0,

    /* U+82AF "芯" */
    0x47, 0xd1, 0x85, 0x38,

    /* U+82E5 "若" */
    0x29, 0xe7, 0xde, 0x38,

    /* U+83DC "菜" */
    0xfd, 0xe5, 0xbf, 0x3b, 0x50,

    /* U+84DD "蓝" */
    0xfc, 0xc6, 0x9e, 0xfc,

    /* U+8679 "虹" */
    0x43, 0xae, 0xba, 0x6a, 0xf0,

    /* U+884C "行" */
    0x5e, 0x7, 0xf2, 0x49, 0x60,

    /* U+8868 "表" */
    0x11, 0xff, 0xc9, 0xe8, 0x90,

    /* U+8870 "衰" */
    0xfb, 0xbe, 0xe7, 0x6c,

    /* U+89C1 "见" */
    0x79, 0xa6, 0x9c, 0x33, 0x70,

    /* U+89C8 "览" */
    0xbe, 0x47, 0x1c, 0xbc,

    /* U+89D2 "角" */
    0x20, 0xef, 0xdf, 0x7e, 0x50,

    /* U+89E3 "解" */
    0x5d, 0xd7, 0x9e, 0x7c, 0x20,

    /* U+89E6 "触" */
    0x96, 0xff, 0xff, 0x80,

    /* U+8BA4 "认" */
    0x50, 0x4d, 0x14, 0x69, 0xa0,

    /* U+8BB0 "记" */
    0x5c, 0x9, 0x73, 0x5, 0xc0,

    /* U+8BBE "设" */
    0x58, 0x27, 0x9a, 0x50, 0xb0,

    /* U+8BBF "访" */
    0x48, 0x45, 0xd5, 0x2c,

    /* U+8BC1 "证" */
    0x0, 0xb1, 0x33, 0x46, 0x8f, 0xc0,

    /* U+8BCA "诊" */
    0x9, 0x6e, 0xd4, 0x7d, 0x20,

    /* U+8BD1 "译" */
    0x5d, 0xb1, 0x9a, 0x45, 0xe1, 0x0,

    /* U+8BD5 "试" */
    0x49, 0xb1, 0xe3, 0x45, 0x40, 0x80,

    /* U+8BE2 "询" */
    0x50, 0xbf, 0xd7, 0x5d, 0x10,

    /* U+8BE5 "该" */
    0x1, 0x7d, 0x16, 0x55, 0xa0, 0x40,

    /* U+8BE6 "详" */
    0x0, 0x93, 0x32, 0xe4, 0x8b, 0xd2, 0x0,

    /* U+8BED "语" */
    0x5c, 0x39, 0xd2, 0xe6, 0x4b, 0x80,

    /* U+8BEF "误" */
    0x38, 0xe7, 0xdf, 0x70, 0x30,

    /* U+8BF4 "说" */
    0x48, 0x76, 0x57, 0xc,

    /* U+8BFB "读" */
    0x5c, 0x7d, 0xa3, 0xf5, 0xc0,

    /* U+8C03 "调" */
    0x5c, 0x75, 0xd7, 0x5c, 0x90,

    /* U+8D25 "败" */
    0xf3, 0xfe, 0xbe, 0xb0, 0x20,

    /* U+8D56 "赖" */
    0x53, 0xff, 0xfe, 0xd4,

    /* U+8D77 "起" */
    0x5d, 0x1f, 0xfc, 0xfc,

    /* U+8DD1 "跑" */
    0xe3, 0xff, 0xfe, 0x3c,

    /* U+8DEF "路" */
    0xe3, 0xef, 0x3e, 0xb8,

    /* U+8DF3 "跳" */
    0xda, 0xf5, 0xb6, 0xac,

    /* U+8F6C "转" */
    0x17, 0xf8, 0xb7, 0x0,

    /* U+8F6E "轮" */
    0x17, 0xab, 0xdf, 0x5c, 0x0,

    /* U+8F6F "软" */
    0x7, 0xe7, 0xa9, 0x59, 0x20,

    /* U+8F7B "轻" */
    0x6, 0xba, 0xfd, 0x3c,

    /* U+8F7D "载" */
    0x7b, 0xff, 0x3a, 0x31, 0x60,

    /* U+8F85 "辅" */
    0x1c, 0xaf, 0x7f, 0x84,

    /* U+8F91 "辑" */
    0x3a, 0xee, 0x9e, 0xac, 0x60,

    /* U+8F93 "输" */
    0x12, 0xf7, 0x7d, 0x44,

    /* U+8FB9 "边" */
    0x10, 0x7d, 0x5a, 0xbc,

    /* U+8FC7 "过" */
    0x88, 0x6e, 0x92, 0xbc,

    /* U+8FD0 "运" */
    0x58, 0xd, 0x9a, 0xbc,

    /* U+8FD4 "返" */
    0x5c, 0x7e, 0x9b, 0xfc,

    /* U+8FD9 "这" */
    0x1, 0x6d, 0x96, 0xbc,

    /* U+8FDB "进" */
    0x48, 0xe7, 0xda, 0xbc,

    /* U+8FDC "远" */
    0xbc, 0xf, 0xda, 0xbc,

    /* U+8FDE "连" */
    0x0, 0xfb, 0x73, 0xfb, 0xe0,

    /* U+9000 "退" */
    0x5c, 0xbb, 0x73, 0x2b, 0xe0,

    /* U+9001 "送" */
    0x68, 0x47, 0xda, 0xfc,

    /* U+9002 "适" */
    0x39, 0xfd, 0x1a, 0x7c,

    /* U+9009 "选" */
    0x20, 0x4f, 0x96, 0xbc,

    /* U+901A "通" */
    0x5c, 0x39, 0x72, 0xeb, 0xe0,

    /* U+903B "逻" */
    0x7c, 0xf5, 0xd0, 0xbc,

    /* U+9053 "道" */
    0x0, 0xfc, 0x72, 0xe5, 0xd7, 0x80,

    /* U+907F "避" */
    0xb1, 0xfe, 0xdf, 0xfc,

    /* U+90E8 "部" */
    0x1, 0x6d, 0xbe, 0xb7, 0xcb, 0x0,

    /* U+91C7 "采" */
    0xc, 0xe8, 0x43, 0xf3, 0x8a, 0x80,

    /* U+91CA "释" */
    0xdf, 0xef, 0x5f, 0xdc, 0x20,

    /* U+91CC "里" */
    0x79, 0xe7, 0x88, 0x33, 0xf0,

    /* U+91CD "重" */
    0x1b, 0xf7, 0x9a, 0xfc,

    /* U+91CF "量" */
    0x79, 0xe7, 0x9a, 0xfc,

    /* U+9488 "针" */
    0x4b, 0xaf, 0xfa, 0x68,

    /* U+949F "钟" */
    0x4a, 0x76, 0xd7, 0x69, 0x20,

    /* U+9519 "错" */
    0x51, 0xf7, 0xfe, 0x78, 0x60,

    /* U+952E "键" */
    0x7e, 0xf7, 0xff, 0x7c,

    /* U+955C "镜" */
    0x43, 0xe5, 0xd7, 0x5c,

    /* U+957F "长" */
    0x51, 0x8f, 0xd4, 0x71, 0x20,

    /* U+95EA "闪" */
    0x3e, 0x73, 0xd8, 0x80,

    /* U+95ED "闭" */
    0x3c, 0xff, 0x7b, 0x80,

    /* U+95EE "问" */
    0x5a, 0x7f, 0x78, 0xc4,

    /* U+95F2 "闲" */
    0x3d, 0x6b, 0xfa, 0x80,

    /* U+95F4 "间" */
    0x7d, 0xf7, 0xfb, 0x0,

    /* U+9605 "阅" */
    0x7d, 0xf3, 0x7d, 0x80,

    /* U+961F "队" */
    0xd3, 0x4d, 0x36, 0xeb, 0x10,

    /* U+9635 "阵" */
    0xc3, 0xef, 0x34, 0xfc, 0x40,

    /* U+9636 "阶" */
    0xd3, 0xae, 0xea, 0xa8,

    /* U+963B "阻" */
    0xfa, 0xaf, 0xae, 0xa9, 0xf0,

    /* U+9664 "除" */
    0xd2, 0xad, 0x2e, 0xfa, 0x40,

    /* U+9700 "需" */
    0x7f, 0xd4, 0xf3, 0x80,

    /* U+9759 "静" */
    0x67, 0xbb, 0x5e, 0x0,

    /* U+975E "非" */
    0x21, 0xe6, 0x8c, 0xec, 0x0,

    /* U+9760 "靠" */
    0x43, 0xf7, 0x9f, 0xfc, 0xc0,

    /* U+9762 "面" */
    0xff, 0xeb, 0xdf, 0x80,

    /* U+97F3 "音" */
    0x27, 0xd5, 0xf7, 0x39, 0xc0,

    /* U+9875 "页" */
    0xfb, 0xa4, 0xa5, 0x40,

    /* U+9876 "顶" */
    0xfd, 0x76, 0xd5, 0x50, 0x10,

    /* U+9879 "项" */
    0x1d, 0x76, 0x5d, 0x8, 0x40,

    /* U+987A "顺" */
    0x1f, 0xbb, 0xdf, 0x0,

    /* U+987B "须" */
    0xbd, 0xdf, 0xfa, 0x4,

    /* U+9884 "预" */
    0xfd, 0x3f, 0x70, 0x0,

    /* U+9898 "题" */
    0xfd, 0xfe, 0xff, 0x80,

    /* U+989C "颜" */
    0x3e, 0xbb, 0xf5, 0x14,

    /* U+9996 "首" */
    0x57, 0xdc, 0xe7, 0x38,

    /* U+9A71 "驱" */
    0xff, 0x3b, 0xdb, 0x3c,

    /* U+9A7B "驻" */
    0xd3, 0xf5, 0xf7, 0x80,

    /* U+9AD8 "高" */
    0xfb, 0x9d, 0x1f, 0x80,

    /* U+9EC4 "黄" */
    0x7d, 0xfd, 0xf3, 0xe7, 0xcd, 0x80,

    /* U+9ED8 "默" */
    0xe3, 0xc6, 0x9a, 0xb8, 0x0,

    /* U+9F50 "齐" */
    0x27, 0xd4, 0x4d, 0xa9, 0x40
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 3, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 5, .adv_w = 48, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 8, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 10, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 13, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 19, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 21, .adv_w = 48, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 22, .adv_w = 48, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 23, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 24, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 28, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 48, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 47, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 51, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 53, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 48, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 110, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 112, .adv_w = 48, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 114, .adv_w = 48, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 115, .adv_w = 48, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 116, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 117, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 130, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 135, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 146, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 148, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 162, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 48, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 164, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 165, .adv_w = 48, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 166, .adv_w = 48, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 167, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 168, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 173, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 182, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 199, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 203, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 211, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 215, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 220, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 225, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 234, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 245, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 253, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 263, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 271, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 276, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 280, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 96, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 292, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 301, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 307, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 312, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 318, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 327, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 344, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 354, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 367, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 377, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 403, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 421, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 426, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 430, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 434, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 443, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 452, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 456, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 465, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 470, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 474, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 491, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 495, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 504, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 508, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 513, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 518, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 523, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 533, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 538, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 542, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 551, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 568, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 577, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 585, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 590, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 595, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 599, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 604, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 609, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 615, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 620, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 628, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 633, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 638, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 646, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 650, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 655, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 660, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 664, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 668, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 680, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 684, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 689, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 694, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 703, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 715, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 719, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 723, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 727, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 731, .adv_w = 96, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 738, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 743, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 749, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 754, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 760, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 766, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 771, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 776, .adv_w = 96, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 783, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 788, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 792, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 797, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 801, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 805, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 810, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 816, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 821, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 825, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 829, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 834, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 842, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 846, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 854, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 859, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 864, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 868, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 872, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 877, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 881, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 886, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 890, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 898, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 902, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 906, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 910, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 914, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 923, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 927, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 931, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 935, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 940, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 945, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 951, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 956, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 962, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 968, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 973, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 977, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 981, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 986, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 990, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 994, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 999, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1005, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1010, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1015, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1019, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1024, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1029, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1033, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1037, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1041, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1046, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1050, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1054, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1058, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1063, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1068, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1073, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1077, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1082, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1087, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1092, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1096, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1101, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1106, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1111, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1116, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1122, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1127, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1133, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1138, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1144, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1149, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1154, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1160, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1165, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1170, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1175, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1181, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1186, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1192, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1197, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1202, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1207, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1213, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1219, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1224, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1229, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1233, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1237, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1242, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1247, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1252, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1256, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1260, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1264, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1268, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1273, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1279, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1284, .adv_w = 96, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1287, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1291, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1296, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1301, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1305, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1309, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1313, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1318, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1323, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1327, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1331, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1336, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1341, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1345, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1350, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1354, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1358, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1362, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1366, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1370, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1375, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1379, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1384, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1389, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1393, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1398, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1402, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1406, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1410, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1415, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1420, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1425, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1429, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1434, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1439, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1443, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1447, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1452, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1456, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1462, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1468, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1472, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1478, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1483, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1488, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1493, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1498, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1502, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1506, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1510, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1514, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1520, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1525, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1531, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1536, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1541, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1545, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1550, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1555, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1560, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1565, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1570, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1575, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1580, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1584, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1588, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1593, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1598, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1603, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1608, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1613, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1619, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1625, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1630, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1636, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1641, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1646, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1652, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1657, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1662, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1666, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1671, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1676, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1682, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1686, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1690, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1694, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1698, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1702, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1707, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1711, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1715, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1719, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1723, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1728, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1733, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1738, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1744, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1748, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1753, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1759, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1763, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1768, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1772, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1778, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1784, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1790, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1794, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1799, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1805, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1811, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1816, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1822, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1827, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1831, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1836, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1840, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1845, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1850, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1855, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1860, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1864, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1869, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1873, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1877, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1881, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1887, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1891, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1895, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1900, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1905, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1909, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1915, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1921, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1925, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1930, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1935, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1939, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1943, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1947, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1952, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1957, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1961, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1965, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1970, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1976, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1980, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1985, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1989, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1993, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1998, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2002, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2007, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2012, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2017, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2021, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2026, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2030, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2035, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2040, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2044, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2049, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2054, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2059, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2063, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2069, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2074, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2080, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2086, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2091, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2097, .adv_w = 96, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2104, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2110, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2115, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2119, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2124, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2129, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2134, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2138, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2142, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2146, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2150, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2154, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2158, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2163, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2168, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2172, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2177, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2181, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2186, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2190, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2194, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2198, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2202, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2206, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2210, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2214, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2218, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2223, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2228, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2232, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2236, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2240, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2245, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2249, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2255, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2259, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2265, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 2271, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2276, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2281, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2285, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2289, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2293, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2298, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2303, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2307, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2311, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2316, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2320, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2324, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2328, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2332, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2336, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2340, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2345, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2350, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2354, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2359, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2364, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2368, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2372, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2377, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2382, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2386, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2391, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2395, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2400, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2405, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2409, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2413, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2417, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2421, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 2425, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2429, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2433, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2437, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2441, .adv_w = 96, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2447, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2452, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x7, 0x9, 0xa, 0xb, 0xd, 0xe, 0x2a,
    0x2d, 0x32, 0x3a, 0x3b, 0x45, 0x49, 0x66, 0x8b,
    0x8c, 0x8e, 0x92, 0x94, 0xa7, 0xae, 0xc5, 0xce,
    0xd3, 0xd6, 0xe3, 0xf6, 0xfb, 0x118, 0x11a, 0x146,
    0x14d, 0x14e, 0x153, 0x15c, 0x17f, 0x19b, 0x19d, 0x1a7,
    0x1dd, 0x1e1, 0x1ee, 0x219, 0x23c, 0x247, 0x24f, 0x25a,
    0x25c, 0x2cf, 0x345, 0x348, 0x349, 0x34d, 0x365, 0x368,
    0x36c, 0x371, 0x373, 0x376, 0x377, 0x37c, 0x385, 0x38c,
    0x399, 0x3b2, 0x3c6, 0x3cf, 0x3fa, 0x406, 0x407, 0x417,
    0x41b, 0x41d, 0x420, 0x430, 0x436, 0x437, 0x44d, 0x49f,
    0x4a0, 0x4a1, 0x4a8, 0x4a9, 0x505, 0x516, 0x53a, 0x547,
    0x54a, 0x555, 0x560, 0x561, 0x570, 0x573, 0x58b, 0x59f,
    0x5c2, 0x5cc, 0x5cd, 0x5d1, 0x5d6, 0x5e3, 0x5e5, 0x5ea,
    0x5ef, 0x5f0, 0x5f3, 0x5f7, 0x604, 0x608, 0x60c, 0x60d,
    0x60e, 0x611, 0x626, 0x62f, 0x638, 0x67c, 0x6d1, 0x72f,
    0x809, 0x868, 0x8de, 0x8e0, 0x8fa, 0x8fe, 0x928, 0x930,
    0x93a, 0x94f, 0xa06, 0xa5e, 0xa6b, 0xaf0, 0xb04, 0xb07,
    0xb0d, 0xb16, 0xb27, 0xb31, 0xb34, 0xb82, 0xbcb, 0xd50,
    0xd57, 0xd58, 0xd5f, 0xd9a, 0xd9e, 0xdb9, 0xdbd, 0xdf8,
    0xdf9, 0xdfc, 0xe0f, 0xe1d, 0xe3a, 0xe40, 0xe42, 0xe4f,
    0xf4c, 0xfe5, 0xfe6, 0xff1, 0xff2, 0x1003, 0x1027, 0x1038,
    0x1055, 0x1073, 0x1076, 0x107f, 0x108f, 0x1093, 0x1094, 0x1095,
    0x10a6, 0x10fa, 0x1100, 0x1102, 0x110f, 0x1115, 0x1153, 0x1155,
    0x1169, 0x1184, 0x1185, 0x11aa, 0x11c3, 0x11c5, 0x11c6, 0x11d7,
    0x1201, 0x1227, 0x123b, 0x1262, 0x126f, 0x140f, 0x1410, 0x1416,
    0x142a, 0x1437, 0x144d, 0x1453, 0x146b, 0x147e, 0x148a, 0x1496,
    0x14c9, 0x14df, 0x14e6, 0x14e9, 0x1501, 0x1502, 0x1507, 0x1509,
    0x155f, 0x1562, 0x156e, 0x158c, 0x15a2, 0x15a5, 0x15a7, 0x15a8,
    0x15cf, 0x15d0, 0x15d2, 0x16ad, 0x16cd, 0x16ce, 0x172f, 0x1736,
    0x173e, 0x1748, 0x1770, 0x1774, 0x1787, 0x17a5, 0x17ad, 0x17b0,
    0x17b9, 0x17e0, 0x17e5, 0x17e7, 0x17f6, 0x180e, 0x182f, 0x183e,
    0x186f, 0x1882, 0x18f4, 0x18ff, 0x1909, 0x190d, 0x191f, 0x192a,
    0x192c, 0x193a, 0x1965, 0x1981, 0x1990, 0x199c, 0x19c4, 0x19e5,
    0x1a07, 0x1a0f, 0x1a21, 0x1a37, 0x1a38, 0x1a39, 0x1a3c, 0x1ac0,
    0x1b0d, 0x1b82, 0x1c21, 0x1c2a, 0x1d21, 0x1d62, 0x1d63, 0x1d64,
    0x1d65, 0x1db5, 0x1dcf, 0x1dd4, 0x1e38, 0x1e49, 0x1e60, 0x1ebf,
    0x1ec4, 0x1ee8, 0x1f41, 0x1f4b, 0x1f4f, 0x1f88, 0x1ff1, 0x2038,
    0x2090, 0x20d1, 0x210f, 0x226f, 0x22b9, 0x22c1, 0x22ed, 0x230a,
    0x2326, 0x2406, 0x2447, 0x2448, 0x2459, 0x2469, 0x2479, 0x24b6,
    0x24ec, 0x2587, 0x25af, 0x25b0, 0x25bb, 0x25e0, 0x2606, 0x2683,
    0x271f, 0x2728, 0x2731, 0x2735, 0x2745, 0x274c, 0x2759, 0x287d,
    0x2884, 0x28f8, 0x29e5, 0x29e9, 0x29ed, 0x2a01, 0x2a6c, 0x2a6e,
    0x2b3a, 0x2bd2, 0x2bef, 0x2bf0, 0x2bfb, 0x2c0b, 0x2c33, 0x2c7a,
    0x2c97, 0x2ccb, 0x2d26, 0x2d2c, 0x2d49, 0x2d7e, 0x2d80, 0x2d97,
    0x2da1, 0x2dad, 0x2e4d, 0x2efb, 0x2f20, 0x2f22, 0x2f2f, 0x30a2,
    0x30a6, 0x30bf, 0x30c4, 0x30c6, 0x30c8, 0x30cf, 0x30d3, 0x30d8,
    0x30dc, 0x30df, 0x30ed, 0x30fc, 0x30ff, 0x3113, 0x3116, 0x3129,
    0x3151, 0x316e, 0x31fb, 0x3203, 0x320c, 0x325a, 0x32cc, 0x32fd,
    0x331a, 0x33ea, 0x33f4, 0x342a, 0x3472, 0x3482, 0x34af, 0x34e5,
    0x35dc, 0x36dd, 0x3879, 0x3a4c, 0x3a68, 0x3a70, 0x3bc1, 0x3bc8,
    0x3bd2, 0x3be3, 0x3be6, 0x3da4, 0x3db0, 0x3dbe, 0x3dbf, 0x3dc1,
    0x3dca, 0x3dd1, 0x3dd5, 0x3de2, 0x3de5, 0x3de6, 0x3ded, 0x3def,
    0x3df4, 0x3dfb, 0x3e03, 0x3f25, 0x3f56, 0x3f77, 0x3fd1, 0x3fef,
    0x3ff3, 0x416c, 0x416e, 0x416f, 0x417b, 0x417d, 0x4185, 0x4191,
    0x4193, 0x41b9, 0x41c7, 0x41d0, 0x41d4, 0x41d9, 0x41db, 0x41dc,
    0x41de, 0x4200, 0x4201, 0x4202, 0x4209, 0x421a, 0x423b, 0x4253,
    0x427f, 0x42e8, 0x43c7, 0x43ca, 0x43cc, 0x43cd, 0x43cf, 0x4688,
    0x469f, 0x4719, 0x472e, 0x475c, 0x477f, 0x47ea, 0x47ed, 0x47ee,
    0x47f2, 0x47f4, 0x4805, 0x481f, 0x4835, 0x4836, 0x483b, 0x4864,
    0x4900, 0x4959, 0x495e, 0x4960, 0x4962, 0x49f3, 0x4a75, 0x4a76,
    0x4a79, 0x4a7a, 0x4a7b, 0x4a84, 0x4a98, 0x4a9c, 0x4b96, 0x4c71,
    0x4c7b, 0x4cd8, 0x50c4, 0x50d8, 0x5150
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 19968, .range_length = 20817, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 493, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_lvgl_6 = {
#else
lv_font_t ui_font_lvgl_6 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 7,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_LVGL_6*/

