#![no_std]

pub mod syscall;

pub use syscall::{close, exit, getpid, open, read, write};

#[macro_export]
macro_rules! xyris_entry {
    ($path:path) => {
        #[no_mangle]
        pub extern "C" fn _start() -> ! {
            let _ = $path();
            $crate::exit(0)
        }
    };
}

#[macro_export]
macro_rules! xyris_panic_handler {
    () => {
        #[panic_handler]
        fn panic(_info: &core::panic::PanicInfo) -> ! {
            $crate::exit(1)
        }
    };
}
