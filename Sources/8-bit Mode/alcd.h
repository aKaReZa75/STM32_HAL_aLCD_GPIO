/**
 ******************************************************************************
 * @file     alcd.h
 * @brief    16x2 Alphanumeric LCD display library header for STM32 HAL
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This library provides a complete interface for HD44780-compatible
 *           LCD displays using 8-bit parallel communication mode via STM32 HAL.
 * 
 * @note     FUNCTION SUMMARY:
 *           - alcd_init       : Initialize LCD in 8-bit mode with proper HD44780 timing sequence
 *           - alcd_write      : Low-level function to send command/data bytes in 8-bit mode
 *           - alcd_putc       : Print single character at current cursor position with auto-wrap
 *           - alcd_puts       : Print null-terminated string starting at current cursor position
 *           - alcd_gotoxy     : Position cursor at specific row (0-1) and column (0-15)
 *           - alcd_clear      : Clear entire display content and reset cursor to home (0,0)
 *           - alcd_display    : Configure display ON/OFF, cursor visibility, and blink state
 *           - alcd_customChar : Create custom 5x8 pixel character in CGRAM (addresses 0-7)
 *           - alcd_backLight  : Control LCD backlight ON/OFF (if backlight pin defined)
 * 
 * @note     Hardware Requirements:
 *           - 10 GPIO pins for LCD control (RS, EN, DB0-DB7)
 *           - Optional: 1 GPIO pin for backlight control
 *           - STM32 microcontroller with HAL library
 * 
 * @note     Usage:
 *           1. Configure GPIO pins in STM32CubeMX or manually in aKaReZa.h
 *           2. Call alcd_init() once at startup
 *           3. Use alcd_puts()/alcd_putc() for text output
 *           4. Use alcd_gotoxy() for cursor positioning
 *           5. Use alcd_customChar() for special symbols
 * 
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/STM32_HAL_aLCD_GPIO
 ******************************************************************************
 */
#ifndef _alcd_H_
#define _alcd_H_

#include "aKaReZa.h"


/* ============================================================================
 *                         COMMAND/DATA MODE SELECTION
 * ============================================================================ */
#define __alcd_writeCmd   0                  /**< Command mode for alcd_write() - sends LCD instructions */
#define __alcd_writeData  1                  /**< Data mode for alcd_write() - sends character data to display */


/* ============================================================================
 *                         DISPLAY DIMENSIONS
 * ============================================================================ */
#define __alcd_max_x  16                     /**< Maximum number of columns (0-15) */
#define __alcd_max_y  2                      /**< Maximum number of rows (0-1) */


/* ============================================================================
 *                         TIMING CONFIGURATION
 * ============================================================================ */
#define __alcd_delay(_delayValue)  delay_us(_delayValue)  /**< Delay macro using microsecond delay function from aKaReZa library */
#define __alcd_delay_CMD      50             /**< Standard command execution time in microseconds */
#define __alcd_delay_modeSet  5000           /**< Mode setting and clear command time in microseconds */
#define __alcd_delay_powerON  50000          /**< Power-on stabilization time in microseconds (50ms) */


/* ============================================================================
 *                         FUNCTION SET COMMANDS
 * ============================================================================ */
/* 8-bit mode commands (for reference only - not used in this implementation) */
#define __alcd_Mode_8bit_2line_5x8   0x38    /**< 8-bit interface, 2-line display, 5x8 font */
#define __alcd_Mode_8bit_1line_5x8   0x30    /**< 8-bit interface, 1-line display, 5x8 font */

/* 4-bit mode commands (used in this implementation) */
#define __alcd_Mode_4bit_2line_5x8   0x28    /**< 4-bit interface, 2-line display, 5x8 font */
#define __alcd_Mode_4bit_1line_5x8   0x20    /**< 4-bit interface, 1-line display, 5x8 font */
#define __alcd_Mode_4bit_Step1       0x33    /**< Initialize LCD for 4-bit mode (sends 0x03 twice) */
#define __alcd_Mode_4bit_Step2       0x32    /**< Set 4-bit mode (sends 0x03 then 0x02) */


/* ============================================================================
 *                         DISPLAY CONTROL COMMANDS
 * ============================================================================ */
#define __alcd_Display_Clear 0x01            /**< Clear entire display and return cursor to home */
#define __alcd_Display_Home  0x02            /**< Return cursor to home position (0,0) without clearing */
#define __alcd_Display_OFF   0x08            /**< Turn display OFF, cursor OFF, blink OFF */
#define __alcd_Display_ON    0x0C            /**< Turn display ON, cursor OFF, blink OFF */


/* ============================================================================
 *                         CURSOR CONTROL COMMANDS
 * ============================================================================ */
#define __alcd_Cursor_OFF    0x0C            /**< Display ON, cursor OFF, blink OFF */
#define __alcd_Cursor_ON     0x0E            /**< Display ON, underline cursor ON, blink OFF */
#define __alcd_Cursor_Blink  0x0D            /**< Display ON, cursor OFF, blink ON (blinking block) */
#define __alcd_Cursor_dashBLINK   0x0F       /**< Display ON, underline cursor ON, blink ON (both effects) */


/* ============================================================================
 *                         ENTRY MODE SET COMMANDS
 * ============================================================================ */
#define __alcd_Entry_Inc     0x06            /**< Increment cursor position (shift right after write) */
#define __alcd_Entry_Dec     0x04            /**< Decrement cursor position (shift left after write) */
#define __alcd_Entry_Shift   0x07            /**< Increment cursor with entire display shift */


/* ============================================================================
 *                         DDRAM ADDRESS COMMANDS
 * ============================================================================ */
#define __alcd_Line1_Start   0x80            /**< DDRAM start address for first line (row 0) */
#define __alcd_Line2_Start   0xC0            /**< DDRAM start address for second line (row 1) */


/* ============================================================================
 *                         CGRAM ADDRESS COMMAND
 * ============================================================================ */
#define __alcd_CGRAM_Start   0x40            /**< CGRAM start address for custom character generation (8 characters, 0-7) */


/* ============================================================================
 *                         FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Initialize LCD in 8-bit mode
 */
void alcd_init(void);

/**
 * @brief Send command or data byte to LCD in 8-bit mode
 */
void alcd_write(uint8_t _data, bool _alcd_cmdData);

/**
 * @brief Print single character at current cursor position
 */
void alcd_putc(char _char);

/**
 * @brief Print null-terminated string at current cursor position
 */
void alcd_puts(char* _str);

/**
 * @brief Move cursor to specified row and column
 */
void alcd_gotoxy(uint8_t _alcd_x, uint8_t _alcd_y);

/**
 * @brief Clear display and reset cursor to home position
 */
void alcd_clear(void);

/**
 * @brief Configure display, cursor, and blink settings
 */
void alcd_display(bool _alcd_Display, bool _alcd_Cursor, bool _alcd_Blink);

/**
 * @brief Create custom character in CGRAM memory
 */
void alcd_customChar(uint8_t _alcd_CGRAMadd, const uint8_t *_alcd_CGRAMdata);

/**
 * @brief Control LCD backlight (if backlight GPIO is defined)
 */
#ifdef __alcd_BL_GPIO_Port
    void alcd_backLight(bool _alcd_BL);
#endif

#endif /* _alcd_H_ */