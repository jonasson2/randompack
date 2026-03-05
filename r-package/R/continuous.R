# R/randompack_continuous.R

methods_continuous <- list(
  unif = function(len, a = 0, b = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(a) || length(a) != 1L || is.na(a))
      stop("a must be a single numeric")
    if (!is.numeric(b) || length(b) != 1L || is.na(b))
      stop("b must be a single numeric")
    a <- as.numeric(a)
    b <- as.numeric(b)
    if (!(a < b)) stop("a must be less than b")
    .Call("randompack_unif_R", self$ptr, len, a, b, PACKAGE = "randompack")
  },

  normal = function(len, mu = 0, sigma = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(mu) || length(mu) != 1L || is.na(mu))
      stop("mu must be a single numeric")
    if (!is.numeric(sigma) || length(sigma) != 1L || is.na(sigma))
      stop("sigma must be a single numeric")
    mu <- as.numeric(mu)
    sigma <- as.numeric(sigma)
    if (!(sigma > 0)) stop("sigma must be positive")
    .Call("randompack_normal_R", self$ptr, len, mu, sigma,
          PACKAGE = "randompack")
  },

  skew_normal = function(len, mu = 0, sigma = 1, alpha) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(mu) || length(mu) != 1L || is.na(mu))
      stop("mu must be a single numeric")
    if (!is.numeric(sigma) || length(sigma) != 1L || is.na(sigma))
      stop("sigma must be a single numeric")
    if (!is.numeric(alpha) || length(alpha) != 1L || is.na(alpha))
      stop("alpha must be a single numeric")
    mu <- as.numeric(mu)
    sigma <- as.numeric(sigma)
    alpha <- as.numeric(alpha)
    if (!(sigma > 0)) stop("sigma must be positive")
    .Call("randompack_skew_normal_R", self$ptr, len, mu, sigma, alpha,
          PACKAGE = "randompack")
  },

  lognormal = function(len, mu = 0, sigma = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(mu) || length(mu) != 1L || is.na(mu))
      stop("mu must be a single numeric")
    if (!is.numeric(sigma) || length(sigma) != 1L || is.na(sigma))
      stop("sigma must be a single numeric")
    mu <- as.numeric(mu)
    sigma <- as.numeric(sigma)
    if (!(sigma > 0)) stop("sigma must be positive")
    .Call("randompack_lognormal_R", self$ptr, len, mu, sigma,
          PACKAGE = "randompack")
  },

  gumbel = function(len, mu = 0, beta = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(mu) || length(mu) != 1L || is.na(mu))
      stop("mu must be a single numeric")
    if (!is.numeric(beta) || length(beta) != 1L || is.na(beta))
      stop("beta must be a single numeric")
    mu <- as.numeric(mu)
    beta <- as.numeric(beta)
    if (!(beta > 0)) stop("beta must be positive")
    .Call("randompack_gumbel_R", self$ptr, len, mu, beta,
          PACKAGE = "randompack")
  },

  pareto = function(len, xm, alpha) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(xm) || length(xm) != 1L || is.na(xm))
      stop("xm must be a single numeric")
    if (!is.numeric(alpha) || length(alpha) != 1L || is.na(alpha))
      stop("alpha must be a single numeric")
    xm <- as.numeric(xm)
    alpha <- as.numeric(alpha)
    if (!(xm > 0)) stop("xm must be positive")
    if (!(alpha > 0)) stop("alpha must be positive")
    .Call("randompack_pareto_R", self$ptr, len, xm, alpha,
          PACKAGE = "randompack")
  },

  exp = function(len, scale = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(scale) || length(scale) != 1L || is.na(scale))
      stop("scale must be a single numeric")
    scale <- as.numeric(scale)
    if (!(scale > 0)) stop("scale must be positive")
    .Call("randompack_exp_R", self$ptr, len, scale, PACKAGE = "randompack")
  },

  gamma = function(len, shape, scale = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(shape) || length(shape) != 1L || is.na(shape))
      stop("shape must be a single numeric")
    if (!is.numeric(scale) || length(scale) != 1L || is.na(scale))
      stop("scale must be a single numeric")
    shape <- as.numeric(shape)
    scale <- as.numeric(scale)
    if (!(shape > 0)) stop("shape must be positive")
    if (!(scale > 0)) stop("scale must be positive")
    .Call("randompack_gamma_R", self$ptr, len, shape, scale,
          PACKAGE = "randompack")
  },

  chi2 = function(len, nu) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(nu) || length(nu) != 1L || is.na(nu))
      stop("nu must be a single numeric")
    nu <- as.numeric(nu)
    if (!(nu > 0)) stop("nu must be positive")
    .Call("randompack_chi2_R", self$ptr, len, nu, PACKAGE = "randompack")
  },

  beta = function(len, a, b) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(a) || length(a) != 1L || is.na(a))
      stop("a must be a single numeric")
    if (!is.numeric(b) || length(b) != 1L || is.na(b))
      stop("b must be a single numeric")
    a <- as.numeric(a)
    b <- as.numeric(b)
    if (!(a > 0)) stop("a must be positive")
    if (!(b > 0)) stop("b must be positive")
    .Call("randompack_beta_R", self$ptr, len, a, b, PACKAGE = "randompack")
  },

  t = function(len, nu) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(nu) || length(nu) != 1L || is.na(nu))
      stop("nu must be a single numeric")
    nu <- as.numeric(nu)
    if (!(nu > 0)) stop("nu must be positive")
    .Call("randompack_t_R", self$ptr, len, nu, PACKAGE = "randompack")
  },

  f = function(len, nu1, nu2) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(nu1) || length(nu1) != 1L || is.na(nu1))
      stop("nu1 must be a single numeric")
    if (!is.numeric(nu2) || length(nu2) != 1L || is.na(nu2))
      stop("nu2 must be a single numeric")
    nu1 <- as.numeric(nu1)
    nu2 <- as.numeric(nu2)
    if (!(nu1 > 0)) stop("nu1 must be positive")
    if (!(nu2 > 0)) stop("nu2 must be positive")
    .Call("randompack_f_R", self$ptr, len, nu1, nu2, PACKAGE = "randompack")
  },

  weibull = function(len, shape, scale = 1) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    len <- as.integer(len)
    if (length(len) != 1L || is.na(len) || len < 0L)
      stop("len must be a non-negative integer")
    if (!is.numeric(shape) || length(shape) != 1L || is.na(shape))
      stop("shape must be a single numeric")
    if (!is.numeric(scale) || length(scale) != 1L || is.na(scale))
      stop("scale must be a single numeric")
    shape <- as.numeric(shape)
    scale <- as.numeric(scale)
    if (!(shape > 0)) stop("shape must be positive")
    if (!(scale > 0)) stop("scale must be positive")
    .Call("randompack_weibull_R", self$ptr, len, shape, scale,
          PACKAGE = "randompack")
  },
  mvn = function(n, Sigma, mu = NULL) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    n <- as_int1(n, "n")
    if (n < 0L) stop("n must be non-negative")
    if (is.null(Sigma)) stop("Sigma must not be NULL")
    if (!is.matrix(Sigma) || !is.numeric(Sigma))
      stop("Sigma must be a numeric matrix")
    d <- nrow(Sigma)
    if (d <= 0L || ncol(Sigma) != d)
      stop("Sigma must be a non-empty square matrix")
    if (anyNA(Sigma)) stop("Sigma must not contain NA")
    if (!all(is.finite(Sigma))) stop("Sigma must be finite")
    if (!all(Sigma == t(Sigma))) stop("Sigma must be symmetric")
    storage.mode(Sigma) <- "double"
    if (!is.null(mu)) {
      if (!is.numeric(mu)) stop("mu must be a numeric vector")
      mu <- as.numeric(mu)
      if (length(mu) != d) stop("mu length must match Sigma")
      if (anyNA(mu)) stop("mu must not contain NA")
      if (!all(is.finite(mu))) stop("mu must be finite")
    }
    .Call("randompack_mvn_R", self$ptr, n, Sigma, mu,
          PACKAGE = "randompack")
  }
)
