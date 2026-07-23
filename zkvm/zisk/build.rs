use std::{env, path::PathBuf};

fn main() {
    let archive_dir = env::var_os("EVMSAIL_ZISK_LIB_DIR")
        .map(PathBuf::from)
        .expect("EVMSAIL_ZISK_LIB_DIR must name the directory containing libevmsail_zisk.a");
    let archive = archive_dir.join("libevmsail_zisk.a");

    println!("cargo:rustc-link-search=native={}", archive_dir.display());
    println!("cargo:rustc-link-lib=static=evmsail_zisk");
    println!("cargo:rerun-if-changed={}", archive.display());
    println!("cargo:rerun-if-env-changed=EVMSAIL_ZISK_LIB_DIR");
    println!("cargo:rerun-if-env-changed=EVMSAIL_ZISK_DEBUG");
    println!("cargo:rustc-check-cfg=cfg(evmsail_debug)");
    if env::var_os("EVMSAIL_ZISK_DEBUG").as_deref() == Some("on".as_ref()) {
        println!("cargo:rustc-cfg=evmsail_debug");
    }
}
