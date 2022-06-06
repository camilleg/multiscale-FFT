#!/bin/bash

# Does the output of ./stft still match the 2009 output,
# which matched MATLAB's output?

uname=$(uname -s 2>/dev/null)
if [[ "$uname" == "Darwin" ]]; then
  # On macOS, change all nan to -nan, to match Linux.
  if ! grep -q -e '-nan' stft_test_sinc.csv; then
    # Found no '-nan', thus we won't be changing '-nan' to '--nan'.
    sed -I '' 's/nan,/-nan,/g' stft_test_sinc.csv
  fi
# elif [[ "$uname" == "Linux" ]]; then
#   :
fi

# Use shasum instead of more familiar tools:
# - macOS's md5 differs in output format from linux's md5sum.
# - macOS's `more` differs, even with LESS_IS_MORE=1.
# - .tar files differ in file owner and timestamp.

a=$(shasum stft_test_*.csv | shasum - | sed 's/ .*//')
if [[ "$a" != "bdf6a1f46ab79888d1d43bb07eef23a99df21bbd" ]]; then
  echo "stft_test_*.csv is incorrect."
  exit 1
fi
