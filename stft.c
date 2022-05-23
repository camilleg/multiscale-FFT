#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// Argument x must be odd.
unsigned reverse(unsigned x)
{
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
  x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
  x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
  x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
  x = ( (x >> 16)              |  (x << 16)             );
  while (x % 2 == 0) {
    x >>= 1;
  }
  return x;
}

typedef struct { double rp, ip; } complex;

complex timesRC(const double x, const complex c)
{
  complex r;
  r.rp = x * c.rp;
  r.ip = x * c.ip;
  return r;
}

complex timesCC(const complex c1, const complex c2)
{
  complex r;
  r.rp = (c1.rp*c2.rp - c1.ip*c2.ip);
  r.ip = (c1.rp*c2.ip + c1.ip*c2.rp);
  return r;
}

complex addCC(const complex c1, const complex c2)
{
  complex r;
  r.rp = c1.rp + c2.rp;
  r.ip = c1.ip + c2.ip;
  return r;
}

complex addRC(const double x, const complex c)
{
  complex r;
  r.rp = x + c.rp;
  r.ip = c.ip;
  return r;
}

complex subCC(const complex c1, const complex c2)
{
  complex r;
  r.rp = c1.rp - c2.rp;
  r.ip = c1.ip - c2.ip;
  return r;
}

complex subRC(const double x, const complex c)
{
  complex r;
  r.rp = x - c.rp;
  r.ip = c.ip;
  return r;
}

complex ce(const int n, const int N)
{
  complex r;
  r.rp = cos((-M_PI*2*n)/N);
  r.ip = sin((-M_PI*2*n)/N);
  return r;
}

complex CfromR(const double x)
{
  complex r;
  r.rp = x;
  r.ip = 0.0;
  return r;
  // todo: test this, even as simple as it is.
}

double PSD(const complex x)
{
  return x.rp*x.rp + x.ip*x.ip;
}

int testMyMath()
{
  complex r; r.rp = 1; r.ip = 2;
  complex c; c.rp = 3; c.ip = 4;
  complex foo = timesRC(5,r);
  complex foo1 = timesCC(r,c);
  complex foo2 = ce(1,4);
  double foo3 = PSD(r);
  double foo4 = PSD(c);
  printf("test result for timesCR is %f, %f\n", foo.rp, foo.ip);
  printf("test result for timesCC is %f, %f\n", foo1.rp, foo1.ip);
  printf("test result for ce is %f, %f\n", foo2.rp, foo2.ip);
  printf("test result for PSD (1,2) is %f\n", foo3);
  printf("test result for PSD (3,4) is %f\n", foo4);
  // todo: if these tests fail, return 0
  int i;
  for (i=1; i<=999; i+=2) {
    if (reverse(reverse(i)) != i) {
      printf("bit-reverse failed for i=%d\n", i);
      return 0;
    }
  }
  return 1;
}

float * buf = NULL;
complex* t = NULL;
complex* X = NULL;

#define poweroftwo (11)
#define N1 (1 << poweroftwo)
int numwin = -1;

// Stuff buf, only one window long (N1).

void teststftZero(void)
{
  int i;
  for (i=0; i<N1; ++i)
    buf[i] = 0;
}

void teststftAlternating(void)
{
  int i;
  for (i=0; i<N1; ++i)
    buf[i] = (i % 2 == 0) ? 1 : -1;
}

void teststftBandlimit(void)
{
  int b, i;
  for (b = (M_PI/8)*1000; b < (M_PI/4)*1000; ++b){
    for (i=0; i<N1; ++i){
      buf[i] += sin((b/1000.0)*i);
    }
  }
}

void teststftSinc(void)
{
  int i;
  for (i=0; i<N1; ++i)
    buf[i] = sin((M_PI/8)*i)/((M_PI/8)*i);
}

void teststftImpulse(void)
{
  buf[0] = 100; // why changed to 10 ? ;;
}

void teststftDualImp(void)
{
  buf[0] = 10;
  buf[356] = 50; // why changed to buf[1780] = 50 ? ;;
}

