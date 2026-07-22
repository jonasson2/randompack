using Randompack
using Base.Threads

function worker(i, results)
  rng = rng_create()
  rng_seed!(rng, 123; spawn_key=[i-1])
  results[i] = random_normal(rng, 1000)
end

results = Vector{Vector{Float64}}(undef, 3)
tasks = Vector{Task}(undef, 3)
for i in 1:3
  tasks[i] = @spawn worker(i, results)
end
foreach(fetch, tasks) # wait till tasks finish
println(results)
