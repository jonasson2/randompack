using Randompack
using Base.Threads

function worker(i, rng, results)
  results[i] = random_normal(rng, 1000)
end

rng = Vector{Randompack.RNG}(undef, 4)
rng[1] = rng_create("pcg64")
rng_seed!(rng[1], 123)
results = Vector{Vector{Float64}}(undef, 3)
tasks = Vector{Task}(undef, 3)
for i in 1:3
  rng[i+1] = Randompack.duplicate(rng[i])
  Randompack.pcg64_set_inc!(rng[i+1], [1, i-1]) # first entry must be odd
  tasks[i] = @spawn worker(i, rng[i+1], results)
end
foreach(fetch, tasks) # wait till tasks finish
println(results)
