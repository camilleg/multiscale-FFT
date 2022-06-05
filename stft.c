#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Reverse a 32-bit value, which must be odd.
// Divide the bits into blocks of size b=1.
// Swap adjacent blocks.
// Double b and repeat, until b reaches half of the word size.
unsigned reverse(unsigned x)
{
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
  x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
  x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
  x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
  x = (( x              >> 16) | ( x               << 16));
  while (x % 2 == 0)
    x >>= 1;
  return x;
}

double complex ce(int n, int N)
{
  return cexp(I * ((-M_PI*2*n) / N));
}

// Power spectral density, i.e., signal power distributed over frequency.
double PSD(double complex x)
{
  return creal(x)*creal(x) + cimag(x)*cimag(x);
}

int testMyMath()
{
  for (int i=1; i<=9999999; i+=2) {
    if (reverse(reverse(i)) != i) {
      printf("bit-reverse failed for i=%d\n", i);
      return 0;
    }
  }
  return 1;
}

float* buf = NULL;
double complex* t = NULL;
double complex* X = NULL;
int numwin = -1;

// C lacks C++'s constexpr.
#define poweroftwo (11)
#define N1 (1 << poweroftwo)

// Stuff buf, only one window long (N1).

void teststftZero()
{
  for (int i=0; i<N1; ++i)
    buf[i] = 0;
}

void teststftAlternating()
{
  for (int i=0; i<N1; ++i)
    buf[i] = (i % 2 == 0) ? 1 : -1;
}

void teststftBandlimit()
{
  teststftZero();
  for (int i=0; i<N1; ++i)
    for (int b = (M_PI/8)*1000; b < (M_PI/4)*1000; ++b)
      buf[i] += sin((b/1000.0)*i);
}

void teststftSinc()
{
  for (int i=0; i<N1; ++i)
    buf[i] = sin((M_PI/8)*i) / ((M_PI/8)*i);
}

void teststftImpulse()
{
  teststftZero();
  buf[0] = 100; // why changed to 10 ? ;;
}

void teststftDualImp()
{
  teststftZero();
  buf[0] = 10;
  buf[356] = 50; // why changed to buf[1780] = 50 ? ;;
}

void teststftShiftImp()
{
  teststftZero();
  buf[500] = 100;
}

void teststftSingleSine()
{
  for (int i=0; i<N1; ++i)
    buf[i] = cos((M_PI/4)*i);
}

void teststftDualSine()
{
  for (int i=0; i<N1; ++i)
    buf[i] = 5*cos((M_PI/3)*i) + cos((M_PI/16)*i);
}

void teststftConst()
{
  for (int i=0; i<N1; ++i)
    buf[i] = 10;
}

// int numbytes = 319999934;
// not more than the file length, 319999934
const int numbytes = 1<<12;

#if 0
void testcase() // this goes away
{
  
  // This is more than one window long, of course!
  
  FILE* pf = fopen("cmi_8bit_8kHz.raw", "rb");
  int numbytesRead = fread(buf, 1, numbytes, pf);
  if (numbytesRead != numbytes) {
    printf("Read only %d of %d bytes of audio data.\n",
      numbytesRead, numbytes);
    numbytes = numbytesRead;
  } else {
    printf("Read %d bytes of audio data.\n", numbytes);
  }
  fclose(pf);
  
  /*printf("HACK: noised input data.\n\n\n\n\n");
  for (i=0; i<N1; ++i)
    buf[i] = (float)rand() / RAND_MAX;*/ // from 0 inclusive to 1 exclusive.

}
#endif

const double complex bogus = 999.9876 + 999.9876 * I;
const long itMax = (poweroftwo-1) * poweroftwo * N1/2;
const long iXMax = poweroftwo * N1;

int init()
{
  const size_t C = sizeof(double complex);
  printf("Array t will be %.1f MB; X, %.1f MB.\n", C*itMax/1e6, C*iXMax/1e6);
  t = malloc(itMax * C);
  X = malloc(iXMax * C);
  buf = malloc(N1*sizeof(float));
  if (!buf || !t || !X) {
    printf("out of memory.\n");
    return 0;
  }
  for (int i=0; i<itMax; ++i)
    t[i] = bogus;

  numwin = floor(numbytes/N1);
  // printf("The data file has %d windows.\n", numwin);
  return 1;
}

