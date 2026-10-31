## Complete Examples

```c
#include "aKaReZa.h"
#include "alcd.h"

int main(void) 
{
    // Initialize LCD
    alcd_init();
    
    // Clear display
    alcd_clear();
    
    // Turn on backlight
    alcd_backLight(true);
    
    // Display text on first row
    alcd_gotoxy(0, 0);
    alcd_puts("Hello, World!");
    
    // Display text on second row
    alcd_gotoxy(0, 1);
    alcd_puts("aKaReZa");
    
    while(1)
    {
        // Main loop
    }
}
```

---

## Important Notes

### Initialization Requirements
- **Always call `alcd_init()` first** before using any other LCD function
- Wait for power stabilization (50ms) before initialization
- Initialization sequence differs between 4-bit and 8-bit modes

### Hardware Considerations
- **Verify pin definitions** in `alcd.h` match your actual hardware connections
- Most LCD modules have a **contrast potentiometer** - adjust for optimal visibility
- **Use appropriate current-limiting resistors** for backlight LED
- **Backlight transistor** is recommended for current handling

### CGRAM (Custom Characters)
- Maximum **8 custom characters** (addresses 0-7)
- ASCII codes 0-7 display custom characters
- Character data is **5×8 pixels** (only 5 bits used per byte)
- CGRAM is **volatile** - lost on power off

### Display Indexing
- **First row:** index `0` (not 1)
- **Second row:** index `1` (not 2)
- **First column:** index `0` (not 1)
- **Last column:** index `15` (not 16)

### Performance Considerations
1. **Command execution:** Minimum 37μs
2. **Clear display:** 1.52ms (longest command)
3. **4-bit mode:** Slower than 8-bit (two cycles per byte)
4. **8-bit mode:** Faster but uses more pins
5. **CGRAM writes:** Perform during initialization
6. **Backlight:** May significantly affect power consumption

### Mode Selection Guidelines

**Choose 4-Bit Mode when:**
- GPIO pins are limited
- Speed is not critical
- Simpler wiring is preferred
- Battery power efficiency matters

**Choose 8-Bit Mode when:**
- High-speed updates required
- Abundant GPIO available
- Maximum performance needed
- Reliability is critical

### Common Issues and Solutions

**Problem:** Display shows random characters
- **Solution:** Check contrast potentiometer adjustment
- **Solution:** Verify power supply voltage (typically 5V)

**Problem:** No display output
- **Solution:** Confirm `alcd_init()` is called first
- **Solution:** Check all pin connections

**Problem:** Garbled text
- **Solution:** Verify timing delays are adequate
- **Solution:** Check for correct mode (4-bit vs 8-bit)

**Problem:** Backlight not working
- **Solution:** Ensure `__alcd_useBL` is set to `true`
- **Solution:** Check transistor and current-limiting resistor

---

## Advanced Usage

### Performance Comparison

| Feature | 4-Bit Mode | 8-Bit Mode |
|---------|------------|------------|
| **Pins Required** | 6 (+1 backlight) | 10 (+1 backlight) |
| **Speed** | Slower (2 cycles/byte) | Faster (1 cycle/byte) |
| **Wiring Complexity** | Simple | Complex |
| **Best Use Case** | GPIO-limited projects | Performance-critical apps |
| **Power Consumption** | Lower | Higher |
| **Reliability** | Good | Excellent |

### Timing Specifications

The library implements precise timing according to HD44780 specifications:

| Operation | Time | Constant |
|-----------|------|----------|
| Standard Command | 50μs | `__alcd_delay_CMD` |
| Mode Setting | 5ms | `__alcd_delay_modeSet` |
| Power-On Delay | 50ms | `__alcd_delay_powerON` |
| Clear Display | 1.52ms | Special handling |

---

## Troubleshooting Guide

### Display Issues

**Symptom:** Blank screen (backlight on)
- Check contrast adjustment (turn potentiometer)
- Verify `alcd_init()` is called
- Check power supply voltage (4.5-5.5V)
- Verify EN pin toggling

