#include "Arduino.h"
void setup();
void loop();
extern int digitalRead(uint8_t pin);
static int current_btn = -1;
int digitalRead(uint8_t pin) {
    if (pin == 2 && current_btn == 0) return LOW;
    if (pin == 3 && current_btn == 1) return LOW;
    if (pin == 4 && current_btn == 2) return LOW;
    return HIGH;
}
int main() {
    setup();
    for(int b = 0; b < 3; b++) {
        current_btn = b;
        loop();
    }
    return 0;
}
