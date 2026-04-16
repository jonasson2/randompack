seed = length(ARGS) >= 1 ? parse(Int, ARGS[1]) : 1
using Random

const CHUNK = 4096
Random.seed!(seed)
buf = Vector{Float64}(undef, CHUNK)

while true
  randn!(buf)
  write(stdout, buf)
end
