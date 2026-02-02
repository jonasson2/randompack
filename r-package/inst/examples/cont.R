Continuous distributions
The RNG object provides methods for generating random variates from common continuous probability distributions. All methods return a numeric vector of length len.

unif(len)
Uniform variates on [0,1).

unif(len, a, b)
Uniform variates on [a,b) with a < b.

normal(len)
Standard normal variates.

normal(len)
Standard normal variates (mean 0 and standard deviation 1).

normal(len, mu, sigma)
Normal variates with mean mu and standard deviation sigma.

skew_normal(len, mu, sigma, alpha)
Skew-normal variates with location mu, scale sigma, and shape alpha.

lognormal(len)
Standard lognormal variates (from standard normal).

lognormal(len, mu, sigma)
Lognormal variates derived from an underlying normal distribution
with mean mu and standard deviation sigma.

exp(len)
Standard exponential variates (with scale 1).

exp(len, scale)
Exponential variates with scale scale.

gamma(len, shape, scale)
Gamma variates with given shape and scale.

chi2(len, nu)
Chi-square variates with nu degrees of freedom.

beta(len, a, b)
Beta variates with shape parameters a and b.

t(len, nu)
Student’s t variates with nu degrees of freedom.

f(len, nu1, nu2)
F variates with nu1 and nu2 degrees of freedom.

gumbel(len, mu, beta)
Gumbel variates with location parameter mu and scale parameter beta.

pareto(len, xm, alpha)
Pareto variates with minimum value xm and shape parameter alpha.

weibull(len, shape, scale)
Weibull variates with given shape and scale.

#' @examples
rng <- randompack_rng()
x <- rng$unif(5)
x <- rng$normal(5)
x <- rng$normal(5, 2, 0.5)
x <- rng$normal(5, mu=2, sigma=0.5)
x <- rng$skew_normal(5, mu=0, sigma=1, alpha=2)
x <- rng$exp(5)
x <- rng$gamma(5, 2, 1)
x <- rng$t(5, nu=5
x <- rng$pareto(5, 1, alpha=2)
