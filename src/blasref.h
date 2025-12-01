// Include file declaring the reference blas functions
//
// Note that vpack_dpstrf_ is used instead of dpstrf_ because the latter routine is
// faulty in Accelerate; lapack_dpstrf.f with Netlib's official code must be compiled and
// linked against.

#ifndef BLASREF_H
#define BLASREF_H

void daxpy_(int *n, double *alpha, double x[], int *incx, double y[], int *incy);

void dcopy_(int *n, double x[], int *incx, double y[], int *incy);

double ddot_(int *n, double x[], int *incx, double y[], int *incy);

void dgeev_(char *jobvl, char *jobvr, int *n, double a[], int *lda, double wr[], double
	    wi[], double vl[], int *ldvl, double vr[], int *ldvr, double work[], int
	    *lwork, int *info, int lenjobvl, int lenjobvr);

void dgemm_(char *transa, char *transb, int *m, int *n, int *k, double *alpha,
            double a[], int *lda, double b[], int *ldb, double *beta, double c[], 
            int *ldc, int lentransa, int lentransb);

void dgemv_(char *transa, int *m, int *n, double *alpha, double a[], int *lda,
            double x[], int *incx, double *beta, double y[], int *incy, int lentransa);

void dger_(int *m, int *n, double *alpha, double x[], int *incx, double y[], 
           int *incy, double a[], int *lda);

void dgetrf_(int *m, int *n, double a[], int *lda, int ipiv[], int *info);

void dgetrs_(char *transa, int *n, int *nrhs, double a[], int *lda, int ipiv[],
             double b[], int *ldb, int *info, int lentransa);

void dlacpy_(char *uplo, int *m, int *n, double a[], int *lda, double b[], int *ldb,
	     int lenuplo);

void dlaset_(char *uplo, int *m, int *n, double *alpha, double *beta, double a[], int
	     *lda, int lenuplo);

double dnrm2_(int *n, double x[], int *incx);

void dpotrf_(char *uplo, int *n, double a[], int *lda, int *info, int lenuplo);

void vpack_dpstrf_(char *uplo, int *n, double a[], int *lda, int piv[], int *rank,
	     double *tol, double work[], int *info, int lenuplo);

void dposv_(char *uplo, int *n, int *nrhs, double a[], int *lda, double b[], int *ldb,
            int *info, int lenuplo);

void dscal_(int *m, double *alpha, double *x, int *incx);

void dspr_(char *uplo, int *n, double *alpha, double x[], int *incx, double ap[],
           int lenuplo);

void dspr2_(char *uplo, int *n, double *alpha, double x[], int *incx, double y[],
            int *incy, double ap[], int lenuplo);

void dsyev_(char *jobz, char *uplo, int *n, double a[], int *lda, double w[],
            double work[], int *lwork, int *info, int lenjobz, int lenuplo);

void dsymm_(char *side, char *uplo, int *m, int *n, double *alpha, double a[], 
            int *lda, double b[], int *ldb, double *beta, double c[], int *ldc,
            int lenside, int lenuplo);

void dsymv_(char *uplo, int *n, double *alpha, double a[], int *lda, double x[],
            int *incx, double *beta, double y[], int *incy, int lenuplo);
void dsyr_(char *uplo, int *n, double *alpha, double x[], int *incx, double a[],
          int *lda, int lenuplo);

void dsyr2k_(char *uplo, char *trans, int *m, int *n, double *alpha, double a[], 
             int *lda, double b[], int *ldb, double *beta, double c[], int *ldc, 
             int lenuplo, int lentrans);
void dsyrk_(char *uplo, char *trans, int *m, int *n, double *alpha, double a[], 
            int *lda, double *beta, double c[], int *ldc, int lenuplo, int lentrans);

void dtrmm_(char *side, char *uplo, char *transa, char *diag, int *m, int *n,
            double *alpha, double a[], int *lda, double b[], int *ldb, int lenside, 
            int lenuplo, int lentransa, int lendiag);

void dtrmv_(char *uplo, char *trans, char *diag, int *n, double A[], int *lda,
            double x[], int *incx, int lenuplo, int lentrans, int lendiag);

void dtrsm_(char *side, char *uplo, char *transa, char *diag, int *m, int *n,
            double *alpha, double a[], int *lda, double b[], int *ldb, int lenside, 
            int lenuplo, int lentransa, int lendiag);

void dtrsv_(char *uplo, char *transa, char *diag, int *n, double a[], int *lda, 
            double x[], int *incx, int lenuplo, int lentransa, int lendiag);

int idamax_(int *n, double dx[], int *incx);

#endif // BLASREF_H
