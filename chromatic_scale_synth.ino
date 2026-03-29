
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

// define a function that converts a frequency to a binary fraction
uint16_t freqToBinary(int freq) {
  uint16_t binary = 0;
  int temp_freq = freq;
  for (int i = 0; i < 16; i++) {
    temp_freq *= 2;
    if (temp_freq >= 1000) {
      binary |= (1u << i);
      temp_freq -= 1000;
    }
  }
  return binary;
}

// deinterleave bits
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

// complex to root
int complexToRoot(uint16_t real, uint16_t imag) {
  int best_n = 0;
  float min_dist = 1e9;
  
  // Scale back to [-1, 1] range approximately
  float r = (float)real / 255.0 * 2.0 - 1.0;
  float im = (float)imag / 255.0 * 2.0 - 1.0;

  for (int n = 0; n < NUM_NOTES; n++) {
    float ex_r = cos(2.0 * PI * n / NUM_NOTES);
    float ex_im = sin(2.0 * PI * n / NUM_NOTES);
    float dist = pow(ex_r - r, 2) + pow(ex_im - im, 2);
    if (dist < min_dist) {
      min_dist = dist;
      best_n = n;
    }
  }
  return best_n;
}

// cyclotomic polynomial for nth root of unity
// Simplified: for n=12, Φ12(x) = x^4 - x^2 + 1
// We can use a lookup table or hardcode for 12 notes if needed,
// but let's try to keep the logic.
uint16_t findCyclotomic(int root) {
  // In a real implementation we'd compute Φ_n(x).
  // For the sake of this synth, we use the root to select a polynomial.
  // Primitive roots of unity for n=12 have Φ12(x).
  // This is a placeholder for the logic described in the MD file.
  if (root % 12 == 0) return 0x1F; // x^4 + x^3 + x^2 + x + 1 (Φ5 approx)
  if (root % 6 == 0) return 0x7;  // x^2 + x + 1 (Φ3)
  return 0x15; // x^4 - x^2 + 1 -> 10101 in binary -> 0x15 (Φ12)
}

void convertCyclotomic(uint16_t cyclotomic, uint16_t *lfsr, uint16_t *taps) {
  *lfsr = 0xACE1;
  *taps = cyclotomic;
}

uint16_t updateLFSR(uint16_t lfsr, uint16_t taps) {
  uint16_t bit = lfsr & taps;
  // Parity of bit
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
    delayMicroseconds(100); // Slower for audible range in simulation
  }
}

void playChord(uint16_t *lfsr_0, uint16_t *lfsr_1, uint16_t *lfsr_2, uint16_t taps_0, uint16_t taps_1, uint16_t taps_2, int pin_0, int pin_1, int pin_2) {
  unsigned long startTime = millis();
  while (millis() - startTime < DURATION) {
    *lfsr_0 = updateLFSR(*lfsr_0, taps_0);
    *lfsr_1 = updateLFSR(*lfsr_1, taps_1);
    *lfsr_2 = updateLFSR(*lfsr_2, taps_2);
    uint8_t bit = (*lfsr_0 & 1u) ^ (*lfsr_1 & 1u) ^ (*lfsr_2 & 1u);
    digitalWrite(pin_0, bit);
    digitalWrite(pin_1, bit);
    digitalWrite(pin_2, bit);
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
  // A Major: A, C#, E -> 0, 4, 7
  playChord(&lfsrs[0], &lfsrs[4], &lfsrs[7], taps[0], taps[4], taps[7], PIN_0, PIN_1, PIN_2);
  for(int i=0; i<NUM_NOTES; i++) {
    playNote(&lfsrs[i], taps[i], PIN_0);
  }
}
