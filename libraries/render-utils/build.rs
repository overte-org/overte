#[allow(unused_must_use)]
fn main() {
    cxx_build::bridge("src/render-utils.rs");
    println!("cargo:rerun-if-changed=src/render-utils.rs");
}
