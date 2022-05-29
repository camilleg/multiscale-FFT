CFLAGS = -O3 -Wall -Werror -pedantic -march=native -ffast-math
LDLIBS = -lm

all: stft

test: stft
	./stft
	./check.sh

clean:
	rm -f stft stft_test_*.csv

.PHONY: all clean test
