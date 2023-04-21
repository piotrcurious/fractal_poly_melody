
// Define a variable x as 2^(1/12), which is the ratio of a semitone
const float x = pow(2, 1.0/12);

// Define a function that takes an integer n as input and returns the frequency of the nth note in the chromatic scale
// The function uses the polynomial base x^k, where k is any non-negative integer
// The function assumes that A4 is the reference note with coefficient 1 and exponent 0
float chromaticScale(int n) {
  // Calculate the coefficient and exponent of the polynomial term for the nth note
  int coefficient = 1;
  int exponent = n;
  // If n is negative, adjust the coefficient and exponent to make them positive
  if (n < 0) {
    coefficient = pow(2, ceil(-n / 12.0));
    exponent = n + 12 * ceil(-n / 12.0);
  }
  // If n is greater than or equal to 12, adjust the coefficient and exponent to make them less than 12
  if (n >= 12) {
    coefficient = pow(2, floor(n / 12.0));
    exponent = n - 12 * floor(n / 12.0);
  }
  // Calculate and return the frequency of the nth note by multiplying the coefficient by x raised to the exponent
  float frequency = coefficient * pow(x, exponent);
  return frequency;
}
