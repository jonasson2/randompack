program TimeDistFortran
  use, intrinsic :: iso_fortran_env
  use randompack
  implicit none

  type(randompack_rng) :: rngd, rngf
  double precision, allocatable :: xd(:)
  real, allocatable :: xf(:)
  character(len=64) :: engine
  integer :: chunk, seed
  double precision :: bench_time, warm_time
  logical :: bitexact, help, ok

  double precision, parameter :: d0 = 0
  double precision, parameter :: d1 = 1
  double precision, parameter :: d2 = 2
  double precision, parameter :: d3 = 3
  double precision, parameter :: d5 = 5
  double precision, parameter :: d10 = 10
  double precision, parameter :: d05 = 0.5d0

  call get_options(engine, bench_time, chunk, seed, bitexact, help, ok)
  if (help) then
    call print_help()
    goto 999
  end if
  if (.not.ok) then
    call print_help()
    stop 1
  end if

  call rngd%create(trim(engine), bitexact)
  call rngf%create(trim(engine), bitexact)
  call rngd%seed(seed)
  call rngf%seed(seed)

  allocate(xd(chunk))
  allocate(xf(chunk))

  warm_time = warmup(0.1d0)

  write(*,'(A)') "engine:           " // trim(engine)
  write(*,'(A)') "time per value:   ns/value"
  write(*,'(A)') "bench_time:       " // trim(adjustl(f8_3(bench_time))) // " s per distribution"
  write(*,'(A,I0)') "chunk:            ", chunk
  write(*,'(A)') "warmup:           " // trim(adjustl(f8_3(warm_time))) // " s"
  write(*,'(A)') "bitexact:         " // merge("on ", "off", bitexact)
  write(*,*)
  write(*,'(A,T16,A,T25,A)') "Distribution", "double", "float"

  call time_case("u01",            d0, d0, 1)
  call time_case("unif(2,5)",      d2, d5, 2)
  call time_case("norm",           d0, d0, 3)
  call time_case("normal(2,3)",    d2, d3, 4)
  call time_case("exp(1)",         d1, d0, 5)
  call time_case("exp(2)",         d2, d0, 6)
  call time_case("lognormal(0,1)", d0, d1, 7)
  call time_case("gumbel(0,1)",    d0, d1, 8)
  call time_case("pareto(1,2)",    d1, d2, 9)
  call time_case("gamma(2,3)",     d2, d3, 10)
  call time_case("gamma(0.5,2)",   d05, d2, 11)
  call time_case("beta(2,5)",      d2, d5, 12)
  call time_case("chi2(5)",        d5, d0, 13)
  call time_case("t(10)",          d10, d0, 14)
  call time_case("F(5,10)",        d5, d10, 15)
  call time_case("weibull(2,3)",   d2, d3, 16)

999 continue

