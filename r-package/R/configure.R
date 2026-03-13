# R/configure.R

as_u32_vec <- function(x, name, max_len = 0L, even = FALSE) {
  if (is.null(x)) stop(sprintf("%s must not be NULL", name))
  if (!is.numeric(x)) stop(sprintf("%s must be a numeric vector", name))
  x <- as.numeric(x)
  n <- length(x)
  if (n == 0L) stop(sprintf("%s must not be empty", name))
  if (max_len > 0L && n > max_len)
    stop(sprintf("%s must have length <= %d", name, max_len))
  if (even && (n %% 2L) != 0L)
    stop(sprintf("%s must have even length", name))
  if (anyNA(x)) stop(sprintf("%s must not contain NA", name))
  if (!all(is.finite(x))) stop(sprintf("%s must be finite", name))
  if (any(x < 0 | x > 4294967295))
    stop(sprintf("%s entries must be in [0, 2^32-1]", name))
  if (any(x != floor(x)))
    stop(sprintf("%s entries must be whole numbers", name))
  x
}

methods_configure <- list(
  seed = function(seed, spawn_key = integer(0)) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    seed <- as.integer(seed)
    if (length(seed) != 1L || is.na(seed)) stop("seed must be a single integer")
    if (is.null(spawn_key)) spawn_key <- integer(0)
    if (!is.integer(spawn_key)) spawn_key <- as.integer(spawn_key)
    if (anyNA(spawn_key)) stop("spawn_key must not contain NA")
    .Call("randompack_seed_R", self$ptr, seed, spawn_key, PACKAGE = "randompack")
    invisible(self)
  },

  randomize = function() {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    .Call("randompack_randomize_R", self$ptr, PACKAGE = "randompack")
    invisible(self)
  },

  full_mantissa = function(enable = TRUE) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    if (length(enable) != 1L || is.na(enable))
      stop("enable must be TRUE or FALSE")
    enable <- as.logical(enable)
    .Call("randompack_full_mantissa_R", self$ptr, enable, PACKAGE = "randompack")
    invisible(self)
  },

  jump = function(p) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    p <- as.integer(p)
    if (length(p) != 1L || is.na(p))
      stop("p must be a single integer")
    .Call("randompack_jump_R", self$ptr, p, PACKAGE = "randompack")
    invisible(self)
  },


  set_state = function(state) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    state <- as_u32_vec(state, "state", even = TRUE)
    .Call("randompack_set_state_R", self$ptr, state, PACKAGE = "randompack")
    invisible(self)
  },

  pcg64_set_inc = function(inc) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    inc <- as_u32_vec(inc, "inc", max_len = 4L)
    .Call("randompack_pcg64_set_inc_R", self$ptr, inc, PACKAGE = "randompack")
    invisible(self)
  },

  philox_set_state = function(counter, key) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    counter <- as_u32_vec(counter, "counter", max_len = 8L)
    key <- as_u32_vec(key, "key", max_len = 4L)
    .Call("randompack_philox_set_state_R", self$ptr, counter, key, PACKAGE = "randompack")
    invisible(self)
  },

  squares_set_state = function(counter, key) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    counter <- as_u32_vec(counter, "counter", max_len = 2L)
    key <- as_u32_vec(key, "key", max_len = 2L)
    .Call("randompack_squares_set_state_R", self$ptr, counter, key,
          PACKAGE = "randompack")
    invisible(self)
  },

  duplicate = function() {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    ext <- .Call("randompack_duplicate_R", self$ptr, PACKAGE = "randompack")
    out <- randompack_rng(engine = self$engine)
    .Call("randompack_free_R", out$ptr, PACKAGE = "randompack")
    out$ptr <- ext
    out
  },
  
  serialize = function() {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    .Call("randompack_serialize_R", self$ptr, PACKAGE = "randompack")
  },

  deserialize = function(raw_state) {
    if (is.null(self$ptr)) stop("RNG is not initialized")
    if (!is.raw(raw_state)) stop("raw_state must be a raw vector")
    .Call("randompack_deserialize_R", self$ptr, raw_state, PACKAGE = "randompack")
    invisible(self)
  }
)

NULL
