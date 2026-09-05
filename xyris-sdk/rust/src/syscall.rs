#![allow(dead_code)]

pub type XyrisResult<T> = core::result::Result<T, XyrisError>;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum XyrisError {
    InvalidArgument,
    NotFound,
    PermissionDenied,
    Busy,
    OutOfMemory,
    Io,
    Unsupported,
    Unknown(i64),
}

impl XyrisError {
    fn from_ret(ret: i64) -> XyrisResult<i64> {
        if ret >= 0 { Ok(ret) } else {
            Err(match ret {
                -22 => Self::InvalidArgument,
                -2 => Self::NotFound,
                -13 => Self::PermissionDenied,
                -16 => Self::Busy,
                -12 => Self::OutOfMemory,
                -5 => Self::Io,
                -95 => Self::Unsupported,
                code => Self::Unknown(code),
            })
        }
    }
}

#[inline(always)]
unsafe fn syscall0(n: usize) -> i64 {
    let ret: i64;
    core::arch::asm!("int 0x80", inlateout("rax") n as i64 => ret, options(nostack, preserves_flags));
    ret
}

#[inline(always)]
unsafe fn syscall1(n: usize, a1: usize) -> i64 {
    let ret: i64;
    core::arch::asm!("int 0x80", inlateout("rax") n as i64 => ret, in("rdi") a1, options(nostack, preserves_flags));
    ret
}

#[inline(always)]
unsafe fn syscall3(n: usize, a1: usize, a2: usize, a3: usize) -> i64 {
    let ret: i64;
    core::arch::asm!("int 0x80", inlateout("rax") n as i64 => ret, in("rdi") a1, in("rsi") a2, in("rdx") a3, options(nostack, preserves_flags));
    ret
}

pub fn write(fd: usize, buf: &[u8]) -> XyrisResult<usize> {
    XyrisError::from_ret(unsafe { syscall3(1, fd, buf.as_ptr() as usize, buf.len()) }).map(|v| v as usize)
}

pub fn read(fd: usize, buf: &mut [u8]) -> XyrisResult<usize> {
    XyrisError::from_ret(unsafe { syscall3(0, fd, buf.as_mut_ptr() as usize, buf.len()) }).map(|v| v as usize)
}

pub fn open(path: *const u8, flags: usize) -> XyrisResult<usize> {
    XyrisError::from_ret(unsafe { syscall1(2, path as usize) }).map(|v| { let _ = flags; v as usize })
}

pub fn close(fd: usize) -> XyrisResult<usize> {
    XyrisError::from_ret(unsafe { syscall1(3, fd) }).map(|v| v as usize)
}

pub fn getpid() -> XyrisResult<usize> {
    XyrisError::from_ret(unsafe { syscall0(39) }).map(|v| v as usize)
}

pub fn exit(code: i32) -> ! {
    unsafe { syscall1(60, code as usize); }
    loop { core::hint::spin_loop(); }
}
