// Mubaddil Core Build Script
// This file is used by Cargo to configure the build process

fn main() {
    // Rebuild if Python changes
    println!("cargo:rerun-if-changed=../requirements.txt");
    
    // PyO3 configuration
    pyo3_build_config::add_extension_module_link_args();
}
