/**
 ******************************************************************************
 * @file     alcd.c
 * @brief    16x2 Alphanumeric LCD display library implementation for STM32 HAL
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This library provides:
 *           - 4-bit parallel interface control for HD44780 compatible LCDs
 *           - Custom character generation (CGRAM)
 *           - Cursor positioning and display control
 *           - Backlight control support via STM32 HAL GPIO
 * 
 * @note     FUNCTION SUMMARY:
 *           Initialization & Control:
 *           - alcd_init      : Initialize LCD in 4-bit mode with HD44780 sequence
 *           - alcd_display   : Configure display, cursor, and blink settings
 *           - alcd_clear     : Clear entire display and reset cursor to home
 *           - alcd_backLight : Control LCD backlight ON/OFF (if enabled)
 *
 *           Cursor & Positioning:
 *           - alcd_gotoxy    : Move cursor to specific row and column position
 *           - alcd_putc      : Print single character with automatic line wrapping
 *           - alcd_puts      : Print null-terminated string at current position
 *
 *           Custom Characters:
 *           - alcd_customChar: Create custom character in CGRAM memory
 *
 *           Low-Level Functions:
 *           - alcd_write     : Send data/command to LCD in 4-bit mode using HAL
 *
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/STM32_HAL_aLCD_GPIO
 ******************************************************************************
 */

#include "alcd.h"


/* ============================================================================
 *                         GLOBAL VARIABLES
 * ============================================================================ */
bool __alcd_initStatus = false;          /**< LCD initialization status flag - false during init, true after */
uint8_t __alcd_x_position = 0;           /**< Current cursor column position (0-15) */
uint8_t __alcd_y_position = 0;           /**< Current cursor row position (0-1) */


/* ============================================================================
 *                      CUSTOM CHARACTER FUNCTIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Create custom character in CGRAM
 * @param _alcd_CGRAMadd: CGRAM address (0-7) for custom character location
 * @param _alcd_CGRAMdata: Pointer to 8-byte array containing character pattern
 * @retval None
 * @note Each character is 5x8 pixels, represented by 8 bytes
 *       Each byte defines one row (5 LSBs used)
 *       After writing, cursor automatically returns to previous position
 * ------------------------------------------------------- */
void alcd_customChar(uint8_t _alcd_CGRAMadd, const uint8_t *_alcd_CGRAMdata)
{
    uint8_t _forCounter = 0;                                       /**< Loop counter for iterating through 8 character bytes */
    
    /* Calculate CGRAM address: base address + (character_index * 8) */
    uint8_t _CG_Add = __alcd_CGRAM_Start + (_alcd_CGRAMadd << 3);  /**< Shift left by 3 equals multiply by 8 */
    
    /* Write all 8 bytes of character pattern to CGRAM */
    for(_forCounter = 0; _forCounter < 8; _forCounter++)           /**< Loop through 8 rows of character pattern */
    {
        alcd_write(_CG_Add++, __alcd_writeCmd);                    /**< Set CGRAM address for current row */
        alcd_write(*_alcd_CGRAMdata++, __alcd_writeData);          /**< Write pattern data for current row */
    };
    
    alcd_gotoxy(__alcd_x_position, __alcd_y_position);             /**< Restore cursor to position before CGRAM write */
};


/* ============================================================================
 *                       BACKLIGHT CONTROL
 * ============================================================================ */

#ifdef __alcd_BL_GPIO_Port
/* -------------------------------------------------------
 * @brief Control LCD backlight state
 * @param _alcd_BL: Backlight state (true=ON/GPIO_PIN_SET, false=OFF/GPIO_PIN_RESET)
 * @retval None
 * @note Only available if __alcd_BL_GPIO_Port is defined in configuration
 *       Uses STM32 HAL GPIO function for direct pin control
 * ------------------------------------------------------- */
void alcd_backLight(bool _alcd_BL)
{
    HAL_GPIO_WritePin(__alcd_BL_GPIO_Port, __alcd_BL_Pin, _alcd_BL);  /**< Set backlight GPIO pin to requested state */
};
#endif


