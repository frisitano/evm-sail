// TEMPLATE — ere guest entry. Untested here (needs the zkVM SDK). The C bridge
// (ere_bridge.c) does the work: read_input -> run the Sail validator -> commit
// the SSZ result. The validation outcome is public-output byte 32.
#![no_std]
#![no_main]

extern "C" { fn evmsail_validate(); }

// SDK-specific entrypoint, e.g. openvm::entry!(main) or zisk's equivalent.
// #[<sdk>::entry]
fn main() {
    unsafe { evmsail_validate() }
}
