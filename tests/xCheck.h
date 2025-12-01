// Declare xCheck and related functions
#ifndef XCHECK_H
#define XCHECK_H

#define xCheck(e) ((e) ? (void)xCheckOK() : xCheckFunc(#e, __FILE__, __LINE__)) 
// returns silently if the condition e is true, but if it is false, then it is printed
// (literally) on stderr along with the source file name and line number where the check
// was made, the common message set with xCheckInit, and the additional message set with
// xCheckAddMsg (if any). The counter returned by xCheckNTotal is incremented and failures
// also increment the static counter returned by xCheckNfailures.

#define xCheckMessage(e,msg)  ((e) ? (void)xCheckOK() : xCheckFunc(msg,__FILE__,__LINE__))
// xCheckMessage(e, msg) behaves as xCheck except that msg is printed instead of the
// condition upon failure.

void xCheckFunc(const char *message, char* file, int line);

void xCheckOK(void);

void xCheckInit(const char *msg);
// Initializes static variables commonmsg, addmsg and NFAIL of xCheck.c to msg, "" and 0.

void xCheckAddMsg(const char *amsg);
// sets additional message (addmsg) to amsg.

int xCheckNFailures(void);
// Returns number of failures since last xCheckInit call.

int xCheckNTotal(void);
// Returns total number of xCheck calls since last xCheckInit call.

#endif
