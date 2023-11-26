CFLAGS = -O3 -Wall -Werror -pedantic -march=native -ffast-math -std=c17
LDLIBS = -lm

all: stft

test: stft
	./stft
	./check.sh

figures: test
	./figures.rb fig2.png 3 stft_test_dual_impulse.csv
	./figures.rb fig3.png 3 stft_test_bandlimit.csv

clean:
	rm -f stft stft_test_*.csv fig2.png fig3.png

.PHONY: all clean figures test
