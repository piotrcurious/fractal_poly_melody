#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define LED_BUILTIN 13

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

typedef uint8_t byte;
typedef bool boolean;

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
unsigned long millis(void);
unsigned long micros(void);

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin);

long map(long x, long in_min, long in_max, long out_min, long out_max);

class Serial_ {
public:
    void begin(unsigned long baud) {}
    void print(const char* s) { printf("%s", s); }
    void print(int n) { printf("%d", n); }
    void print(unsigned int n) { printf("%u", n); }
    void print(long n) { printf("%ld", n); }
    void print(unsigned long n) { printf("%lu", n); }
    void print(double n) { printf("%f", n); }
    void println(const char* s) { printf("%s\n", s); }
    void println(int n) { printf("%d\n", n); }
    void println(unsigned int n) { printf("%u\n", n); }
    void println(long n) { printf("%ld\n", n); }
    void println(unsigned long n) { printf("%lu\n", n); }
    void println(double n) { printf("%f\n", n); }
    void println() { printf("\n"); }
};

extern Serial_ Serial;

#define PIN_A0 14
#define PIN_A1 15
#define PIN_A2 16
#define PIN_A3 17
#define PIN_A4 18
#define PIN_A5 19

#endif
