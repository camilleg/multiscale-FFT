# Efficient simultaneous multi-scale computation of FFTs

An efficient algorithm for simultaneously computing FFTs at
multiple window sizes is introduced. It requires 42% fewer
complex multiplies for an 8192-sample window. It is motivated
by STFT visualization of non-speech audio events,
which can vary widely in scale and resolution of both time
and frequency. A reference implementation in C agrees with
MATLAB's implementation, for numerous test inputs.

Published in 2009 as a
[technical report](https://fodava.gatech.edu/visual-data-analytics-technical-reports-2009?page=3)
of the [NSF](https://nsf.gov/awardsearch/showAward?AWD_ID=0807329) program
[Foundations of Data and Visual Analytics](https://fodava.gatech.edu/about-us).

## To cite

Dave Cohen, Camille Goudeseune, and Mark Hasegawa-Johnson.  2009.  
"Efficient simultaneous multi-scale computation of FFTs."  
*Technical report GT-FODAVA-09-01.*