/* ============================================================================
 *                       DISPLAY CONTROL FUNCTIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Configure display, cursor, and blink settings
 * @param _alcd_Display: Display ON/OFF (true=visible, false=blank)
 * @param _alcd_Cursor: Cursor visibility ON/OFF (true=underline visible, false=hidden)
 * @param _alcd_Blink: Cursor blink ON/OFF (true=blinking block, false=steady)
 * @retval None
 * @note All three settings can be configured independently
 *       Settings are combined into single command byte using bit manipulation
 * ------------------------------------------------------- */
void alcd_display(bool _alcd_Display, bool _alcd_Cursor, bool _alcd_Blink)
{
    /* Start with base display OFF command (0x08) */
    uint8_t _cursorState = __alcd_Display_OFF;                     /**< Initialize with all features disabled */
    
    bitChange(_cursorState, 0, _alcd_Blink);                       /**< Set bit 0: cursor blink enable */
    bitChange(_cursorState, 1, _alcd_Cursor);                      /**< Set bit 1: cursor visibility enable */
    bitChange(_cursorState, 2, _alcd_Display);                     /**< Set bit 2: display ON/OFF */
    
    alcd_write(_cursorState, __alcd_writeCmd);                     /**< Send combined display control command to LCD */
};

/* -------------------------------------------------------
 * @brief Clear entire display and reset cursor to home
 * @retval None
 * @note This function performs two operations:
 *       1. Sends clear command to LCD hardware
 *       2. Resets internal position tracking to (0,0)
 *       Requires extended delay for clear operation to complete
 * ------------------------------------------------------- */
void alcd_clear(void)
{
    alcd_write(__alcd_Display_Clear, __alcd_writeCmd);             /**< Send clear display command (0x01) */
    __alcd_x_position = 0;                                         /**< Reset column position to start */
    __alcd_y_position = 0;                                         /**< Reset row position to start */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait for clear operation (takes longer than normal commands) */
};


/* ============================================================================
 *                       CURSOR POSITIONING
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Move cursor to specific position on display
 * @param _alcd_x: Column position (0 to __alcd_max_x-1, typically 0-15)
 * @param _alcd_y: Row position (0 to __alcd_max_y-1, typically 0-1)
 * @retval None
 * @note Invalid positions (out of bounds) are clamped to (0,0)
 *       DDRAM addresses: Line 0 starts at 0x80, Line 1 starts at 0xC0
 * ------------------------------------------------------- */
void alcd_gotoxy(uint8_t _alcd_x, uint8_t _alcd_y)
{
    uint8_t _address = 0x00;                                       /**< Variable to hold calculated DDRAM address */

    /* Validate position boundaries */
    if(_alcd_x >= __alcd_max_x || _alcd_y >= __alcd_max_y)         /**< Check if position exceeds display dimensions */
    {
        __alcd_x_position = 0;                                     /**< Clamp to origin if invalid */
        __alcd_y_position = 0;
    }
    else                                                           /**< Position is valid */
    {
        __alcd_x_position = _alcd_x;                               /**< Update internal column tracker */
        __alcd_y_position = _alcd_y;                               /**< Update internal row tracker */
    };

    /* Calculate DDRAM address based on row */
    _address = (__alcd_y_position == 0) ? __alcd_Line1_Start : __alcd_Line2_Start;  /**< Select base address for row 0 or 1 */
    _address = _address + __alcd_x_position;                       /**< Add column offset to base address */

    alcd_write(_address, __alcd_writeCmd);                         /**< Send DDRAM address command (bit 7 already set in line start constants) */
};


/* ============================================================================
 *                       CHARACTER OUTPUT FUNCTIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Print null-terminated string to LCD
 * @param _str: Pointer to null-terminated character string
 * @retval None
 * @note String is printed starting at current cursor position
 *       Each character advances cursor and may trigger auto-wrap
 *       Continues until '\0' terminator is encountered
 * ------------------------------------------------------- */
