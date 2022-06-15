#!/bin/bash

# Does the output of ./stft still match the 2009 output,
# which matched MATLAB's output?

# Change any nan to -nan, to match the original build.
if ! grep -q -e '-nan' stft_test_sinc.csv; then
  # Found no '-nan', thus we won't be changing '-nan' to '--nan'.
  sed -i 's/nan,/-nan,/g' stft_test_sinc.csv
fi

# Use shasum instead of more familiar tools:
# - macOS's md5 differs in output format from linux's md5sum.
# - macOS's `more` differs, even with LESS_IS_MORE=1.
# - .tar files differ in file owner and timestamp.

a=$(shasum stft_test_*.csv | shasum - | sed 's/ .*//')
if [[ "$a" != "37f0d22b6a23c7c4f213145caf61d561907f9e56" ]]; then
  echo "stft_test_*.csv is incorrect."
  exit 1
fi