**Symptom:** Display shows blocks/squares
- Incorrect initialization sequence
- Wrong mode selected (4-bit vs 8-bit)
- Timing issues (clock speed mismatch)
- Verify the system clock configuration in STM32CubeMX.

**Symptom:** Partial or corrupted characters
- Loose connections on data lines
- Incorrect pin definitions in alcd.h
- Ground connection issues
- Voltage fluctuations

**Symptom:** Characters appear in wrong positions
- DDRAM address calculation error
- Check `alcd_gotoxy()` parameters
- Row index confusion (0/1 vs 1/2)

### Backlight Issues

**Symptom:** Backlight won't turn on
- Check `__alcd_useBL` is set to `true`
- Verify transistor connection
- Check current-limiting resistor value
- Test transistor with multimeter

**Symptom:** Backlight flickers
- Insufficient current capacity
- Weak power supply
- Poor transistor selection
- Check ground connections

### Timing Issues

**Symptom:** Random characters after power-on
- Insufficient power-on delay
- Increase `__alcd_delay_powerON`
- Check power supply rise time

**Symptom:** Occasional missed characters
- Timing too fast for LCD
- Increase `__alcd_delay_CMD`
- Check crystal/oscillator accuracy (and ensure the correct source is selected in CubeMX)

---

## Migration Guide

### Migrating from 4-Bit to 8-Bit Mode

1. **Copy the correct source files:**
   ```
   Copy from: source/8-bit mode/
   Replace: alcd.h and alcd.c
   ```

2. **Update hardware connections:**
   - Connect DB0-DB3 pins
   - Update pin definitions in alcd.h

3. **Recompile and test:**
   - No code changes required
   - API remains identical

### Migrating from 8-Bit to 4-Bit Mode

1. **Copy the correct source files:**
   ```
   Copy from: source/4-bit mode/
   Replace: alcd.h and alcd.c
   ```

2. **Update hardware connections:**
   - Disconnect DB0-DB3 pins
   - Update pin definitions in alcd.h

3. **Recompile and test:**
   - No code changes required
   - Performance may be slower

---

## FAQ (Frequently Asked Questions)

**Q: Can I use this library with LCD displays larger than 16x2?**  
A: Currently, the library is optimized for 16×2 displays. For 20×4 or other sizes, you'll need to modify the `alcd_gotoxy()` function and DDRAM address calculations.

**Q: Why is my text scrolling automatically?**  
A: This happens when you write beyond column 15. The LCD has 40 bytes of DDRAM per line, but only 16 are visible. Use `alcd_gotoxy()` to position correctly.

**Q: Can I connect the LCD directly to 3.3V microcontrollers?**  
A: Most HD44780 LCDs require 5V. For 3.3V systems, use a level shifter or find a 3.3V-compatible LCD module.

**Q: How do I display numbers?**  
A: Use `sprintf()` or convert manually:
```c
#include <stdio.h>

int value = 123;
char buffer[16];
sprintf(buffer, "Value: %d", value);
alcd_puts(buffer);
```

**Q: Why does my custom character look wrong?**  
A: Ensure you're using only the lower 5 bits of each byte. Bits 5-7 are ignored. Use online generators for accuracy.

**Q: Can I read data from the LCD?**  
A: This library is write-only. Reading requires connecting the RW pin to a GPIO (currently tied to GND) and implementing read functions.

**Q: How do I create animated characters?**  
A: Update CGRAM data periodically:
```c
while(1)
{
    alcd_customChar(0, frame1);
    _delay_ms(200);
    alcd_customChar(0, frame2);
    _delay_ms(200);
}
```

**Q: What's the maximum update rate?**  
A: Approximately 1000 characters/second in 8-bit mode, 500 in 4-bit mode. Actual rate depends on MCU clock and delays.

**Q: Why doesn't my backlight control work?**  
A: Ensure:
1. Only available when the backlight pin is defined in your CubeMX configuration (e.g., __alcd_BL_GPIO_Port).
2. Transistor is correctly connected
3. Current-limiting resistor is appropriate
4. Pin definition matches hardware

---

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
