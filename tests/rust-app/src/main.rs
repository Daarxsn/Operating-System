#![no_std]
#![no_main]

use xyris_sdk::write;

fn main() {
    let msg = b"XyrisOS 7.5 Rust SDK application OK\n";
    let _ = write(1, msg);
}

xyris_sdk::xyris_entry!(main);
xyris_sdk::xyris_panic_handler!();
