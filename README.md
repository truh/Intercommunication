Tiva Template
==================

## Toolchain

### Dependencies:

* flex
* bison
* libgmp3-dev
* libmpfr-dev
* libncurses5-dev
* libmpc-dev
* autoconf
* texinfo
* libftdi-dev
* python-yaml
* zlib1g-dev
* cutecom
* gcc-multilib

To get all dependencies on Debian:

    apt-get install flex bison libgmp3-dev libmpfr-dev libncurses5-dev \
    libmpc-dev autoconf texinfo build-essential libftdi-dev python-yaml \
    zlib1g-dev gcc-multilib cutecom

You will need an ARM bare-metal toolchain to build code for Tiva targets.
You can get a toolchain from the
[gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded) project that is
pre-built for your platform. Extract the package and add the `bin` folder to
your PATH.

The TivaWare package contains all of the header files and drivers for
Tiva parts. Grab the file *SW-TM4C-2.0.1.11577.exe* from
[here](http://software-dl.ti.com/tiva-c/SW-TM4C/latest/index_FDS.html) and unzip it into a directory
then run `make` to build TivaWare.

    mkdir -p ~/opt/tivaware
    cd ~/opt/tivaware
    unzip SW-TM4C-2.0.1.11577.exe
    make

## Writing and Building Firmware

1. Clone the
   [tiva-template](https://github.com/uctools/tiva-template)
   repository (or fork it and clone your own repository).

	git clone git@github.com:mborko/tiva-template

2. Modify the Makefile:
    * Set TARGET to the desired name of the output file (eg: TARGET = main)
    * Set SOURCES to a list of your sources (eg: SOURCES = main.c
      startup\_gcc.c)
    * Set TIVAWARE\_PATH to the full path to where you extracted and built
      TivaWare (eg: TIVAWARE_PATH = $(HOME)/opt/tivaware)

3. Run `make`

4. The output files will be created in the 'build' folder

## Flashing

Add the Launchpad to the udev rules to be able to access it as a non-root user. Edit a rule-file and add the device id:

    vim /etc/udev/rules.d/80-embedded-devices.rules
    # TI Launchpad TM4C123GXL
    SUBSYSTEM=="usb", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", MODE="0666", SYMLINK+="lm4f", GROUP="dialout"

The easiest way to flash your device is using lm4flash. First, grab lm4tools
from Git.

    cd ~/opt
    git clone git://github.com/utzig/lm4tools.git

Then build lm4flash and run it:

    cd lm4tools/lm4flash
    make
    lm4flash /path/to/executable.bin

For easier usage add the lm4flash to your PATH variable.

## Credits

Thanks to Recursive Labs for their
[guide](http://recursive-labs.com/blog/2012/10/28/stellaris-launchpad-gnu-linux-getting-started/)
which this template is based on.
