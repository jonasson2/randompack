% TimeDistributions.m
% Time continuous distributions in MATLAB (ns/value)

clear;
rng(7);                     % fixed seed
chunk = 4096;
bench_time = 0.2;           % seconds per distribution
reps = max(1, floor(1e6 / chunk));

fprintf('Distribution       ns/value\n');

% Warmup (JIT, first-call effects)
for i=1:50000, rand(1000,1); end  % for the cpu boosting
rand(chunk,1);
randn(chunk,1);
exprnd(1,chunk,1);
wblrnd(1,2,chunk,1);

% u01
ns = time_dist(@() rand(chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'u01', ns);

% unif(2,5)
ns = time_dist(@() 2 + 3*rand(chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'unif(2,5)', ns);

% norm
ns = time_dist(@() randn(chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'norm', ns);

% normal(2,3)
ns = time_dist(@() 2 + 3*randn(chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'normal(2,3)', ns);

% lognormal(0,1)
ns = time_dist(@() lognrnd(0,1,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'lognormal(0,1)', ns);

% gumbel(0,1)
ns = time_dist(@() -log(-log(rand(chunk,1))), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'gumbel(0,1)', ns);

% pareto(1,2)  (Type I, xm=1, alpha=2)
ns = time_dist(@() (1 - rand(chunk,1)).^(-1/2), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'pareto(1,2)', ns);

% exp(1)
ns = time_dist(@() exprnd(1,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'exp(1)', ns);

% exp(2)
ns = time_dist(@() exprnd(2,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'exp(2)', ns);

% gamma(2,3)
ns = time_dist(@() gamrnd(2,3,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'gamma(2,3)', ns);

% chi2(5)
ns = time_dist(@() chi2rnd(5,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'chi2(5)', ns);

% beta(2,5)
ns = time_dist(@() betarnd(2,5,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'beta(2,5)', ns);

% t(10)
ns = time_dist(@() trnd(10,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 't(10)', ns);

% F(5,10)
ns = time_dist(@() frnd(5,10,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'F(5,10)', ns);

% weibull(2,1)
ns = time_dist(@() wblrnd(1,2,chunk,1), chunk, reps, bench_time);
fprintf('%-18s %8.2f\n', 'weibull(2,1)', ns);


% ---------- local function ----------

function ns = time_dist(f, chunk, reps, bench_time)
  calls = 0;
  t0 = tic;
  while toc(t0) < bench_time
    for i = 1:reps
      x = f();
      sink = x(end); %#ok<NASGU>  % force use
    end
    calls = calls + reps;
  end
  ns = 1e9 * toc(t0) / (calls * chunk);
end
