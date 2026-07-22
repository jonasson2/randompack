program TimeDistGfortran
  use, intrinsic :: iso_fortran_env, only: real64, compiler_version, compiler_options
  use randompack
  implicit none

  type(randompack_rng) :: rng
  real(real64), allocatable :: xd(:)
  real, allocatable :: xf(:)
  character(len=64) :: engine
  integer :: chunk, seed
  real(real64) :: bench_time, warm_time
  logical :: help, ok

  call get_options(engine, bench_time, chunk, seed, help, ok)
  if (help) then
    call print_help()
    goto 999
  end if
  if (.not. ok) then
    call print_help()
    stop 1
  end if

  call rng%create(trim(engine))
  call rng%seed(seed)
  call seed_intrinsic(seed)

  allocate(xd(chunk))
  allocate(xf(chunk))

  warm_time = warmup(0.1_real64)

  write(*,'(A)') "Fortran intrinsic: random_number"
  write(*,'(A)') "Compiler:          " // compiler_version()
  write(*,'(A)') "Compiler options:  " // compiler_options()
  write(*,'(A)') "Randompack engine: " // trim(engine)
  write(*,'(A,I0)') "chunk:             ", chunk
  write(*,'(A,F8.3,A)') "bench_time:        ", bench_time, " s per case"
  write(*,'(A,F8.3,A)') "warmup:            ", warm_time, " s"
  write(*,*)
  write(*,'(A,T18,A,T31,A,T44,A)') "Distribution", "gfortran", "Randompack", "Factor"

  call time_case_double("u01 double")
  call time_case_float("u01 float")

999 continue

contains
  subroutine print_help()
    write(*,'(A)') "TimeDistGfortran - compare gfortran random_number vs Randompack"
    write(*,'(A)') "Usage: TimeDistGfortran [options]"
    write(*,*)
    write(*,'(A)') "Options:"
    write(*,'(A)') "  -h            Show this help message"
    write(*,'(A)') "  -e engine     Randompack engine (default x256++simd)"
    write(*,'(A)') "  -t seconds    Benchmark time per case (default 0.2)"
    write(*,'(A)') "  -c chunk      Chunk size per call (default 4096)"
    write(*,'(A)') "  -s seed       RNG seed (default 7)"
  end subroutine print_help

  subroutine get_options(engine, bench_time, chunk, seed, help, ok)
    character(len=*), intent(out) :: engine
    real(real64), intent(out) :: bench_time
    integer, intent(out) :: chunk, seed
    logical, intent(out) :: help, ok
    integer :: argc, i, ios
    character(len=128) :: arg, val

    engine = "x256++simd"
    bench_time = 0.2_real64
    chunk = 4096
    seed = 7
    help = .false.
    ok = .true.
    argc = command_argument_count()
    i = 1
    do while (i <= argc)
      call get_command_argument(i, arg)
      select case (trim(arg))
        case ("-h")
          help = .true.
          return
        case ("-e")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          engine = trim(val)
        case ("-t")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          read(val, *, iostat=ios) bench_time
          if (ios /= 0 .or. bench_time <= 0.0_real64) ok = .false.
        case ("-c")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          read(val, *, iostat=ios) chunk
          if (ios /= 0 .or. chunk <= 0) ok = .false.
        case ("-s")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          read(val, *, iostat=ios) seed
          if (ios /= 0) ok = .false.
        case default
          ok = .false.
      end select
      if (.not. ok) return
      i = i + 1
    end do
  end subroutine get_options

  subroutine seed_intrinsic(seed)
    integer, intent(in) :: seed
    integer, allocatable :: seedvector(:)
    integer :: n, i
    call random_seed(size=n)
    allocate(seedvector(n))
    seedvector = [(seed + 37*i, i=1,n)]
    call random_seed(put=seedvector)
    deallocate(seedvector)
  end subroutine seed_intrinsic

  function warmup(min_seconds) result(seconds)
    real(real64), intent(in) :: min_seconds
    real(real64) :: seconds, t0, t1
    call cpu_time(t0)
    do
      call random_number(xd)
      call rng%unif(xd)
      call cpu_time(t1)
      seconds = t1 - t0
      if (seconds >= min_seconds) exit
    end do
  end function warmup

  subroutine time_case_double(name)
    character(len=*), intent(in) :: name
    real(real64) :: gf_ns, rp_ns
    call seed_intrinsic(seed)
    gf_ns = time_fill_double_intrinsic()
    call rng%seed(seed)
    rp_ns = time_fill_double_randompack()
    call print_case(name, gf_ns, rp_ns)
  end subroutine time_case_double

  subroutine time_case_float(name)
    character(len=*), intent(in) :: name
    real(real64) :: gf_ns, rp_ns
    call seed_intrinsic(seed)
    gf_ns = time_fill_float_intrinsic()
    call rng%seed(seed)
    rp_ns = time_fill_float_randompack()
    call print_case(name, gf_ns, rp_ns)
  end subroutine time_case_float

  function time_fill_double_intrinsic() result(ns)
    real(real64) :: ns, elapsed, t0, t1
    integer :: ncall
    call random_number(xd)
    call cpu_time(t0)
    ncall = 0
    do
      call random_number(xd)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0e9_real64/(real(ncall, real64)*chunk)
  end function time_fill_double_intrinsic

  function time_fill_float_intrinsic() result(ns)
    real(real64) :: ns, elapsed, t0, t1
    integer :: ncall
    call random_number(xf)
    call cpu_time(t0)
    ncall = 0
    do
      call random_number(xf)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0e9_real64/(real(ncall, real64)*chunk)
  end function time_fill_float_intrinsic

  function time_fill_double_randompack() result(ns)
    real(real64) :: ns, elapsed, t0, t1
    integer :: ncall
    call rng%unif(xd)
    call cpu_time(t0)
    ncall = 0
    do
      call rng%unif(xd)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0e9_real64/(real(ncall, real64)*chunk)
  end function time_fill_double_randompack

  function time_fill_float_randompack() result(ns)
    real(real64) :: ns, elapsed, t0, t1
    integer :: ncall
    call rng%unif(xf)
    call cpu_time(t0)
    ncall = 0
    do
      call rng%unif(xf)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0e9_real64/(real(ncall, real64)*chunk)
  end function time_fill_float_randompack

  subroutine print_case(name, gf_ns, rp_ns)
    character(len=*), intent(in) :: name
    real(real64), intent(in) :: gf_ns, rp_ns
    write(*,'(A,T16,F10.2,T29,F10.2,T42,F8.2)') trim(name), gf_ns, rp_ns, &
      gf_ns/rp_ns
  end subroutine print_case
end program TimeDistGfortran