void alcd_puts(char* _str)
{
    /* Iterate through string until null terminator */
    while (*_str != '\0')                                          /**< Check for end of string */
    {
        alcd_putc(*_str++);                                        /**< Print current character and advance pointer */
    };  
};

/* -------------------------------------------------------
 * @brief Print single character to LCD
 * @param _char: Character to print (ASCII value)
 * @retval None
 * @note Character is printed at current cursor position
 *       Automatically wraps to next line when reaching end of current line
 *       Cursor position is tracked internally for proper wrapping
 * ------------------------------------------------------- */
void alcd_putc(char _char)
{
    alcd_write(_char, __alcd_writeData);                           /**< Send character data to LCD */
    __alcd_x_position++;                                           /**< Advance to next column */
    
    /* Check for end of line condition */
    if(__alcd_x_position >= __alcd_max_x)                          /**< If cursor reaches end of line */
    {
        __alcd_x_position = 0;                                     /**< Reset to first column */
        __alcd_y_position++;                                       /**< Move to next row */
        alcd_gotoxy(__alcd_x_position, __alcd_y_position);         /**< Update cursor position on LCD */
    };
};


/* ============================================================================
 *                       LOW-LEVEL WRITE FUNCTIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Send data or command to LCD in 4-bit mode
 * @param _data: 8-bit data/command to send to LCD
 * @param _alcd_cmdData: Mode selection (false=Command, true=Data)
 * @retval None
 * @note 4-bit mode protocol:
 *       1. Set RS pin (0=command, 1=data)
 *       2. Send high nibble (bits 7-4) on DB7-DB4
 *       3. Pulse EN high then low
 *       4. Send low nibble (bits 3-0) on DB7-DB4
 *       5. Pulse EN high then low
 *       Uses longer delays during initialization phase
 * ------------------------------------------------------- */
void alcd_write(uint8_t _data, bool _alcd_cmdData)
{
    /* Set command/data mode */
    HAL_GPIO_WritePin(__alcd_RS_GPIO_Port, __alcd_RS_Pin, _alcd_cmdData);  /**< RS=0 for command, RS=1 for data */

    /* ===== SEND HIGH NIBBLE (bits 7-4) ===== */
    HAL_GPIO_WritePin(__alcd_DB4_GPIO_Port, __alcd_DB4_Pin, bitCheck(_data, 4));  /**< Send bit 4 of data */
    HAL_GPIO_WritePin(__alcd_DB5_GPIO_Port, __alcd_DB5_Pin, bitCheck(_data, 5));  /**< Send bit 5 of data */
    HAL_GPIO_WritePin(__alcd_DB6_GPIO_Port, __alcd_DB6_Pin, bitCheck(_data, 6));  /**< Send bit 6 of data */
    HAL_GPIO_WritePin(__alcd_DB7_GPIO_Port, __alcd_DB7_Pin, bitCheck(_data, 7));  /**< Send bit 7 of data */

    /* Latch high nibble with enable pulse */
    HAL_GPIO_WritePin(__alcd_EN_GPIO_Port, __alcd_EN_Pin, GPIO_PIN_SET);   /**< Enable high - start data latch */
    if(__alcd_initStatus == false)                                 /**< Check initialization status */
    {
        __alcd_delay(__alcd_delay_modeSet);                        /**< Use longer delay during initialization (5ms) */
    }
    else                                                           /**< Normal operation mode */
    {
        __alcd_delay(__alcd_delay_CMD);                            /**< Use shorter delay for normal commands (50us) */
    }
    HAL_GPIO_WritePin(__alcd_EN_GPIO_Port, __alcd_EN_Pin, GPIO_PIN_RESET); /**< Enable low - complete data latch */
    

    /* ===== SEND LOW NIBBLE (bits 3-0) ===== */
    HAL_GPIO_WritePin(__alcd_DB4_GPIO_Port, __alcd_DB4_Pin, bitCheck(_data, 0));  /**< Send bit 0 of data */
    HAL_GPIO_WritePin(__alcd_DB5_GPIO_Port, __alcd_DB5_Pin, bitCheck(_data, 1));  /**< Send bit 1 of data */
    HAL_GPIO_WritePin(__alcd_DB6_GPIO_Port, __alcd_DB6_Pin, bitCheck(_data, 2));  /**< Send bit 2 of data */
    HAL_GPIO_WritePin(__alcd_DB7_GPIO_Port, __alcd_DB7_Pin, bitCheck(_data, 3));  /**< Send bit 3 of data */

    /* Latch low nibble with enable pulse */
    HAL_GPIO_WritePin(__alcd_EN_GPIO_Port, __alcd_EN_Pin, GPIO_PIN_SET);   /**< Enable high - start data latch */
    if(__alcd_initStatus == false)                                 /**< Check initialization status */
    {
        __alcd_delay(__alcd_delay_modeSet);                        /**< Use longer delay during initialization (5ms) */
    }
    else                                                           /**< Normal operation mode */
    {
        __alcd_delay(__alcd_delay_CMD);                            /**< Use shorter delay for normal commands (50us) */
    }
    HAL_GPIO_WritePin(__alcd_EN_GPIO_Port, __alcd_EN_Pin, GPIO_PIN_RESET); /**< Enable low - complete data latch */
  
};


