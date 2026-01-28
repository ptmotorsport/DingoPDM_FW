#pragma once

#include "hal.h"

#define NEOPIXEL_NUM_LEDS 8
#define NEOPIXEL_RESET_SLOTS 50

// NeoPixel timing for WS2812B at 800kHz (1.25us period)
// Using PWM frequency that gives clean bit timing
#define NEOPIXEL_PWM_FREQUENCY 800000  // 800kHz
#define NEOPIXEL_PWM_PERIOD 100        // Gives 1.25us per bit

// Bit timings as PWM duty cycles
#define NEOPIXEL_BIT_0 32   // ~0.4us high (32% of 1.25us)
#define NEOPIXEL_BIT_1 64   // ~0.8us high (64% of 1.25us)

struct NeoPixelColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
    NeoPixelColor() : r(0), g(0), b(0) {}
    NeoPixelColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    
    // Predefined colors
    static NeoPixelColor Red()    { return NeoPixelColor(255, 0, 0); }
    static NeoPixelColor Green()  { return NeoPixelColor(0, 255, 0); }
    static NeoPixelColor Blue()   { return NeoPixelColor(0, 0, 255); }
    static NeoPixelColor White()  { return NeoPixelColor(255, 255, 255); }
    static NeoPixelColor Amber()  { return NeoPixelColor(255, 191, 0); }
    static NeoPixelColor Off()    { return NeoPixelColor(0, 0, 0); }
};

class NeoPixel {
public:
    NeoPixel(PWMDriver* pwmDriver, uint8_t channel);
    
    void Init();
    void SetPixel(uint8_t index, NeoPixelColor color);
    void SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void Clear();
    void Show();
    void SetBrightness(uint8_t brightness); // 0-255
    
    NeoPixelColor GetPixel(uint8_t index);
    
private:
    PWMDriver* m_pwm;
    uint8_t m_channel;
    uint8_t m_brightness;
    ioline_t m_pin;  // GPIO line for bit-banging
    
    // Buffer for PWM DMA data (not currently used - using bit-bang instead)
    uint16_t m_buffer[(NEOPIXEL_NUM_LEDS * 24) + NEOPIXEL_RESET_SLOTS];
    
    // Current LED colors
    NeoPixelColor m_pixels[NEOPIXEL_NUM_LEDS];
    
    void ConvertPixelsToPWM();
    uint8_t ApplyBrightness(uint8_t value);
    void SendByte(uint8_t byte);
};

// Status LED definitions for the NeoPixel strip
enum class NeoPixelLED : uint8_t {
    Power = 0,      // LED1: Green when on
    Temp = 1,       // LED2: Green <90°C, Red >90°C
    CAN = 2,        // LED3: White when OK
    Error = 3,      // LED4: Red when error
    Output1 = 4,    // LED5: Output monitor
    Output2 = 5,    // LED6: Output monitor
    Output3 = 6,    // LED7: Output monitor
    Output4 = 7     // LED8: Output monitor
};
