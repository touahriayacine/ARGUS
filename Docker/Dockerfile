FROM ubuntu:24.04

RUN mkdir ARGUS

Copy ./* ./ARGUS/

RUN apt-get update && \ 
    apt-get install -y gcc-avr avr-libc git python3.12 python3.12-venv qemu-system make cmake ninja-build bzip2 glib-2.0 bison flex libgcrypt20-dev libslirp-dev


RUN git clone https://github.com/espressif/qemu.git
workdir qemu
RUN ./configure --disable-werror
RUN make install
workdir ..

RUN git clone https://github.com/espressif/esp-idf.git
Workdir esp-idf/
RUN bash install.sh
RUN . ./export.sh

Workdir ..
RUN python3 -m pip install -r requirements.txt
