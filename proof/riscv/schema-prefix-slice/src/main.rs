#![no_std]
#![no_main]

use core::panic::PanicInfo;

const STATELESS_INPUT_FIXED_HEADER_LEN: usize = 18;
const STATELESS_INPUT_FIELD_OFFSET0: u32 = 16;
const PUBLIC_KEY_BYTES: usize = 65;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[no_mangle]
static PROOF_INPUT: [u8; STATELESS_INPUT_FIXED_HEADER_LEN] = [
    0x00, 0x01, // big-endian schema id 1
    0x10, 0x00, 0x00, 0x00, // new_payload_request offset
    0x10, 0x00, 0x00, 0x00, // witness offset
    0x10, 0x00, 0x00, 0x00, // chain_config offset
    0x10, 0x00, 0x00, 0x00, // public_keys offset
];

#[no_mangle]
static mut PROOF_OUTPUT: u32 = 0;

#[no_mangle]
pub extern "C" fn _start() -> ! {
    let ok = proof_input_header_well_formed(PROOF_INPUT.as_ptr(), PROOF_INPUT.len());

    unsafe {
        core::ptr::addr_of_mut!(PROOF_OUTPUT).write_volatile(if ok { 1 } else { 0 });
    }

    loop {
        core::hint::spin_loop();
    }
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn proof_schema_prefix_ok(input: *const u8, len: usize) -> bool {
    if len < 2 || input.is_null() {
        return false;
    }

    let byte0 = unsafe { input.read() };
    let byte1 = unsafe { input.add(1).read() };

    byte0 == 0x00 && byte1 == 0x01
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn proof_input_header_well_formed(input: *const u8, len: usize) -> bool {
    if len < STATELESS_INPUT_FIXED_HEADER_LEN || input.is_null() {
        return false;
    }

    if !proof_schema_prefix_ok(input, len) {
        return false;
    }

    let o0 = unsafe { read_u32le(input, 2) };
    let o1 = unsafe { read_u32le(input, 6) };
    let o2 = unsafe { read_u32le(input, 10) };
    let o3 = unsafe { read_u32le(input, 14) };

    let public_keys_start = 2 + o3 as usize;
    o0 == STATELESS_INPUT_FIELD_OFFSET0
        && o0 <= o1
        && o1 <= o2
        && o2 <= o3
        && public_keys_start <= len
        && (len - public_keys_start) % PUBLIC_KEY_BYTES == 0
}

unsafe fn read_u32le(input: *const u8, offset: usize) -> u32 {
    let b0 = input.add(offset).read() as u32;
    let b1 = input.add(offset + 1).read() as u32;
    let b2 = input.add(offset + 2).read() as u32;
    let b3 = input.add(offset + 3).read() as u32;
    b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
}
