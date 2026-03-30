# Chromatic Polynomials and LFSR-Based Synthesis

This document describes the theory for generating arbitrary tones and chords from the chromatic scale using Linear Feedback Shift Registers (LFSRs) that represent polynomials.

## Theory

To find LFSRs that will generate tones or chords of the chromatic scale directly, we reverse the process of converting a note to a binary polynomial.

1. **Frequency to Binary**: Convert the frequency $ (in Hz) to its binary representation (e.g., 23.25 \approx 0b1000001011$).
2. **Deinterleaving**: Deinterleave the bits of the binary integer part to get the real and imaginary parts of a complex number  = a + bi$.
3. **Complex to Root**: Map $ to a root of unity  = e^{2\pi i k / n}$, where $ is the number of semitones (typically 12).
4. **Order of the Root**: Find the smallest $ such that $ is a primitive himBHsth root of unity (where  = 12/\gcd(k, 12)$).
5. **Cyclotomic Polynomial**: Find the cyclotomic polynomial $\Phi_m(x)$ that has $ as a root:
   1309\Phi_m(x) = \prod_{\substack{1 \leq k \leq m \ \gcd(k,m) = 1}} (x - e^{2\pi i k / m})1309
6. **LFSR Taps**: Convert $\Phi_m(x) \pmod{2}$ to LFSR taps. For example, $\Phi_{12}(x) = x^4 - x^2 + 1 = x^4 + x^2 + 1 \pmod{2}$ corresponds to a Galois LFSR with taps at ^2$ and ^0$.

## Implementation Examples (mod 2)

- $\Phi_1 = x+1 \rightarrow 0x3$
- $\Phi_2 = x+1 \rightarrow 0x3$
- $\Phi_3 = x^2+x+1 \rightarrow 0x7$
- $\Phi_4 = x^2+1 \rightarrow 0x5$
- $\Phi_6 = x^2+x+1 \rightarrow 0x7$
- $\Phi_{12} = x^4+x^2+1 \rightarrow 0x15$

## Chords

To generate chords, multiple LFSRs representing different notes are combined using bitwise operations (XOR/OR) or played simultaneously on separate pins.

## Pitch Accuracy

The pitch is matched by adjusting the clock rate of the LFSR updates. For a frequency $, the toggle interval should be $\frac{1}{2f}$ seconds.
