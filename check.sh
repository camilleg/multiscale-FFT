#!/bin/bash

# Does the output of ./stft still match the 2009 output,
# which matched MATLAB's output?

a=$(more stft_test_*.csv | md5sum)
if [[ "$a" != "edf43a9fc57bba7beb850733fb58b9d1  -" ]]; then
  echo "stft_test_*.csv is incorrect."
  exit 1
fi
