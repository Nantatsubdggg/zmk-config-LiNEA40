/*
 * Battery Level Typing Behavior for ZMK
 *
 * キーを押すとバッテリー残量をキーストロークとして送信する: "L:50% R:80%"
 * ホストOS側のキーボードレイアウトはUS配列を前提としている
 */

#define DT_DRV_COMPAT zmk_behavior_battery_type

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/battery.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/bluetooth/central.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* キーストローク間のディレイ（ms）。BLEで文字落ちする場合は値を大きくする */
#define KEYSTROKE_DELAY_MS 30

/* 出力フォーマット定義 */
#define FORMAT_PREFIX   "L:"
#define FORMAT_MIDDLE   "% R:"
#define FORMAT_SUFFIX   "%"
#define FORMAT_NO_DATA  "--"

/* 文字→HIDキーコードのマッピング構造体 */
struct char_keycode {
    uint32_t keycode;
    bool shift;
};

/* HID Usage IDs */
#define HID_KEY_A          0x04
#define HID_KEY_B          0x05
#define HID_KEY_L          0x0F
#define HID_KEY_R          0x15
#define HID_KEY_1          0x1E
#define HID_KEY_2          0x1F
#define HID_KEY_3          0x20
#define HID_KEY_4          0x21
#define HID_KEY_5          0x22
#define HID_KEY_6          0x23
#define HID_KEY_7          0x24
#define HID_KEY_8          0x25
#define HID_KEY_9          0x26
#define HID_KEY_0          0x27
#define HID_KEY_SPACE      0x2C
#define HID_KEY_MINUS      0x2D
#define HID_KEY_SEMICOLON  0x33
#define HID_KEY_LSHIFT     0xE1

static const struct char_keycode CHAR_MAP[] = {
    ['0'] = { .keycode = HID_KEY_0, .shift = false },
    ['1'] = { .keycode = HID_KEY_1, .shift = false },
    /* ... 2〜9も同様 ... */
    ['L'] = { .keycode = HID_KEY_L, .shift = true },
    ['R'] = { .keycode = HID_KEY_R, .shift = true },
    [' '] = { .keycode = HID_KEY_SPACE, .shift = false },
    [':'] = { .keycode = HID_KEY_SEMICOLON, .shift = true },  /* US: Shift+; */
    ['%'] = { .keycode = HID_KEY_5, .shift = true },           /* US: Shift+5 */
    ['-'] = { .keycode = HID_KEY_MINUS, .shift = false },
};

#define CHAR_MAP_SIZE (sizeof(CHAR_MAP) / sizeof(CHAR_MAP[0]))
