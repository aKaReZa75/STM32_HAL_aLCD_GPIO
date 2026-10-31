### Initialization

#### `void alcd_init(void)`

**Description:**  
Initializes the LCD module. This function **must be called first** before using any other LCD function.

**Operation:**
- Confirms GPIO pins are configured as outputs (this is done in CubeMX)
- Executes HD44780 initialization sequence
- Sets 4-bit or 8-bit interface mode
- Enables the display
- Clears the screen

**Parameters:**  
None

**Returns:**  
None

**Example:**
```c
#include "aKaReZa.h"
#include "alcd.h"

int main(void) 
{
    alcd_init(); // Initialize LCD - Must be called first
    
    // Your code here
    
    while(1)
    {
        // Main loop
    }
}
```

> [!IMPORTANT]
> Currently, this library supports **only 16×2 LCD displays**. For different sizes, library modifications may be required.

---

### Display Control

#### `void alcd_clear(void)`

**Description:**  
Clears the entire display and returns the cursor to the home position (0, 0).

**Parameters:**  
None

**Returns:**  
None

**Execution Time:**  
~1.52ms (longest command execution time)

**Example:**
```c
alcd_clear(); // Clear display and reset cursor
```

---

#### `void alcd_display(bool _alcd_Display, bool _alcd_Cursor, bool _alcd_Blink)`

**Description:**  
Controls the display, cursor, and blink settings.

**Parameters:**
- `_alcd_Display` - Display state
  - `true`: Turn display ON
  - `false`: Turn display OFF
- `_alcd_Cursor` - Cursor visibility
  - `true`: Show cursor (underline)
  - `false`: Hide cursor
- `_alcd_Blink` - Cursor blink state
  - `true`: Enable blinking (block cursor)
  - `false`: Disable blinking

**Returns:**  
None

**Default State:**  
Display ON, Cursor OFF, Blink OFF

**Examples:**
```c
// Display ON, cursor and blink OFF (default)
alcd_display(true, false, false);

// Display ON, cursor ON (underline), blink OFF
alcd_display(true, true, false);

// Display ON, cursor OFF, blink ON (block blink)
alcd_display(true, false, true);

// Display ON, cursor ON with blink (underline + block)
alcd_display(true, true, true);

// Turn off display completely
alcd_display(false, false, false);
```

> [!NOTE]
> By default, the display is ON. You don't need to explicitly enable it after initialization.


### Cursor Control

#### `void alcd_gotoxy(uint8_t _alcd_x, uint8_t _alcd_y)`

**Description:**  
Moves the cursor to the specified position on the LCD.

**Parameters:**
- `_alcd_x` - Column position (0 to 15)
- `_alcd_y` - Row position (0 or 1)

**Returns:**  
None

**Valid Ranges:**
- Column: 0-15 (16 columns)
- Row: 0-1 (2 rows)

**Examples:**
```c
alcd_gotoxy(0, 0);   // First column, first row (home position)
alcd_gotoxy(5, 1);   // Sixth column, second row
alcd_gotoxy(15, 0);  // Last column, first row
alcd_gotoxy(0, 1);   // First column, second row
```

> [!WARNING]
> - First row index is `0`, second row index is `1` (not 1 and 2)
> - Exceeding valid ranges may cause unexpected behavior

---

### Data Writing

#### `void alcd_write(uint8_t _data, bool _alcd_cmdData)`

**Description:**  
Low-level function to send a command or data byte to the LCD.

**Parameters:**
- `_data` - 8-bit value to send (command or data)
- `_alcd_cmdData` - Byte type selector
  - `__alcd_writeCmd` (false): Send as command
  - `__alcd_writeData` (true): Send as data

**Returns:**  
None

**Execution Time:**  
Minimum 37μs per command

**Examples:**
```c
// Write character 'A' (ASCII 0x41)
alcd_write(0x41, __alcd_writeData);

// Send clear display command
alcd_write(__alcd_Display_Clear, __alcd_writeCmd);

// Move cursor to second line
alcd_write(__alcd_Line2_Start, __alcd_writeCmd);

// Write character 'Z'
alcd_write('Z', __alcd_writeData);
```

> [!NOTE]
> This is a low-level function. For normal text display, use `alcd_putc()` or `alcd_puts()` instead.

---

#### `void alcd_putc(char _char)`

**Description:**  
Displays a single character at the current cursor position.

**Parameters:**
- `_char` - Character to display (ASCII character)

**Returns:**  
None

**Examples:**
```c
alcd_putc('A');      // Display 'A'
alcd_putc('9');      // Display '9'
alcd_putc(' ');      // Display space
alcd_putc(0);        // Display custom character 0 (from CGRAM)
```

---

#### `void alcd_puts(char* _str)`

**Description:**  
Displays a null-terminated string starting at the current cursor position.

**Parameters:**
- `_str` - Pointer to null-terminated string

**Returns:**  
None

**Examples:**
```c
alcd_puts("Hello");           // Display "Hello"
alcd_puts("LCD Display");     // Display "LCD Display"

char message[] = "STM32 MCU";
alcd_puts(message);           // Display "STM32 MCU"
```

**Complete Example:**
```c
#include "aKaReZa.h"
#include "alcd.h"

int main(void) 
{
    alcd_init();
    
    alcd_gotoxy(0, 0);           // First row
    alcd_putc('X');              // Single character
    
    alcd_gotoxy(0, 1);           // Second row
    alcd_puts("Hello, World!");  // String
    
    while(1)
    {
        // Main loop
    }
}
```

---

### Backlight Control