#define getT(i, j, k) (t[(i)*poweroftwo*N1/2 + (j)*2 + (k)])

double complex getX(int i, int j)
{
  return X[i*N1 + j];
}
void setX(int i, int j, double complex c)
{
  X[i*N1 + j] = c;
}


void computeTemp1(const int offset)
{
  const float* window = buf + offset;
  const int Smax = poweroftwo*N1/4;
  int Bsize = N1/2;
  int Ssize = Smax;
  for (int iBsize = 0; iBsize < poweroftwo-1; ++iBsize, Bsize/=2, Ssize/=2) {
    int iTstep = 0;
    int iTstep1 = 0;
    for (; iTstep1 < Smax; iTstep1 += Ssize, iTstep += Bsize*2) {
      int iTnum = 0;
      for (int iTnum1 = 0; iTnum1 < Bsize/2; ++iTnum1) {
        for (int iTnum2 = 0; iTnum2 < 2; ++iTnum2, ++iTnum) {
          getT(iBsize, iTstep1 + iTnum1, iTnum2) =
            (window[iTnum + iTstep] - window[iTnum + iTstep + Bsize]) *
              ce(iTnum, Bsize*2);
        }
      }
    }
  }
}

void computeSize2DFTs(const int offset)
{
  const float* w = buf + offset; // window
  for (int iWnum = 0; iWnum < N1; iWnum+=2) {
    // Actually (w +- w) * ce(0,2)).
    setX(0, iWnum  , w[iWnum] + w[iWnum+1]);
    setX(0, iWnum+1, w[iWnum] - w[iWnum+1]);
  }
}

// Middle branches.
void computeMiddle()
{
  int Bsize = N1/2;
  int c = 1;
  int Smax = poweroftwo*N1/4;
  int Ssize1 = Smax;
  int pot = poweroftwo-1;
  int iBsize;
  for (iBsize = 0; iBsize < poweroftwo-1; ++iBsize,Bsize/=2,--pot,c*=2,Ssize1/=2) {
    int Bnum;
    for (Bnum = 0; Bnum < c; ++Bnum) {
      int Ssize=Bsize;
      int count1 = 0;
      int step;
      for (step = 0; step < pot; ++step,Ssize/=2) {
	const int Ssize2 = Ssize/2;
        const int iTstep = Bnum*Ssize1;
        int count = 0;
        for (int i = 0; i < Bsize; i+=2,++count1) {
          const int iTnum1 = iTstep + count1;
          for (int iTnum = 0; iTnum < 2; ++iTnum,++count) {
	    // This block spends most of the compute time.
	    double complex* const tt = &getT(iBsize, iTnum1, iTnum);
	    tt[Bsize] = (Ssize > 2) ?
              ((count % Ssize < Ssize2) ?
                tt[Ssize2] + tt[0] :
                (tt[-Ssize2] - tt[0]) * ce(count%Ssize - Ssize2, Ssize)
	      )
            : ((iTnum % 2 < 1) ?
		tt[0] + tt[1] :
		(tt[-1]      - tt[0]) * ce(count%Ssize - Ssize2, Ssize)
	      );
          }
        }
      }
    }
  }
}

// Odd-numbered DFT values.
void computeOdd()
{
  int d = N1/4;
  int iBsize = poweroftwo-2;
  int Smin = poweroftwo;
  int Bsize = 4;
  int pot = 1;
  for (int iWsize = 1; iWsize < poweroftwo; ++iWsize,d/=2,--iBsize,Bsize*=2,Smin*=2,++pot) {
    int Tstep = pot * Bsize/4;
    for (int iWnum = 0; iWnum < d; ++iWnum, Tstep+=Smin) {
      int count = 0;
      int iTnum = Bsize/2;
      for (int iTnum1 = 0; iTnum1 < Bsize/2; iTnum1+=2, ++count) {
        for (int iTnum2 = 0; iTnum2 < 2; ++iTnum2, ++iTnum) {
          setX(iWsize, reverse(iTnum) + (iWnum * Bsize),
            getT(iBsize, Tstep + count, iTnum2));
        }
      }
    }
  }
}

