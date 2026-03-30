# Chromatic Polynomials and LFSR-Based Synthesis

This document describes the theory for generating arbitrary tones and chords from the chromatic scale using Linear Feedback Shift Registers (LFSRs) that represent polynomials.

## Theory

To find LFSRs that will generate tones or chords of the chromatic scale directly, we reverse the process of converting a note to a binary polynomial.

1. **Frequency to Binary**: Convert the frequency f (in Hz) to a fixed-point binary representation (e.g., Q10.6).
2. **Deinterleaving**: Deinterleave the bits of the binary integer part to get the real and imaginary parts of a complex number z = a + bi. This maps the frequency into a complex coordinate.
3. **Complex to Root**: Map the complex coordinate (a, b) to a root of unity index k using the angle: theta = atan2(b, a). The index k is (theta * 12 / (2 * PI) + 0.5) mod 12.
4. **Order of the Root**: Determine the order m of the k-th root of unity in the group of 12th roots: m = 12/gcd(k, 12).
5. **Cyclotomic Polynomial**: Select the cyclotomic polynomial Phi_m(x) where the root is a primitive m-th root of unity.
6. **LFSR Taps**: Convert Phi_m(x) (mod 2) to LFSR taps for a Galois LFSR implementation.

## Cyclotomic Polynomials (mod 2)

- m=1, 2: Phi_1, Phi_2 = x+1 -> 0x3
- m=3, 6: Phi_3, Phi_6 = x^2+x+1 -> 0x7
- m=4: Phi_4 = x^2+1 -> 0x5
- m=12: Phi_12 = x^4+x^2+1 -> 0x15

## Fractal Arpeggios

In addition to direct synthesis, the sketches implement fractal arpeggios using the Mandelbrot-style iteration:
z(n+1) = z(n)^2 + c
where z(n) is the frequency and c is a constant derived from a bit-flipped LFSR polynomial. This creates self-similar melodic sequences that can be converged toward target musical structures (like jazz chords) using optimization algorithms.

## Implementation Notes

- **Mock Environment**: A C++ mock environment is provided to test the sketches without Arduino hardware.
- **Polyphony**: Chords are implemented by tracking individual microsecond toggle times for each note's LFSR, allowing multiple frequencies to be output simultaneously.
