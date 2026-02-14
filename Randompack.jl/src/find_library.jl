using Libdl

const _libpath = Ref{String}("librandompack")

function _choose_libpath()::String
  try
    Base.require(@__MODULE__, :Randompack_jll)
    jll = getfield(@__MODULE__, :Randompack_jll)
    return getfield(jll, :librandompack_path)
  catch
  end
  p = get(ENV, "RANDOMPACK_LIB", "")
  if !isempty(p)
    return p
  end
  prefix = get(ENV, "RANDOMPACK_PREFIX", "")
  if !isempty(prefix)
    if Sys.isapple()
      return joinpath(prefix, "lib", "librandompack.dylib")
    elseif Sys.iswindows()
      return joinpath(prefix, "bin", "librandompack.dll")
    else
      return joinpath(prefix, "lib", "librandompack.so")
    end
  end
  return "librandompack"
end

function _try_preload_openblas()
  try
    Base.require(@__MODULE__, :OpenBLAS32_jll)
    ob = getfield(@__MODULE__, :OpenBLAS32_jll)
    Libdl.dlopen(getfield(ob, :libopenblas_path), Libdl.RTLD_GLOBAL)
  catch
    # Optional dep not present (or loader error); fall back to system resolution.
  end
end

function __init__()
  _libpath[] = _choose_libpath()
  if Sys.islinux()
    try
      Libdl.dlopen("libm.so.6", Libdl.RTLD_GLOBAL)
    catch
    end
  end
  _try_preload_openblas()
  flags = Libdl.RTLD_GLOBAL
  if Sys.islinux()
    flags = flags | Libdl.RTLD_DEEPBIND
  end
  Libdl.dlopen(_libpath[], flags)
end
