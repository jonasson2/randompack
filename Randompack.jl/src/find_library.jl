using Libdl

const _libpath = Ref{String}("librandompack")
const _libhandle = Ref{Ptr{Cvoid}}(C_NULL)

function _dlopen_flags()
  flags = Libdl.RTLD_GLOBAL
  if Sys.islinux()
    flags |= Libdl.RTLD_DEEPBIND
  end
  return flags
end

function _dev_install_lib()::String
  prefix = normpath(joinpath(@__DIR__, "..", "..", "install"))
  if Sys.isapple()
    return joinpath(prefix, "lib", "librandompack.dylib")
  elseif Sys.iswindows()
    return joinpath(prefix, "bin", "librandompack.dll")
  else
    return joinpath(prefix, "lib", "librandompack.so")
  end
end

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
  devlib = _dev_install_lib()
  if isfile(devlib)
    return devlib
  end
  return "librandompack"
end

function __init__()
  _libpath[] = _choose_libpath()
  _libhandle[] = Libdl.dlopen(_libpath[], _dlopen_flags())
end
