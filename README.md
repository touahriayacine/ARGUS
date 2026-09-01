# ARGUS:  data-dRiven instruction-level enerGy estimation and scoring for mcU and iot Software

ARGUS is the first toolkit that exists to estimate the energy consumption and to attribute an energy effeciency score to encryption workloads designed for IoT devices. ARGUS for now, supports two microcontrollers **Arduino Uno AVR-8** and **ESP32 C3 mini RISC-V**. It doesn't require any piece of hardware to perform the estimation due to the instruction tracing fully emulated with QEMU software. QEMU is an open-source project and has a large comunity. ARGUS enables a unified software to capture a fuller and wider energy profile of encryption programs through estimations across multiple MCUs and CPU architectures. For now, ARGUS is tested on Ubuntu Linux operating system only.

## 1. Installation

### Docker installation

<!-- You can launch directly new container from a ready to use docker image where all the dependencies are pre-installed, or create new image from a `Dockerfile`:

1. Using our pre-configured Docker image:

```bash
wget 
``` -->

Building everything from a `Dockerfile`:
```bash
cd ARGUS/Docker/
docker build -t argus_image:latest .
docker run -d --name argus_container argus_image:latest
docker exec -it argus_container bash
cd /esp-idf/
. ./export
cd ../ARGUS
```

To test if everything is set:

```bash
bash test_all.sh # script to test all projects inside the tests/ folder
```

### Manual installation

Clone the repository:

```bash
git clone https://gitlab.irit.fr/siera/argus.git
cd ARGUS/
```

If you want to compile a project you have to install `gcc-avr` and `avr-libc` for the AVR projects and `idf.py` the espressif toolchain utilitify for RISC-V projects

To compile AVR projects, install the `GCC` toolchain along with the `Libc` library:

```bash
sudo apt update
sudo apt install gcc-avr avr-libc make
```

To compile RISC-V projects, install the toolchain utility provided by Espressif in their Github respository. Ensure that `cmake` is installed.

```bash
git clone https://github.com/espressif/esp-idf.git
cd esp-idf/
bash install.sh
. ./export.sh
```

If you don't install the RISC-V toolchain create new virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

Install python dependencies:

```bash
pip install -r requirements.txt
```

Install QEMU for AVR:

```bash
sudo apt install qemu-system
```

The QEMU version installed previousely does support the RISCV32 version but since the ESP32 has a modified RISC-V32 developed by Espressif, we use their QEMU version. Ensure that `Ninja`, `bzip2`, `iasl`, `flex`, `bison`, `glib-2.0`, `libgcrypt20-dev`, and `libslirp-dev` are installed.

```bash
git clone https://github.com/espressif/qemu.git
cd qemu/
./configure --disable-werror
```

## 2. Usage

To test ***ARGUS***, we created the ***tests/*** folder for both MCUs:

```bash
python3 main.py --project-path tests/RISCV/ascon96v1 --mcu esp32c3 # RISCV project
python3 main.py --project-path tests/AVR/tinyjambu128v2 --mcu arduino_uno # AVR project
```
There is always a `help` command to explore the required and optional arguments

```bash
python3 main.py --help
```

We provided a script `test_all.sh` to automate the test of all projects inside the `tests/` folder:

```bash
bash test_all.sh
```

---
## 3. Licence

This repository is licensed under Academic Software Evaluation License, detailed in the LICENSE.txt file.