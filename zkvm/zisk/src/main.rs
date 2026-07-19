//! ZisK entrypoint for the evm-sail stateless validator.

#![no_main]

use ere_platform_zisk::ziskos;

ziskos::entrypoint!(main);

unsafe extern "C" {
    fn zkvm_start() -> core::ffi::c_int;
    #[cfg(evmsail_debug)]
    fn evmsail_zisk_report_debug();
}

fn main() {
    let status = unsafe { zkvm_start() };
    #[cfg(evmsail_debug)]
    unsafe {
        evmsail_zisk_report_debug();
    }
    assert_eq!(status, 0, "evm-sail guest returned non-zero status");
}
