using Random

function main()
  seed = length(ARGS) >= 1 ? parse(UInt32, ARGS[1]) : UInt32(1234)
  rng = MersenneTwister(seed)
  buf = Vector{UInt32}(undef, 65536)

  while true
    rand!(rng, buf)
    write(stdout, buf)
  end
end

try
  main()
catch e
  msg = lowercase(sprint(showerror, e))
  if e isa Base.IOError && occursin("broken pipe", msg)
    exit(0)
  else
    rethrow()
  end
end
