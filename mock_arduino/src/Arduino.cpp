#include "Arduino.h"
#include <chrono>
#include <thread>

Serial_ Serial;

auto start_time = std::chrono::steady_clock::now();

__attribute__((weak)) void pinMode(uint8_t pin, uint8_t mode) {}
__attribute__((weak)) void digitalWrite(uint8_t pin, uint8_t val) {}
__attribute__((weak)) int digitalRead(uint8_t pin) { return HIGH; }
__attribute__((weak)) int analogRead(uint8_t pin) { return 512; }
__attribute__((weak)) void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
__attribute__((weak)) void delayMicroseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
__attribute__((weak)) unsigned long millis(void) {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}
__attribute__((weak)) unsigned long micros(void) {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
}

__attribute__((weak)) void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {}
__attribute__((weak)) void noTone(uint8_t _pin) {}

__attribute__((weak)) long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
