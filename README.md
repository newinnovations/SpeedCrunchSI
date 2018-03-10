# SpeedCrunch/SI

SpeedCrunch is a high-precision scientific calculator. It features a syntax-highlighted
scrollable display and is designed to be fully used via keyboard. Some distinctive
features are auto-completion of functions and variables, a formula book, and quick
insertion of constants from various fields of knowledge. It is available for Windows, OS X,
and Linux in a number of languages.

## About this fork
This fork adds SI (metric) prefixes to SpeedCrunch. You can enter numbers like 2.5k and 3.369n for 2.5e3 and 3.369e-9 respectively. Also the form of 2k5 and 3n369 may  be used. There is a corresponding engineering format that shows SI prefixes instead of exponents. For example:

    44k1 × 512
    = 22.5792M

There are some minor modifications like suppression of e0 in engineering format and buttons for decimal precision on the status bar.

The unit mil (used in pcb design) was added to the conversions.

    5 mil
    = 127µ meter


## Using units and variables

To distinguish between metric prefixes and variables use a space separator.

    a=5
    = 5

    5a
    = 5e−18

    5 a
    = 25

The same with units

    10u meter in inch
    = 393.70078740157480315µ inch

## Required Qt5 packages for building

    sudo apt install qtbase5-dev qtbase5-dev-tools qttools5-dev qttools5-dev-tools

## Building

    mkdir build
    cd build
    cmake ../src
    make

## About SpeedCrunch

Read more on: http://speedcrunch.org/