contains

  subroutine print_help()
    write(*,'(A)') "TimeDistFortran — time distributions (ns/value), double and float"
    write(*,'(A)') "Usage: TimeDistFortran [options]"
    write(*,*)
    write(*,'(A)') "Options:"
    write(*,'(A)') "  -h            Show this help message"
    write(*,'(A)') "  -e engine     RNG engine (default x256++simd)"
    write(*,'(A)') "  -t seconds    Benchmark time per distribution (default 0.1)"
    write(*,'(A)') "  -c chunk      Chunk size (values per call, default 4096)"
    write(*,'(A)') "  -s seed       RNG seed (default 7)"
    write(*,'(A)') "  -b            Use bitexact log/exp implementations"
  end subroutine

  subroutine get_options(engine, bench_time, chunk, seed, bitexact, help, ok)
    character(len=*), intent(out) :: engine
    double precision, intent(out) :: bench_time
    integer, intent(out) :: chunk, seed
    logical, intent(out) :: bitexact, help, ok
    integer :: argc, i, ios
    character(len=128) :: arg
    character(len=128) :: val
    engine = "x256++simd"
    bench_time = 0.1d0
    chunk = 4096
    seed = 7
    bitexact = .false.
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
          if (ios /= 0) then
            ok = .false.
            return
          end if
          if (bench_time <= 0) then
            ok = .false.
            return
          end if
        case ("-c")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          read(val, *, iostat=ios) chunk
          if (ios /= 0) then
            ok = .false.
            return
          end if
          if (chunk <= 0) then
            ok = .false.
            return
          end if
        case ("-s")
          if (i == argc) then
            ok = .false.
            return
          end if
          i = i + 1
          call get_command_argument(i, val)
          read(val, *, iostat=ios) seed
          if (ios /= 0) then
            ok = .false.
            return
          end if
        case ("-b")
          bitexact = .true.
        case default
          ok = .false.
          return
      end select
      i = i + 1
    end do
    return
  end subroutine

  function warmup(min_seconds) result(seconds)
    double precision, intent(in) :: min_seconds
    double precision :: seconds, t0, t1
    call cpu_time(t0)
    do
      call rngd%unif(xd)
      call rngf%unif(xf)
      call cpu_time(t1)
      seconds = t1 - t0
      if (seconds >= min_seconds) exit
    end do
  end function

  function f8_3(x) result(s)
    double precision, intent(in) :: x
    character(len=16) :: s
    write(s,'(F8.3)') x
  end function

  subroutine time_case(name, p1, p2, idx)
    character(len=*), intent(in) :: name
    double precision, intent(in) :: p1, p2
    integer, intent(in) :: idx
    double precision :: nsd, nsf
    nsd = time_fill_d(idx, p1, p2)
    nsf = time_fill_f(idx, p1, p2)
    write(*,'(A,T14,2F8.2)') trim(adjustl(name)), nsd, nsf
  end subroutine

  function time_fill_d(idx, p1, p2) result(ns)
    integer, intent(in) :: idx
    double precision, intent(in) :: p1, p2
    double precision :: ns, elapsed, t0, t1
    integer :: ncall
    call fill_d(idx, p1, p2)
    call cpu_time(t0)
    ncall = 0
    do
      call fill_d(idx, p1, p2)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0d9/dble(ncall*chunk)
  end function

  function time_fill_f(idx, p1, p2) result(ns)
    integer, intent(in) :: idx
    double precision, intent(in) :: p1, p2
    double precision :: ns, elapsed, t0, t1
    integer :: ncall
    call fill_f(idx, p1, p2)
    call cpu_time(t0)
    ncall = 0
    do
      call fill_f(idx, p1, p2)
      ncall = ncall + 1
      call cpu_time(t1)
      elapsed = t1 - t0
      if (elapsed >= bench_time) exit
    end do
    ns = elapsed*1.0d9/dble(ncall*chunk)
  end function

  subroutine fill_d(idx, p1, p2)
    integer, intent(in) :: idx
    double precision, intent(in) :: p1, p2
    select case (idx)
      case (1)
        call rngd%unif(xd)
      case (2)
        call rngd%unif(xd, p1, p2)
      case (3)
        call rngd%normal(xd)
      case (4)
        call rngd%normal(xd, p1, p2)
      case (5)
        call rngd%exp(xd, p1)
      case (6)
        call rngd%exp(xd, p1)
      case (7)
        call rngd%lognormal(xd, p1, p2)
      case (8)
        call rngd%gumbel(xd, p1, p2)
      case (9)
        call rngd%pareto(xd, p1, p2)
      case (10)
        call rngd%gamma(xd, p1, p2)
      case (11)
        call rngd%gamma(xd, p1, p2)
      case (12)
        call rngd%beta(xd, p1, p2)
      case (13)
        call rngd%chi2(xd, p1)
      case (14)
        call rngd%t(xd, p1)
      case (15)
        call rngd%f(xd, p1, p2)
      case (16)
        call rngd%weibull(xd, p1, p2)
    end select
  end subroutine

  subroutine fill_f(idx, p1, p2)
    integer, intent(in) :: idx
    double precision, intent(in) :: p1, p2
    select case (idx)
      case (1)
        call rngf%unif(xf)
      case (2)
        call rngf%unif(xf, real(p1), real(p2))
      case (3)
        call rngf%normal(xf)
      case (4)
        call rngf%normal(xf, real(p1), real(p2))
      case (5)
        call rngf%exp(xf, real(p1))
      case (6)
        call rngf%exp(xf, real(p1))
      case (7)
        call rngf%lognormal(xf, real(p1), real(p2))
      case (8)
        call rngf%gumbel(xf, real(p1), real(p2))
      case (9)
        call rngf%pareto(xf, real(p1), real(p2))
      case (10)
        call rngf%gamma(xf, real(p1), real(p2))
      case (11)
        call rngf%gamma(xf, real(p1), real(p2))
      case (12)
        call rngf%beta(xf, real(p1), real(p2))
      case (13)
        call rngf%chi2(xf, real(p1))
      case (14)
        call rngf%t(xf, real(p1))
      case (15)
        call rngf%f(xf, real(p1), real(p2))
      case (16)
        call rngf%weibull(xf, real(p1), real(p2))
    end select
  end subroutine

end program TimeDistFortran
