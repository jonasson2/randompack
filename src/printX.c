#include <inttypes.h>
#include "printX.h"
#ifdef DEBUG_PRINT

#include <math.h>
#include <string.h>
#include <ctype.h>

#ifdef MEX
#include "mex.h"
#define printf mexPrintf
extern int mexPrintf(const char *fmt, ...);
#define fflush(x)

#else
#include <stdio.h>
#endif

static int noprint = 0;
static int width = 7;
static int ndec = 3;
static char fmtc = 'f';

void printOff(void)                    { noprint = 1; }
void printOn(void)                     { noprint = 0; }
void printSetStatus(int status)        { noprint = status; }
int printIsOff(void)                   { return noprint; }
void printSetNdec(int n)               { ndec = n; }
void printSetWidth(int w)              { width = w; }
void printSetFmtChar(char f)           { fmtc = f; }
void printSetFmt(int w, int n, char f) { width = w; ndec = n; fmtc = f; }
int printGetNdec(void)                 { return ndec; }

void printNewl(void) { // print new line
  if (noprint) return;
  printf("\n");
  fflush(0);
}

void printMsg(const char *s) { // print message (or string without name)
  if (noprint) return;
  printf("%s\n", s);
  fflush(0);
}

void printMsgUpper(const char *s) {
  while (*s) putchar(toupper((unsigned char)*s++));
  putchar('\n');
  fflush(0);
}

void printMsgUnderl(const char *s) { // print underlined message
  int i, nm = strlen(s);
  putchar('\n');
  printMsg(s);
  for (i=0; i<nm; i++) printf("%c", '-');
  putchar('\n');
  fflush(0);
}

void printI(const char *name, int i) { // print named int
  if (noprint) return;
  printf("%s = %d\n", name, i);
  fflush(0);
}

void print64(const char *name, uint64_t x) {
  if (noprint) return;
  printf("%s = %" PRIu64 "\n", name, x);
  fflush(0);
}

void print32(const char *name, uint32_t x) {
  if (noprint) return;
  printf("%s = %u\n", name, x);
  fflush(0);
}

void printP(const char *name, const void *p) {
  if (noprint) return;
  printf("%s = %p\n",name, p);
  fflush(0);
}

void printD(const char *name, double d) { // print named double
  char fmt[] = "%s = %1.*_\n", *uscore;
  if (noprint) return;
  uscore = strchr(fmt, '_'); *uscore = fmtc;
  printf(fmt,name,ndec,d);
  fflush(0);
}

void printV(const char *name, const double x[], int n) { // print named vector
  char fmt[] = "%1.*_ ", *uscore;
  int j;
  if (noprint) return;
  uscore = strchr(fmt, '_'); *uscore = fmtc;
  printf("%s = ",name);
  if (x==0)
    printf("NULL");
  else
    for (j=0; j<n; j++) printf(fmt, ndec, x[j]);
  printf("\n");
  fflush(0);
}

void printIV(const char *name, const int iv[], int n) { // print named integer vector
  int j;
  if (noprint) return;
  printf("%s = ",name);
  for (j=0; j<n; j++) printf("%d ", iv[j]);
  printf("\n");
  fflush(0);
}

void printIVS(const char *name, const int iv[], int n) { // summarized integer vector
  if (noprint) return;
  printf("%s = ", name);
  if (!iv) {
    printf("NULL\n");
    fflush(0);
    return;
  }
  if (n <= 6) {
    for (int i = 0; i < n; i++) printf("%d ", iv[i]);
  }
  else {
    for (int i = 0; i < 4; i++) printf("%d ", iv[i]);
    printf("... ");
    for (int i = n - 2; i < n; i++) printf("%d ", iv[i]);
  }
  printf("\n");
  fflush(0);
}

void printS(const char *name, const char *s) { // print named string
  if (noprint) return;
  printf("%s = %s\n",name,s);
  fflush(0);
}

static void printMat(char *transp, const char *name, const double d[], int ldd,
		     int nr, int nc) {
  // local function to print matrix, called by printM and printMP
  //printf("In printMat, d[1]=%.3f\n", d[1]);
  char fmt[] = "%*.*_ ", *uscore;
  int i, j, is, ns, idx;
  if (noprint) return;
  uscore = strchr(fmt, '_'); *uscore = fmtc;
  ns = strlen(name);
  printf("%s = ",name);
  if (d==0) 
    printf("NULL\n");
  else {
    for (i=0; i<nr; i++) {
      if (i>0) for (is=0; is<ns+3; is++) printf(" ");
      for (j=0; j<nc; j++) {
        idx = *transp == 'N' ? j*ldd + i : i*ldd + j;
        //printf("In printMat, idx=%d\n", idx);
        if (fmtc == 'f') { // when fmt is f print < 1e-14 as 0
          if (fabs(d[idx]) >= 1e-14 && fmtc == 'f')
            printf("%*.*f ", width, ndec, d[idx]);
          else
            printf("%*.0f ", width, d[idx]);
        }
        else {
          printf(fmt, width, ndec, d[idx]);
        }
      }
      printf("\n");
    }
    if (nr==0) printf("\n");
  }
  fflush(0);
}

void printM(const char *name, const double A[], int nr, int nc) { // print named matrix
  //printf("In printM, A[1]=%.3f\n", A[1]);
  printMat("N", name, A, nr, nr, nc);
}

void printMT(const char *name, const double A[], int nr, int nc) { // print transpose
  //printf("In printMT, A[1]=%.3f\n", A[1]);
  printMat("T", name, A, nr, nc, nr);
}

void printMP(const char *name, const double *ap, int nr, int nc,
             const double A[], int ldA) {
  // print submatrix
  char s[100];
  int r = (ap-A) % ldA;
  int c = (ap-A) / ldA;
  sprintf(s, "%s[%d:%d,%d:%d]", name, r, r+nr-1, c, c+nc-1);
  printMat("N", s, ap, ldA, nr, nc);
}

// Following functions are useful for debugging:
void print2I(const char *s, int i1, int i2) { // print 2 integers
  if (noprint) return;
  printf("%s = %d, %d\n", s, i1, i2);
  fflush(0);
}

void print3I(const char *s, int i1, int i2, int i3) { // print 3 integers
  if (noprint) return;
  printf("%s = %d, %d, %d\n", s, i1, i2, i3);
  fflush(0);
}

void print4I(const char *s, int i1, int i2, int i3, int i4) { // print 4 integers
  if (noprint) return;
  printf("%s = %d, %d, %d, %d\n", s, i1, i2, i3, i4);
  fflush(0);
}

void print5I(const char *s, int i1, int i2, int i3, int i4, int i5) { // print 5 ints
  if (noprint) return;
  printf("%s = %d, %d, %d, %d, %d\n", s, i1, i2, i3, i4, i5);
  fflush(0);
}

void print6I(const char *s, int i1, int i2, int i3, int i4, int i5, int i6) { // 6 ints
  if (noprint) return;
  printf("%s = %d, %d, %d, %d, %d, %d\n", s, i1, i2, i3, i4, i5, i6);
  fflush(0);
}

#endif /* DEBUG_PRINT */
