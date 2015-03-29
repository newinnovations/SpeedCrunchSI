# SpeedCrunch/SI

## About this fork
This fork adds SI prefixes to SpeedCrunch. You can enter numbers like 2.5k and 3.369n for 2.5e3 and 3.369e-9 respectively. Now the form of 2k5 and 3n369 may also be used. There is a corresponding engineering format that shows SI prefixes instead of exponents. Eg: 44k1\*512 = 22.5792M

There are some minor modifications like suppression of e0 in engineering format and buttons for decimal precision on the status bar. 

## About SpeedCrunch
SpeedCrunch is a high-precision scientific calculator. It features a syntax-highlighted scrollable display and is designed to be fully used via keyboard. Some distinctive features are auto-completion of functions and variables, a formula book, and quick insertion of constants from various fields of knowledge. Available for Windows, OS X, and Linux in a number of languages.

## Building
To build from source, use either CMake or QMake to build the project:

    cd src && cmake . && make

or

    cd src && qmake speedcrunch.pro && make
    
In order to have your application settings stored in the same location as
the executable for portability (e.g. running from a USB drive or a directory
without requiring installation and special permissions), configure the project
as below:

    cmake -DCMAKE_CXX_FLAGS:STRING="-DSPEEDCRUNCH_PORTABLE"

or

    qmake "DEFINES+=SPEEDCRUNCH_PORTABLE" speedcrunch.pro

## License
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
