CC=g++
CFLAGS=-I mock_arduino/include -x c++
MOCK_SRC=mock_arduino/src/Arduino.cpp

all: test_chromatic_scale_synth test_jazz_converger test_single_jazz_chord_convergent test_simple_melody

test_chromatic_scale_synth: test_chromatic_scale_synth.cpp chromatic_scale_synth.ino $(MOCK_SRC)
	$(CC) $(CFLAGS) $^ -o $@

test_jazz_converger: test_jazz_converger.cpp jazz_converger.ino $(MOCK_SRC)
	$(CC) $(CFLAGS) $^ -o $@

test_single_jazz_chord_convergent: test_single_jazz_chord_convergent.cpp single_jazz_chord_convergent.ino $(MOCK_SRC)
	$(CC) $(CFLAGS) $^ -o $@

test_simple_melody: test_simple_melody.cpp simple_melody.ino $(MOCK_SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f test_chromatic_scale_synth test_jazz_converger test_single_jazz_chord_convergent test_simple_melody