// Even-numbered DFT values.
void computeEven()
{
  const int iWsizeMax = poweroftwo;
  int iSnumMax = 2; // aka window size
  for (int iWsize = 0; iWsize < iWsizeMax-1; ++iWsize,iSnumMax*=2) {
    const int iWnumMax = N1/(iSnumMax*2);
    for (int iWnum = 0; iWnum < iWnumMax; ++iWnum) {
      for (int iSnum = 0; iSnum < iSnumMax; ++iSnum) {
        setX(iWsize+1, iSnum*2 + iSnumMax*(iWnum*2),
          getX(iWsize, iSnum   + iSnumMax*(iWnum*2    )) +
          getX(iWsize, iSnum   + iSnumMax*(iWnum*2 + 1)));
      }
    }
  }
}

void computeNestedWindows(const int offset)
{
  computeTemp1(offset);
  computeSize2DFTs(offset);
  computeMiddle();
  computeOdd();
  computeEven();
}

void testSTFT(const char* filename, void testfunction())
{
  FILE* file = fopen(filename, "w");
  if (!file) {
    printf("Failed to create file '%s'.\n", filename);
    return;
  }
  teststftZero();
  testfunction();

  int i,j;
#if 0
  for (i=0; i<poweroftwo; ++i)
    for (j=0; j<N1; ++j)
      setX(i, j, 999.999); // Output should not have any of these.
#endif

  computeNestedWindows(0);
  int h = 2;
  for (i = 0; i < poweroftwo; ++i, h*=2) {
    fprintf(file,"\"w. 2^%d=%d.\",", i+1, h);
  }
  fprintf(file,"\n");

  for (j=0; j<N1; ++j) {
    for (i=0; i<poweroftwo; ++i) {
      setX(i, j, PSD(getX(i, j)));
      fprintf(file, "%f,", creal(getX(i, j)));
    }
    fprintf(file,"\n");
  }
}

void timeSTFT()
{
  teststftZero();
  const int iMax = 10;
  struct timeval t0, t;
  gettimeofday(&t0, 0); 

  for (int i=0; i<iMax; ++i)
    computeNestedWindows(0);

  gettimeofday(&t, 0);
  printf("Mean %.4f usec\n", ((t.tv_sec-t0.tv_sec)*1e6 + (t.tv_usec-t0.tv_usec))/iMax);
}

#if 0
int run()
{
  const char* filename = "stft_test.csv";
  FILE* file = fopen(filename, "w");
  if (!file) {
    printf("failed to create test-output file '%s'.\n", filename);
    return 1;
  }
  for (int l = 0; l < numwin*N1; l+=N1) {
    computeNestedWindows(l);
    int i,j,k;
    int h = 2;
      for (i = 0; i < poweroftwo; ++i,h*=2) {
        for (j = 0; j < N1/h; ++j) {
          for (k = 0; k < h; ++k) {
            setX(i, j, k, sqrt(PSD(getX(i, j, k))));
            fprintf(file, "%d, ", creal(getX(i, j, k)));
          }
        }
      }
  }
  return 0;
}
#endif

int main()
{
  if (!testMyMath())
    return 1;
  if (!init())
    return 1;

  /*
  timeSTFT();
  return 0;
  */

  testSTFT("stft_test_zeros.csv", &teststftZero);
  testSTFT("stft_test_alternating.csv", &teststftAlternating);
  testSTFT("stft_test_dual_sines.csv", &teststftDualSine);
  testSTFT("stft_test_bandlimit.csv", &teststftBandlimit);
  testSTFT("stft_test_impulse.csv", &teststftImpulse);
  testSTFT("stft_test_dual_impulse.csv", &teststftDualImp);
  testSTFT("stft_test_shift_imp.csv", &teststftShiftImp);
  testSTFT("stft_test_single_sine.csv", &teststftSingleSine);
  testSTFT("stft_test_constant.csv", &teststftConst);
  testSTFT("stft_test_sinc.csv", &teststftSinc);
  //run();  

  long c = 0L;
  for (int i=0; i<itMax; ++i)
    if (t[i] != bogus)
      ++c;
  printf("nonbogus: %ld of %ld, %.2f%%\n", c, itMax, 100.0 * c/itMax);
  return 0;
}
