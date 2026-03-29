
#include "Arduino.h"

// define the pins for the output
#define PIN_0 8
#define PIN_1 9
#define PIN_2 10

// define the number of notes in the chromatic scale
#define NUM_NOTES 12

// define the duration of each note in milliseconds
#define DURATION 500

// define an array of frequencies for each note in the chromatic scale
// Starting from A4 = 440Hz
const int freqs[NUM_NOTES] = {440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831};

// define an array of LFSRs for each note in the chromatic scale
uint16_t lfsrs[NUM_NOTES];

// define an array of taps for each LFSR in the chromatic scale
uint16_t taps[NUM_NOTES];

// Function prototypes
uint16_t freqToBinary(int freq);
void deinterleave(uint16_t binary, uint16_t *real, uint16_t *imag);
int complexToRoot(uint16_t real, uint16_t imag);
uint16_t findCyclotomic(int root);
void convertCyclotomic(uint16_t cyclotomic, uint16_t *lfsr, uint16_t *taps);
uint16_t updateLFSR(uint16_t lfsr, uint16_t taps);
void playNote(uint16_t *lfsr, uint16_t taps, int pin);
void playChord(uint16_t *lfsr_0, uint16_t *lfsr_1, uint16_t *lfsr_2, uint16_t taps_0, uint16_t taps_1, uint16_t taps_2, int pin_0, int pin_1, int pin_2);

// Converts a frequency to a binary fraction representation
uint16_t freqToBinary(int freq) {
  uint16_t binary = 0;
  // Based on the theory in chromatic_polynomials.md, we want to represent
  // the frequency as a binary fraction.
  // Suppose freq = 523.25. 523.25 = 0b1000001011.01...
  // For the sake of a 16-bit representation, we'll store the significant bits.
  binary = (uint16_t)freq;
  return binary;
}

// Deinterleave bits to get real and imaginary parts of a complex number
void deinterleave(uint16_t binary, uint16_t *real, uint16_t *imag) {
  *real = 0;
  *imag = 0;
  for (int i = 0; i < 16; i++) {
    uint16_t bit = (binary >> i) & 1u;
    if (i % 2 == 0) {
      *real |= (bit << (i / 2));
    } else {
      *imag |= (bit << (i / 2));
    }
  }
}

// Convert complex number to root of unity index
int complexToRoot(uint16_t real, uint16_t imag) {
  // Map back to a root index n where z = e^(2*pi*i*n/12)
  // This is a simplified mapping based on the angle
  float angle = atan2((float)imag, (float)real);
  if (angle < 0) angle += 2 * PI;
  int n = (int)(angle * NUM_NOTES / (2 * PI) + 0.5) % NUM_NOTES;
  return n;
}

// Cyclotomic polynomials for n-th roots of unity (modulo 2)
// Φ1(x) = x + 1 (0b11 = 0x3)
// Φ2(x) = x + 1 (0b11 = 0x3)
// Φ3(x) = x^2 + x + 1 (0b111 = 0x7)
// Φ4(x) = x^2 + 1 (0b101 = 0x5)
// Φ6(x) = x^2 - x + 1 = x^2 + x + 1 (mod 2) (0b111 = 0x7)
// Φ12(x) = x^4 - x^2 + 1 = x^4 + x^2 + 1 (mod 2) (0b10101 = 0x15)
uint16_t findCyclotomic(int root) {
  // The MD file says we need Φ_n(x) where the root is a primitive n-th root of unity.
  // For each of our 12 notes (n=0..11), we check the order of the root in the group of 12th roots.

  // order of root k in Z_12 is 12 / gcd(k, 12)
  auto gcd = [](int a, int b) {
    while (b) { a %= b; int t = a; a = b; b = t; }
    return a;
  };

  int order = 12 / gcd(root, 12);

  switch(order) {
    case 1:  return 0x3;  // Φ1
    case 2:  return 0x3;  // Φ2
    case 3:  return 0x7;  // Φ3
    case 4:  return 0x5;  // Φ4
    case 6:  return 0x7;  // Φ6
    case 12: return 0x15; // Φ12
    default: return 0x15;
  }
}

void convertCyclotomic(uint16_t cyclotomic, uint16_t *lfsr, uint16_t *taps) {
  *lfsr = 0xACE1;
  *taps = cyclotomic;
}

uint16_t updateLFSR(uint16_t lfsr, uint16_t taps) {
  uint16_t bit = lfsr & taps;
  uint16_t p = 0;
  for(int i=0; i<16; i++) {
    if ((bit >> i) & 1) p = !p;
  }
  return (lfsr >> 1) | (p << 15);
}

void playNote(uint16_t *lfsr, uint16_t taps, int pin) {
  unsigned long startTime = millis();
  while (millis() - startTime < DURATION) {
    *lfsr = updateLFSR(*lfsr, taps);
    digitalWrite(pin, *lfsr & 1u);
    delayMicroseconds(100);
  }
}

void playChord(uint16_t *lfsr_0, uint16_t *lfsr_1, uint16_t *lfsr_2, uint16_t taps_0, uint16_t taps_1, uint16_t taps_2, int pin_0, int pin_1, int pin_2) {
  unsigned long startTime = millis();
  while (millis() - startTime < DURATION) {
    *lfsr_0 = updateLFSR(*lfsr_0, taps_0);
    *lfsr_1 = updateLFSR(*lfsr_1, taps_1);
    *lfsr_2 = updateLFSR(*lfsr_2, taps_2);
    digitalWrite(pin_0, *lfsr_0 & 1u);
    digitalWrite(pin_1, *lfsr_1 & 1u);
    digitalWrite(pin_2, *lfsr_2 & 1u);
    delayMicroseconds(100);
  }
}

void setup() {
  pinMode(PIN_0, OUTPUT);
  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  Serial.begin(9600);
  
  for (int i = 0; i < NUM_NOTES; i++) {
    uint16_t binary = freqToBinary(freqs[i]);
    uint16_t real, imag;
    deinterleave(binary, &real, &imag);
    int root = complexToRoot(real, imag);
    uint16_t cyclotomic = findCyclotomic(root);
    convertCyclotomic(cyclotomic, &lfsrs[i], &taps[i]);
  }
}

void loop() {
  // A Major: A, C#, E -> index 0, 4, 7
  playChord(&lfsrs[0], &lfsrs[4], &lfsrs[7], taps[0], taps[4], taps[7], PIN_0, PIN_1, PIN_2);
  for(int i=0; i<NUM_NOTES; i++) {
    playNote(&lfsrs[i], taps[i], PIN_0);
  }
}
