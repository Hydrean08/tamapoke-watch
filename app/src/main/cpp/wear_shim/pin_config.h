#pragma once

#include "board.h"

#define LCD_SDIO0 0
#define LCD_SDIO1 0
#define LCD_SDIO2 0
#define LCD_SDIO3 0
#define LCD_SCLK  0
#define LCD_CS    0
#define LCD_RESET 0
#define LCD_WIDTH  466
#define LCD_HEIGHT 466

#define IIC_SDA 0
#define IIC_SCL 0
#define TP_INT  0
#define TP_RESET 0
#define TP_SDA 0
#define TP_SCL 0

#define DISPLAY_BEGIN_HZ 80000000
#define TOUCH_ADDR 0x5A
#define TOUCH_POLLING 0

#define HAS_PMU 1
#define HAS_RTC 1
#define HAS_AUDIO 1
#define HAS_SD 1
#define BOARD_NAME "Wear OS Watch"
