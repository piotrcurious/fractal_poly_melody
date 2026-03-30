
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
const float freqs[NUM_NOTES] = {440.00, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61};

// define an array of LFSR states
uint16_t lfsrs[NUM_NOTES];
// define an array of taps for each LFSR
uint16_t taps[NUM_NOTES];
// define an array of seeds for each LFSR
uint16_t seeds[NUM_NOTES];
// define an array of half-periods (microseconds) for each note
unsigned int half_periods[NUM_NOTES];

// Function prototypes
uint16_t freqToBinary(float freq);
void deinterleave(uint16_t binary, float *real, float *imag);
int complexToRoot(float real, float imag);
uint16_t findCyclotomic(int root);
void convertCyclotomic(uint16_t cyclotomic, uint16_t *seed, uint16_t *tap_bits);
uint16_t updateLFSR(uint16_t lfsr, uint16_t tap_bits);
void playNote(int index, int pin);
void playChord(int idx0, int idx1, int idx2, int pin0, int pin1, int pin2);

// Converts a frequency to a 16-bit fixed-point representation (e.g., Q10.6)
uint16_t freqToBinary(float freq) {
  // Use Q10.6 format: 10 bits for integer, 6 bits for fraction
  return (uint16_t)(freq * 64.0f);
}

// Deinterleave bits of the binary fraction to get the real and imaginary parts
// as coordinates in a complex plane.
void deinterleave(uint16_t binary, float *real, float *imag) {
  uint16_t r_bits = 0, i_bits = 0;
  for (int i = 0; i < 16; i++) {
    uint16_t bit = (binary >> i) & 1u;
    if (i % 2 == 0) r_bits |= (bit << (i / 2));
    else i_bits |= (bit << (i / 2));
  }
  // Map 8-bit results to range [-1.0, 1.0]
  *real = (float)r_bits / 128.0f - 1.0f;
  *imag = (float)i_bits / 128.0f - 1.0f;
}

// Maps the complex parts to the root index n where z = e^(2*pi*i*n/NUM_NOTES)
int complexToRoot(float real, float imag) {
  float angle = atan2(imag, real);
  if (angle < 0) angle += 2.0f * PI;
  int n = (int)(angle * (float)NUM_NOTES / (2.0f * PI) + 0.5f) % NUM_NOTES;
  return n;
}

// Determines the cyclotomic polynomial Φ_m(x) based on the order of the root index.
uint16_t findCyclotomic(int root) {
  auto gcd = [](int a, int b) {
    while (b) { a %= b; int t = a; a = b; b = t; }
    return a;
  };
  // root is the k-th root of unity (k=0..11)
  // Its order m in the group of 12th roots is m = 12 / gcd(k, 12)
  int m = 12 / gcd(root, 12);

  // Φ_m(x) mod 2 coefficients:
  // Φ1  = x + 1             -> 0x3   (11)
  // Φ2  = x + 1             -> 0x3   (11)
  // Φ3  = x^2 + x + 1       -> 0x7   (111)
  // Φ4  = x^2 + 1           -> 0x5   (101)
  // Φ6  = x^2 + x + 1       -> 0x7   (111)
  // Φ12 = x^4 + x^2 + 1     -> 0x15  (10101)
  switch(m) {
    case 1:  return 0x0003;
    case 2:  return 0x0003;
    case 3:  return 0x0007;
    case 4:  return 0x0005;
    case 6:  return 0x0007;
    case 12: return 0x0015;
    default: return 0x0015;
  }
}

// Converts the cyclotomic polynomial to LFSR taps and seed.
void convertCyclotomic(uint16_t cyclotomic, uint16_t *seed, uint16_t *tap_bits) {
  int d = 0;
  for (int i=15; i>=0; i--) {
    if ((cyclotomic >> i) & 1) {
      d = i;
      break;
    }
  }
  // Taps are the coefficients of the polynomial excluding the highest degree term
  *tap_bits = cyclotomic & ~(1u << d);
  // Seed should be non-zero
  *seed = (1u << d);
}

// Galois LFSR update logic
uint16_t updateLFSR(uint16_t lfsr, uint16_t tap_bits) {
  uint16_t out = lfsr & 1;
  lfsr >>= 1;
  if (out) lfsr ^= tap_bits;
  return lfsr;
}

void playNote(int index, int pin) {
  unsigned long startTime = millis();
  uint16_t state = lfsrs[index];
  uint16_t t = taps[index];
  unsigned int hp = half_periods[index];

  while (millis() - startTime < DURATION) {
    state = updateLFSR(state, t);
    if (state == 0) state = seeds[index];
    digitalWrite(pin, state & 1u);
    delayMicroseconds(hp);
  }
  lfsrs[index] = state;
}

void playChord(int idx0, int idx1, int idx2, int pin0, int pin1, int pin2) {
  unsigned long startTime = millis();
  uint16_t s[3] = {lfsrs[idx0], lfsrs[idx1], lfsrs[idx2]};
  uint16_t t[3] = {taps[idx0], taps[idx1], taps[idx2]};
  unsigned int hp[3] = {half_periods[idx0], half_periods[idx1], half_periods[idx2]};
  uint8_t pins[3] = {(uint8_t)pin0, (uint8_t)pin1, (uint8_t)pin2};

  unsigned long last_toggle[3];
  unsigned long now_us = micros();
  for(int i=0; i<3; i++) last_toggle[i] = now_us;

  int note_indices[3] = {idx0, idx1, idx2};

  while (millis() - startTime < DURATION) {
    now_us = micros();
    for(int i=0; i<3; i++) {
        if (now_us - last_toggle[i] >= hp[i]) {
            s[i] = updateLFSR(s[i], t[i]);
            if (s[i] == 0) s[i] = seeds[note_indices[i]];
            digitalWrite(pins[i], s[i] & 1u);
            last_toggle[i] = now_us;
        }
    }
    // Small delay for efficiency
    delayMicroseconds(5);
  }
  lfsrs[idx0] = s[0]; lfsrs[idx1] = s[1]; lfsrs[idx2] = s[2];
}

void setup() {
  pinMode(PIN_0, OUTPUT);
  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  Serial.begin(9600);
  
  for (int i = 0; i < NUM_NOTES; i++) {
    // 1. Freq to Binary
    uint16_t binary = freqToBinary(freqs[i]);
    // 2. Deinterleave to Complex
    float real, imag;
    deinterleave(binary, &real, &imag);
    // 3. Complex to Root Index
    int root = complexToRoot(real, imag);
    // 4. Root to Cyclotomic Polynomial
    uint16_t cyclotomic = findCyclotomic(root);
    // 5. Polynomial to LFSR
    convertCyclotomic(cyclotomic, &seeds[i], &taps[i]);
    lfsrs[i] = seeds[i];
    // 6. Pitch matching
    half_periods[i] = (unsigned int)(1000000.0f / (freqs[i] * 2.0f));
  }
}

void loop() {
  // Play an A Major chord (A4, C#5, E5) -> indices 0, 4, 7
  playChord(0, 4, 7, PIN_0, PIN_1, PIN_2);
  // Play each note of the chromatic scale
  for(int i=0; i<NUM_NOTES; i++) {
    playNote(i, PIN_0);
  }
}