#### `void alcd_backLight(bool _alcd_BL)`

**Description:**  
Controls the LCD backlight (if enabled via `__alcd_useBL` macro).

**Parameters:**
- `_alcd_BL` - Backlight state
  - `true`: Turn backlight ON
  - `false`: Turn backlight OFF

**Returns:**  
None

**Availability:**  
Only available when `#define __alcd_useBL true` is set in `alcd.h`

**Examples:**
```c
// Turn backlight ON
alcd_backLight(true);

// Turn backlight OFF
alcd_backLight(false);

// Blinking backlight example
while(1)
{
    alcd_backLight(true);
    HAL_Delay(500);
    alcd_backLight(false);
    HAL_Delay(500);
}
```

> [!IMPORTANT]
> - The backlight pin should be connected through an NPN transistor
> - Typical LCD backlight current: 20-150mA
> - Direct connection to MCU pin may damage the microcontroller
> - Consider power consumption in battery-powered applications

---

### Custom Characters

#### `void alcd_customChar(uint8_t _alcd_CGRAMadd, const uint8_t *_alcd_CGRAMdata)`

**Description:**  
Creates a custom character and stores it in the LCD's CGRAM (Character Generator RAM).

**Parameters:**
- `_alcd_CGRAMadd` - CGRAM address (0-7)
- `_alcd_CGRAMdata` - Pointer to 8-byte array defining the character pattern

**Returns:**  
None

**Character Format:**
- Each byte represents one row (8 rows total)
- Only lower 5 bits are used (5×8 pixel characters)
- Top to bottom arrangement

**CGRAM Limits:**
- Maximum 8 custom characters (addresses 0-7)
- Custom characters accessible via ASCII codes 0-7

**Examples:**

```c
#include "aKaReZa.h"
#include "alcd.h"

// Define custom characters
uint8_t heart[8] = 
{
    0b00000,  // Row 1: .....
    0b01010,  // Row 2: .#.#.
    0b11111,  // Row 3: #####
    0b11111,  // Row 4: #####
    0b01110,  // Row 5: .###.
    0b00100,  // Row 6: ..#..
    0b00000,  // Row 7: .....
    0b00000   // Row 8: .....
};

uint8_t smiley[8] = 
{
    0b00000,  // Row 1: .....
    0b01010,  // Row 2: .#.#.
    0b01010,  // Row 3: .#.#.
    0b00000,  // Row 4: .....
    0b10001,  // Row 5: #...#
    0b01110,  // Row 6: .###.
    0b00000,  // Row 7: .....
    0b00000   // Row 8: .....
};

uint8_t arrow[8] = 
{
    0b00100,  // Row 1: ..#..
    0b00110,  // Row 2: ..##.
    0b00111,  // Row 3: ..###
    0b00110,  // Row 4: ..##.
    0b00100,  // Row 5: ..#..
    0b00000,  // Row 6: .....
    0b00000,  // Row 7: .....
    0b00000   // Row 8: .....
};

int main(void) 
{
    alcd_init();
    
    // Store custom characters in CGRAM
    alcd_customChar(0, heart);   // Address 0
    alcd_customChar(1, smiley);  // Address 1
    alcd_customChar(2, arrow);   // Address 2
    
    // Display custom characters
    alcd_gotoxy(0, 0);
    alcd_putc(0);  // Display heart
    alcd_putc(' ');
    alcd_putc(1);  // Display smiley
    alcd_putc(' ');
    alcd_putc(2);  // Display arrow
    
    while(1)
    {
        // Main loop
    }
}
```

> [!TIP]
> **Online Custom Character Generators:**
> - [LCD Character Creator](https://maxpromer.github.io/LCD-Character-Creator/)
> - [HD44780 Custom Character Generator](https://www.quinapalus.com/hd44780udg.html)
> 
> These tools provide visual character design and generate the byte arrays automatically.

**Best Practices:**
- Define custom characters during initialization
- Store frequently used characters
- CGRAM data persists until power off or reinitialization
- Use descriptive names for character arrays

---

## Function Summary Table

| Function | Purpose | Mode Support |
|----------|---------|--------------|
| `alcd_init()` | Initialize LCD module | 4-bit / 8-bit |
| `alcd_clear()` | Clear display and reset cursor | 4-bit / 8-bit |
| `alcd_display(d, c, b)` | Control display, cursor, blink | 4-bit / 8-bit |
| `alcd_gotoxy(x, y)` | Move cursor to position | 4-bit / 8-bit |
| `alcd_write(data, type)` | Send command or data byte | 4-bit / 8-bit |
| `alcd_putc(char)` | Display single character | 4-bit / 8-bit |
| `alcd_puts(string)` | Display string | 4-bit / 8-bit |
| `alcd_backLight(state)` | Control backlight | 4-bit / 8-bit |
| `alcd_customChar(addr, data)` | Create custom character | 4-bit / 8-bit |

# 🌟 Support Me
If you found this repository useful:
- Subscribe to my [YouTube Channel](https://www.youtube.com/@aKaReZa75).
- Share this repository with others.
- Give this repository and my other repositories a star.
- Follow my [GitHub account](https://github.com/aKaReZa75).

# ✉️ Contact Me
Feel free to reach out to me through any of the following platforms:
- 📧 [Email: aKaReZa75@gmail.com](mailto:aKaReZa75@gmail.com)
- 🎥 [YouTube: @aKaReZa75](https://www.youtube.com/@aKaReZa75)
- 💼 [LinkedIn: @akareza75](https://www.linkedin.com/in/akareza75)