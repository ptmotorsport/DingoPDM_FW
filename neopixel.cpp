#include "neopixel.h"
#include "port.h"
#include <string.h>

NeoPixel::NeoPixel(PWMDriver* pwmDriver, uint8_t channel)
    : m_pwm(pwmDriver), m_channel(channel), m_brightness(255)
{
    memset(m_buffer, 0, sizeof(m_buffer));
    memset(m_pixels, 0, sizeof(m_pixels));
    
    // Determine GPIO pin based on PWM driver and channel
    if (pwmDriver == &PWMD1 && channel == 0) {
        m_pin = PAL_LINE(GPIOA, 8U);  // PA8 - TIM1_CH1
    } else if (pwmDriver == &PWMD3 && channel == 2) {
        m_pin = PAL_LINE(GPIOB, 10U); // PB10 - TIM3_CH3
    } else {
        m_pin = 0; // Invalid
    }
}

void NeoPixel::Init() {
    // Configure pin as output
    if (m_pin != 0) {
        palSetLineMode(m_pin, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
        palClearLine(m_pin);
    }
    
    // Set brightness to 10%
    SetBrightness(26);
    
    // Clear the strip
    Clear();
    Show();
}

void NeoPixel::SetPixel(uint8_t index, NeoPixelColor color) {
    if (index >= NEOPIXEL_NUM_LEDS) return;
    m_pixels[index] = color;
}

void NeoPixel::SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    SetPixel(index, NeoPixelColor(r, g, b));
}

NeoPixelColor NeoPixel::GetPixel(uint8_t index) {
    if (index >= NEOPIXEL_NUM_LEDS) return NeoPixelColor::Off();
    return m_pixels[index];
}

void NeoPixel::Clear() {
    for (uint8_t i = 0; i < NEOPIXEL_NUM_LEDS; i++) {
        m_pixels[i] = NeoPixelColor::Off();
    }
}

void NeoPixel::SetBrightness(uint8_t brightness) {
    m_brightness = brightness;
}

uint8_t NeoPixel::ApplyBrightness(uint8_t value) {
    return (uint8_t)(((uint16_t)value * m_brightness) >> 8);
}

void NeoPixel::SendByte(uint8_t byte) {
    // Send 8 bits, MSB first
    // WS2812B-2020 timing at 180MHz (5.56ns per cycle):
    // T0H = 220-380ns (~54 cycles), T0L = 580-1us (~144 cycles)
    // T1H = 580-1us (~126 cycles), T1L = 580-1us (~144 cycles)
    // palSetLine/palClearLine overhead: ~5 cycles each
    
    for (int8_t bit = 7; bit >= 0; bit--) {
        if (byte & (1 << bit)) {
            // Send '1' bit: ~700ns high, ~800ns low
            palSetLine(m_pin);
            // 126 cycles - 5 = 121 cycles = 15 x 8 NOPs + 1
            __asm__ volatile(
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n"
            );
            palClearLine(m_pin);
            // 144 cycles - 5 = 139 cycles = 17 x 8 NOPs + 3
            __asm__ volatile(
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n"
            );
        } else {
            // Send '0' bit: ~300ns high, ~800ns low
            palSetLine(m_pin);
            // 54 cycles - 5 = 49 cycles = 6 x 8 NOPs + 1
            __asm__ volatile(
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n"
            );
            palClearLine(m_pin);
            // 144 cycles - 5 = 139 cycles = 17 x 8 NOPs + 3
            __asm__ volatile(
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                "nop\n nop\n nop\n"
            );
        }
    }
}

void NeoPixel::ConvertPixelsToPWM() {
    uint32_t pos = 0;
    
    // Convert each pixel to PWM data
    for (uint8_t i = 0; i < NEOPIXEL_NUM_LEDS; i++) {
        // Apply brightness
        uint8_t g = ApplyBrightness(m_pixels[i].g);
        uint8_t r = ApplyBrightness(m_pixels[i].r);
        uint8_t b = ApplyBrightness(m_pixels[i].b);
        
        // WS2812B order is GRB
        uint32_t color = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
        
        // Convert 24 bits to PWM values (MSB first)
        for (int8_t bit = 23; bit >= 0; bit--) {
            m_buffer[pos++] = (color & (1 << bit)) ? NEOPIXEL_BIT_1 : NEOPIXEL_BIT_0;
        }
    }
    
    // Add reset pulse (low for >50us)
    for (uint32_t i = 0; i < NEOPIXEL_RESET_SLOTS; i++) {
        m_buffer[pos++] = 0;
    }
}

void NeoPixel::Show() {
    if (m_pin == 0) return;  // Invalid pin
    
    // Disable interrupts for precise timing
    chSysLock();
    
    // Send data for each LED
    for (uint8_t i = 0; i < NEOPIXEL_NUM_LEDS; i++) {
        // Apply brightness
        uint8_t g = ApplyBrightness(m_pixels[i].g);
        uint8_t r = ApplyBrightness(m_pixels[i].r);
        uint8_t b = ApplyBrightness(m_pixels[i].b);
        
        // WS2812B order is GRB
        SendByte(g);
        SendByte(r);
        SendByte(b);
    }
    
    // Re-enable interrupts
    chSysUnlock();
    
    // Reset pulse (>50us low)
    palClearLine(m_pin);
    chThdSleepMicroseconds(60);
}
