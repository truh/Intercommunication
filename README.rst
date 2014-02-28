#############
Tiva Template
#############

=========
Toolchain
=========

~~~~~~~~~~~~~
Dependencies:
~~~~~~~~~~~~~

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
* pkg-config
* gcc-multilib

To get all dependencies on Debian:

    apt-get install flex bison libgmp3-dev libmpfr-dev libncurses5-dev \
    libmpc-dev autoconf texinfo build-essential libftdi-dev python-yaml \
    zlib1g-dev gcc-multilib pkg-config cutecom

You will need an ARM bare-metal toolchain to build code for Tiva targets.
You can get a toolchain from the
`gcc-arm-embedded <https://launchpad.net/gcc-arm-embedded>`_ project that is
pre-built for your platform. Extract the package and add the `bin` folder to
your PATH.

The TivaWare package contains all of the header files and drivers for
Tiva parts. Grab the file *SW-TM4C-2.0.1.11577.exe* from
`here <http://software-dl.ti.com/tiva-c/SW-TM4C/latest/index_FDS.html>`_ and unzip it into a directory
then run `make` to build TivaWare.

    mkdir -p ~/opt/tivaware
    cd ~/opt/tivaware
    unzip SW-TM4C-2.0.1.11577.exe
    make

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writing and Building Firmware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Clone the
   `tiva-template <https://github.com/uctools/tiva-template>`_
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

========
Flashing
========

Add the Launchpad to the udev rules to be able to access it as a non-root user. Edit a rule-file and add the device id:

    vim /etc/udev/rules.d/80-embedded-devices.rules
    # TI Launchpad TM4C123GXL
    SUBSYSTEM=="usb", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", MODE="0666", SYMLINK+="lm4f", GROUP="dialout"

Don't forget to add your user to the defined group in /etc/group. Afterwards you should restart the udev system.

The easiest way to flash your device is using lm4flash. First, grab lm4tools
from Git.

    cd ~/opt
    git clone https://github.com/utzig/lm4tools.git

Then build lm4flash and run it:

    cd lm4tools/lm4flash
    make
    lm4flash /path/to/executable.bin

For easier usage add the lm4flash to your PATH variable:

    export PATH="$HOME/opt/lm4flash:$PATH"

=========
Debugging
=========

To begin debugging just run: 

.. code:: bash

    make debug

This will also rebuild the project. If you want to debug without rebuilding
run:

.. code:: bash

    debug/debug_nemiver.sh

~~~~~~~~~~~~~
Used Programs
~~~~~~~~~~~~~

-----------------
arm-none-eabi-gdb
-----------------

Should already be installed with the 
`gcc-arm-embedded <https://launchpad.net/gcc-arm-embedded>`_ toolchain

**Configuration [debug/run.gdb]**

.. code:: text

    target remote localhost:3333
    monitor reset halt
    thbreak main
    monitor reset init

-------
OpenOCD
-------

Open On-Chip Debugger

http://openocd.sourceforge.net

Can be installed on Debian with:

.. code:: bash
    
    sudo apt-get install openocd

**Configuration [debug/debug.cfg]**

.. code:: text

    #daemon configuration
    telnet_port 4444
    gdb_port 3333

    #board specific
    source [find interface/ti-icdi.cfg]

    set WORKAREASIZE 0x8000
    set CHIPNAME tm4c123gh6pm
    source [find target/stellaris_icdi.cfg]

-------
Nemiver
-------

Graphical interface for Gnu Debugger.

https://wiki.gnome.org/Apps/Nemiver

Can be installed on Debian with:

.. code:: bash
    
    sudo apt-get install nemiver

**Runscript [debug/debug_nemiver.sh]**

.. code:: bash

    #!/bin/bash

    # start xterm with openocd in the background
    xterm -e openocd -f ./debug/debug.cfg &

    # save the PID of the background process
    XTERM_PID=$!

    # wait a bit to be sure the hardware is ready
    sleep 2

    # execute some initialisation commands via gdb
    arm-none-eabi-gdb --batch --command=./debug/run.gdb build/$1

    # start the gdb gui
    nemiver --remote=localhost:3333 --gdb-binary="$(which arm-none-eabi-gdb)" build/main

    # close xterm when the user has exited nemiver
    kill $XTERM_PID

=======
Credits
=======

Thanks to Recursive Labs for their
`guide <http://recursive-labs.com/blog/2012/10/28/stellaris-launchpad-gnu-linux-getting-started/>`_
which this template is based on.

The debugging code and instructions are based on:
`Getting started with stellaris launchpad 
<http://www.jann.cc/2012/12/11/getting_started_with_the_ti_stellaris_launchpad_on_linux.html>`_.