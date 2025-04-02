# Montemp

[![Rust](https://img.shields.io/badge/Rust-1.70+-blue)](https://www.rust-lang.org)
[![Linux](https://img.shields.io/badge/Linux-Kernel_Module-green)](https://www.kernel.org)

Rust utility with C kernel driver for temperature monitoring.

## Quick Start

```bash
# Clone and build
git clone https://github.com/ANCLAW-KANE/montemp.git
cd montemp && cargo build --release

# Build and load driver
cd driver && make
sudo insmod montemp.ko
