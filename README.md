# MAR24 ArUco Detection Program (C++)

## Synopsis

**marvision -- the vision program for the minimal autonomous rover (MAR) project.**

```sh
marvision [OPTIONS]
```

## Installation

This program is designed to be cross-compiled and be run on Raspberry Pi Zero
series. However, it is possible to install on other machines. The build system
of this project is GNU Autotools.

We assume you have `build-essential` and other essentials e.g. `gcc`, `g++`,
`glibc`, `make`, `autotools` installed.

### Build for Host (Linux)

Building for host instead of cross compiling is discouraged and not tested.

1. Ensure you have OpenCV (`opencv` and `opencv_contrib`) installed. The
   repositories can be found: [OpenCV](https://github.com/opencv/opencv) and
   [OpenCV Extras](https://github.com/opencv/opencv_contrib/). Please install
   version 4.9.0. You should consider the revisions below, taken from the [recipe](https://git.openembedded.org/meta-openembedded/tree/meta-oe/recipes-support/opencv/opencv_4.9.0.bb) provided by OpenEmbedded. 
   ```
   SRCREV_opencv = "dad8af6b17f8e60d7b95a1203a1b4d22f56574cf"
   SRCREV_contrib = "c7602a8f74205e44389bd6a4e8d727d32e7e27b4"
   ```  
   Using a wrong version of OpenCV can cause the code fail to compile and waste
   your time/storage.
2. Use Autotools to compile.
   ```sh
   ./configure
   make
   sudo make install
   ```
      
### Cross-Compile

1. Prepare your systems. Use a Debian Linux if possible; other distros may also
   work. 
2. Download our SDK from the [release notes of
   meta-marfb](https://www.southampton.ac.uk/~qs2g22/mar/marfb-release/release-notes.html).
   Choose the latest stable "Standard SDK". 
3. Install the SDK. See the [SDK
   manual](https://docs.yoctoproject.org/sdk-manual/using.html) of the Yocto
   Project.
   ```sh
   ./path/to/sdk.sh
   ```
   Enter path when prompted.
4. Run SDK environment setup script. What to run is prompted on the last few
   lines of the SDK installation output. 
   ```sh
   . /path/to/environment-setup-script
   ```
   After environment setup, you can test by e.g. echo $CC or which autoreconf.
   ```sh
   echo $CC
   arm-oe-linux-gnueabi-gcc -marm -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64 --sysroot=/usr/local/oecore-aarch64/sysroots/arm1176jzfshf-vfp-oe-linux-gnueabi
   which autoreconf
   /usr/local/oecore-aarch64/sysroots/aarch64-oesdk-linux/usr/bin/autoreconf
   ```
5. Autoreconf.
   ```sh
   autoreconf
   ```
6. Cross compile.
   ```sh
   ./configure ${CONFIGURE_FLAGS}
   ```
   E.g.,

    ```sh
    ./configure --host=... --with-libtool-sysroot=...
    ```
7. Make and install.
   ```sh
   make
   make install DESTDIR=./tmp
   ```
8. Copy to target and run.


This program can be configured, built, installed and packaged by
`meta-marfb`--a sister project that builds fast boot Linux for MAR.

## Usage

TODO

## License

`SPDX-License-Identifier: GPL-3.0-or-later`

This project is licensed under **GNU General Public License v3.0 or later**
(`GPL-3.0-or-later`).

Copyright (C) 2024  Qiyang Sun and the MAR Project Maintainers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Acknowledgements

## Sister Projects

- **MAR24 ArUco Detect Python**, the Python version of this program, can be found
  at: https://git.soton.ac.uk/qs2g22/mar24-aruco-py.git
- **Meta MAR Fast Boot**, the Yocto layer that builds the fast boot Linux
  operating system, can be found at: https://git.soton.ac.uk/qs2g22/meta-marfb.git
