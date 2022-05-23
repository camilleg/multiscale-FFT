CFLAGS = -O3 -Wall -Werror -pedantic
LDLIBS = -lm

all: stft

test: stft
	./stft
	./check.sh

clean:
	rm -f stft stft_test_*.csv

.PHONY: all clean test
