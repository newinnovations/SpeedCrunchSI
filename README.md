# SpeedCrunch/SI

## About this fork
This fork adds SI (metric) prefixes to SpeedCrunch. You can enter numbers like 2.5k and 3.369n for 2.5e3 and 3.369e-9 respectively. Also the form of 2k5 and 3n369 may  be used. There is a corresponding engineering format that shows SI prefixes instead of exponents. Eg:

    44k1512
    = 22.5792M

There are some minor modifications like suppression of e0 in engineering format and buttons for decimal precision on the status bar.

The unit mil (used in pcb design) was added to the conversions.

    5 mil
    = 127µ meter


## Required Qt5 packages for building

    sudo apt install qtbase5-dev qtbase5-dev-tools qttools5-dev qttools5-dev-tools


## About SpeedCrunch
SpeedCrunch is a high-precision scientific calculator. It features a syntax-highlighted
scrollable display and is designed to be fully used via keyboard. Some distinctive
features are auto-completion of functions and variables, a formula book, and quick
insertion of constants from various fields of knowledge. It is available for Windows, OS X,
and Linux in a number of languages.

![capture.png](https://bitbucket.org/repo/dR7BnG/images/3654665019-capture.png)

## Building
To build SpeedCrunch, you need:

- A C++11-capable compiler (e.g. Microsoft Visual C++ 2013 or later, GCC 4.8 or later)
- [Qt](http://qt.io) 5.2 or later
- [CMake](http://cmake.org) 2.8.12 or later

To build SpeedCrunch in a dedicated build directory and install it, run the following
commands from the root of the source directory:

    mkdir build
    cd build
    cmake ../src
    make install

When building against a Qt version that is not the system default Qt installation,
point CMake towards the Qt installation to use by setting the `CMAKE_PREFIX_PATH`
environment variable to the prefix directory of the Qt installation when running CMake.

You can customize the build using the following variables. These are specified when
running CMake, in the form `cmake ../src -Dvariable=value`.

- **PORTABLE_SPEEDCRUNCH**: Set this to `on` to have the application settings stored
  in the same location as the executable, e.g. for running from a USB drive without
  requiring installation.
- **CMAKE_INSTALL_PREFIX**: Change the installation prefix for SpeedCrunch.
- **REBUILD_MANUAL**: Set to `on` to also rebuild the included manual. By default,
  a bundled prebuilt copy is used to minimize dependencies. Rebuilding the manual
  requires the following additional software:
    - [Python](http://python.org) 3.4 or later
    - [Sphinx](http://sphinx-doc.org) 1.3 or later
    - [the Quark theme](https://pypi.python.org/pypi/quark-sphinx-theme) 0.2 or later

## Contributing
- Report bugs or request features in the
  [issue tracker](https://bitbucket.org/heldercorreia/speedcrunch/issues).
- Add or improve a [translation](https://www.transifex.com/projects/p/speedcrunch/).
- Send a message to the [forum](https://groups.google.com/group/speedcrunch).
- Follow the news on the [blog](http://speedcrunch.blogspot.com).

## License
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
