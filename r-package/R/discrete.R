# R/randompack_discrete.R

as_int1 <- function(x, name) {
  if (!is.numeric(x) || length(x) != 1L || is.na(x))
    stop(sprintf("%s must be a length-1 numeric", name))
  x <- as.numeric(x)
  if (!is.finite(x)) stop(sprintf("%s must be finite", name))
  if (x != floor(x)) stop(sprintf("%s must be integer-ish", name))
  if (x < -2147483648 || x > 2147483647)
    stop(sprintf("%s is out of range", name))
  as.integer(x)
}

methods_discrete <- list(
  int = function(len, min, max) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as_int1(len, "len")
    min <- as_int1(min, "min")
    max <- as_int1(max, "max")
    if (len < 0L) stop("len must be non-negative")
    if (min > max) stop("min must be <= max")
    .Call("randompack_int_R", self$ptr, len, min, max, PACKAGE = "randompack")
  },

  perm = function(n) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    n <- as_int1(n, "n")
    if (n < 0L) stop("n must be non-negative")
    .Call("randompack_perm_R", self$ptr, n, PACKAGE = "randompack")
  },

  sample = function(n, k) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    n <- as_int1(n, "n")
    k <- as_int1(k, "k")
    if (n < 0L) stop("n must be non-negative")
    if (k < 0L) stop("k must be non-negative")
    if (k > n) stop("k must be <= n")
    .Call("randompack_sample_R", self$ptr, n, k, PACKAGE = "randompack")
  },

  raw = function(n) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    n <- as_int1(n, "n")
    if (n < 0L) stop("n must be non-negative")
    .Call("randompack_raw_R", self$ptr, n, PACKAGE = "randompack")
  }
)

NULL

NULL