/* ============================================================================
 *                       INITIALIZATION FUNCTION
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Initialize LCD in 4-bit mode following HD44780 specification
 * @retval None
 * @note Initialization sequence (HD44780 datasheet compliant):
 *       1. Wait >40ms after Vcc rises to 4.5V (power-on delay)
 *       2. Send 0x33 - Initialize for 4-bit mode (sends 0x03 twice)
 *       3. Send 0x32 - Set 4-bit mode (sends 0x03 then 0x02)
 *       4. Send 0x28 - Function set: 4-bit, 2-line, 5x8 font
 *       5. Send 0x0C - Display ON, cursor OFF, blink OFF
 *       6. Send 0x0C - Ensure cursor is OFF
 *       7. Send 0x06 - Entry mode: increment cursor, no display shift
 *       8. Send 0x01 - Clear display
 * @note GPIO pins must be configured as outputs before calling this function
 *       Uses __alcd_initStatus flag to control timing during initialization
 * ------------------------------------------------------- */
void alcd_init(void)
{
    __alcd_initStatus = false;                                     /**< Mark as not initialized - enables longer delays */
    __alcd_delay(__alcd_delay_powerON);                            /**< Wait for LCD power stabilization (50ms) */

    /* Turn on backlight if available */
    #ifdef __alcd_BL_GPIO_Port
        HAL_GPIO_WritePin(__alcd_BL_GPIO_Port, __alcd_BL_Pin, GPIO_PIN_SET);  /**< Enable backlight at startup */
    #endif

    /* HD44780 initialization sequence for 4-bit mode */
    alcd_write(__alcd_Mode_4bit_Step1, __alcd_writeCmd);           /**< Step 1: Send 0x33 - prepare for 4-bit mode */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Mode_4bit_Step2, __alcd_writeCmd);           /**< Step 2: Send 0x32 - set 4-bit interface */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Mode_4bit_2line_5x8, __alcd_writeCmd);       /**< Function set: 4-bit, 2 lines, 5x8 dots */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Display_ON, __alcd_writeCmd);                /**< Display ON, cursor OFF, blink OFF */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Cursor_OFF, __alcd_writeCmd);                /**< Ensure cursor is OFF (redundant but ensures state) */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Entry_Inc, __alcd_writeCmd);                 /**< Entry mode: increment cursor, no display shift */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for command to execute */
    
    alcd_write(__alcd_Display_Clear, __alcd_writeCmd);             /**< Clear display and reset cursor to home */
    __alcd_delay(__alcd_delay_modeSet);                            /**< Wait 5ms for clear operation (takes longer) */
    
    __alcd_initStatus = true;                                      /**< Mark as initialized - enables shorter delays */
};