void teststftShiftImp(void)
{
  buf[500] = 100;
}

void teststftSingleSine(void)
{
  int i;
  for (i=0; i<N1; ++i){ 
    buf[i] = cos((M_PI/4)*i);
  }
}

void teststftDualSine(void)
{
  int i;
  for (i=0; i<N1; ++i){ 
    buf[i] = 5*cos((M_PI/3)*i) + cos((M_PI/16)*i);
  }
}

void teststftConst(void)
{
  int i;
  for (i=0; i<N1; ++i)
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

const complex bogus = { 999.9876, 999.9876 } ;
const long itMax = (poweroftwo-1) * poweroftwo * N1/2;
const long iXMax = poweroftwo * N1;

int init()
{
  const long C = sizeof(complex);
  printf("Array t will be %ld MB.\n", C * itMax /1000000);
  printf("Array X will be %ld MB.\n", C * iXMax /1000000);
  t = malloc(itMax * C);
  X = malloc(iXMax * C);
  buf = malloc(N1*sizeof(float));
  if (!buf || !t || !X) {
    printf("out of memory.\n");
    return 0;
  }
  int i;
  for (i=0; i<itMax; ++i)
    t[i] = bogus;

  numwin = floor(numbytes/N1);
  // printf("The data file has %d windows.\n", numwin);

  return 1;
}

#define getT(i, j, k) (t[(i)*poweroftwo*N1/2 + (j)*2 + (k)])
#define getX(i, j) (X[(i)*N1 + (j)])

void computeTemp1(const int offset)
{
  int iBsize, iTnum, iTnum1, iTnum2, iTstep, iTstep1;
  int Bsize = N1/2;
  int Smax = poweroftwo*N1/4;
  int Ssize = Smax;
  const float* window = buf + offset;
  for(iBsize = 0; iBsize < poweroftwo-1; iBsize++, Bsize /= 2, Ssize /= 2){
    iTstep = 0;
    for(iTstep1 = 0; iTstep1 < Smax; iTstep1 += Ssize, iTstep += Bsize*2){
      iTnum = 0;
      for(iTnum1 = 0; iTnum1 < Bsize/2; iTnum1++){
        for(iTnum2 = 0; iTnum2 < 2; iTnum2++, iTnum++){
          getT(iBsize, iTstep1 + iTnum1, iTnum2) =
            timesRC((window[iTnum + iTstep] - window[iTnum + iTstep + Bsize]),
              ce(iTnum, Bsize*2));
        }
      }
    }
  }
}

void computeSize2DFTs(const int offset)
{
  const float* window = buf + offset;
  int iWnum;
  for (iWnum = 0; iWnum < N1; iWnum+=2) {
    // Actually timesRC(w +- w, ce(0,2)).
    getX(0, iWnum    ) = CfromR(window[iWnum] + window[iWnum+1]);
    getX(0, iWnum + 1) = CfromR(window[iWnum] - window[iWnum+1]);
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
  for (iBsize = 0; iBsize < poweroftwo-1; iBsize++,Bsize/=2,pot--,c*=2,Ssize1/=2) {
    int Bnum;
    for (Bnum = 0; Bnum < c; Bnum++) {
      int Ssize=Bsize;
      int count1 = 0;
      int step;
      for (step = 0; step < pot; step++,Ssize/=2) {
	const int Ssize2 =  Ssize/2;
        const int iTstep = Bnum*Ssize1;
        int count = 0;
	int i;
        for (i = 0; i < Bsize; i+=2, count1++) {
          const int iTnum1 = iTstep + count1;
	  int iTnum;
          for(iTnum = 0; iTnum < 2; iTnum++, count++) {
	    // This block spends most of the compute time.
	    complex* const tt = &getT(iBsize, iTnum1, iTnum);
	    tt[Bsize] = (Ssize > 2) ?
              ((count % Ssize < Ssize2) ?
                addCC(tt[Ssize2], tt[0]) :
                timesCC(subCC(tt[-Ssize2], tt[0]), ce(count%Ssize - Ssize2, Ssize))
	      )
            : ((iTnum % 2 < 1) ?
		addCC(tt[0], tt[1]) :
		timesCC(subCC(tt[-1],      tt[0]), ce(count%Ssize - Ssize2, Ssize))
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
  int iWsize,iWnum,iTnum,iTnum1,iTnum2;
  int d = N1/4;
  int iBsize = poweroftwo-2;
  int Smin = poweroftwo;
  int Bsize = 4;
  int pot = 1;
  for (iWsize = 1; iWsize < poweroftwo; iWsize++,d/=2,iBsize--,Bsize*=2,Smin*=2,pot++) {
    int Tstep = pot * Bsize/4;
    for (iWnum = 0; iWnum < d; iWnum++, Tstep+=Smin) {
      int count = 0;
      iTnum = Bsize/2;
      for (iTnum1 = 0; iTnum1 < Bsize/2; iTnum1+=2, count++) {
        for (iTnum2 = 0; iTnum2 < 2; iTnum2++, iTnum++) {
          getX(iWsize, reverse(iTnum) + (iWnum * Bsize)) =
            getT(iBsize, Tstep + count, iTnum2);
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
  int iWsize, iWnum, iSnum;
  for (iWsize = 0; iWsize < iWsizeMax-1; iWsize++,iSnumMax*=2) {
    const int iWnumMax = N1/(iSnumMax*2);
    for (iWnum = 0; iWnum < iWnumMax; iWnum+=1) {
      for (iSnum = 0; iSnum < iSnumMax; iSnum++) {
        getX(iWsize+1, (iSnum*2) + (iWnum*iSnumMax*2)) =
          addCC(getX(iWsize, iSnum + (iSnumMax*iWnum*2)),
            getX(iWsize, iSnum + iSnumMax*(iWnum*2 + 1)));
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
  FILE *file;
  teststftZero();
  testfunction();
  file = fopen(filename, "w");
  if (file==NULL) {
    printf("failed to create test-output file '%s'.\n", filename);
    return;
  }

  int i,j;
#if 1
  for (j=0; j<N1; ++j) {
    for (i=0; i<poweroftwo; ++i) {
      getX(i, j).rp = 999.999; // output file should not have any of these!
    }
  }
#endif

  computeNestedWindows(0);
  int h = 2;
  for (i = 0; i < poweroftwo; i++, h*=2) {
    fprintf(file,"\"w. 2^%d=%d.\",", i+1, h);
  }
  fprintf(file,"\n");

  for (j=0; j<N1; j++) {
    for (i=0; i<poweroftwo; i++) {
      getX(i, j).rp = PSD(getX(i, j));
      fprintf(file, "%f,", getX(i, j).rp);
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

  int i;
  for (i=0; i<iMax; ++i)
    computeNestedWindows(0);

  gettimeofday(&t, 0);
  printf("Mean %.4f usec\n", ((t.tv_sec-t0.tv_sec)*1e6 + (t.tv_usec-t0.tv_usec))/iMax);
}

#if 0
void run()
{
  const char* filename = "stft_test.csv";
  FILE *file = fopen(filename, "w"); 
  if(file==NULL) {
    printf("failed to create test-output file '%s'.\n", filename);
    return 1;
  }

  int l;  
  for (l = 0; l < numwin*N1; l+=N1) {
    computeNestedWindows(l);
    int i,j,k;
    int h = 2;
      for (i = 0; i < poweroftwo; ++i,h*=2) {
        for (j = 0; j < N1/h; ++j) {
          for (k = 0; k < h; ++k) {
            getX(i, j, k).rp = sqrt(getX(i, j, k).rp*getX(i, j, k).rp + getX(i, j, k).ip*getX(i, j, k).ip);
            fprintf(file, "%d, ", getX(i, j, k).rp);
          }
        }
      }
  }
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

  long c = 0;
  int i;
  for (i=0; i<itMax; ++i)
    if (memcmp(&t[i], &bogus, sizeof(complex)))
      ++c;
  printf("nonbogus: %ld of %ld, %.2f%%\n", c, itMax, 100.0 * c/itMax);

  return 0;
}