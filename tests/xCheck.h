// Declare xCheck and related functions
#ifndef XCHECK_H
#define XCHECK_H

#define xCheck(e) ((e) ? (void)xCheckOK() : \
  xCheckFunc(#e, __FILE__, __LINE__, __func__, 0))
// returns silently if the condition e is true; if it is false, then the condition and the
// source location are printed. The counter returned by xCheckNTotal is incremented and
// failures also increment the static counter returned by xCheckNfailures.

#define xCheckMsg(e,msg) ((e) ? (void)xCheckOK() : \
  xCheckFunc(#e, __FILE__, __LINE__, __func__, msg))

void xCheckFunc(char *message, const char *file, int line, const char *func, const
                char *ctx);

void xCheckOK(void);

void xCheckInit(void);
// Resets counters.

int xCheckNFailures(void);
// Returns number of failures since last xCheckInit call.

int xCheckNTotal(void);
// Returns total number of xCheck calls since last xCheckInit call.

#endif
