# Planetaria 

## Purpose

This project makes use of NOVAS C 3.1 and the NASA JPL DE430 ephemeris to demonstrate how to accomplish tasks such as:

1. find the position (right ascension and declination) of any or all of the major planets; or
1. determine new and full moons during a given time period; or
1. find the rise and set times for a given planet (or the sun or moon) for a given location on the earth.

This is a code sketch that I built to help me learn about things like:

1. Different astronomical time scales (UT1, UTC, TT, TDB). A good overview is here: https://www.stjarnhimlen.se/comp/time.html
1. Right ascension and declination.
1. Equatorial and celestial coordinate systems.
1. The use of root finders to find new and full moons and rise/set events.

## Project Contents

This project contains the following directories:

- `novas-wrapper/NOVAS-C`: contains the NOVAS 3.1 C distribution source (from http://aa.usno.navy.mil/software/novas/novas_c). Patches have been made to the source to resolve the issues described at http://aa.usno.navy.mil/software/novas/novas_faq.php.
- `novas-wrapper/src`: contains the source code for the `novas-wrapper` C++ wrapper library for the NOVAS 3.1 C distribution. (See below.)
- `ephemeris-data`: contains the JPL DE430 ephemeris and a copy of the USNO UT1/UTC delta values (as `finals.data`).
- `planetaria`: a demo application making use of the `novas-wrapper` library.

### The `novas-wrapper` Library

This library contains functions to read and parse the USNO UT1/UTC delta values (as `finals.data`), to convert between different astronomical time scales, to manage the DE430 ephemeris, and to perform basic operations against the ephemeris.

The `src/astro_time` files are probably the most useful portion of this library. They contain all the logic required to convert between different astronomical time scales, encapsulated in a type that can be passed to other functions in this library.

The `src/novas_utils` files contain logic to get planet locations, build planet objects (as defined by the NOVAS C functions), and perform other operations to handle data types from the `src/novas_wrapper` files.

The `src/novas_wrapper` files contain logic to call and interpret the results of the NOVAS C functions. The NOVAS C functions are wrapped in error checking logic and accept C++ types such as `src/astro_time` and references rather than pointers.

### The `planetaria` Demo Application

The `planetaria` directory contains a simple demo application that calls functions from the `src/planet_utils` files and then prints the output to standard out. 

The `src/planet_utils` files contain logic to get current planetary positions, get moon phase events (new and full), and get rise and set times for a planet at a location on Earth. Results are returned in JSON. These files make use of the `novas-wrapper` library functions (from `novas_utils` and `novas_wrapper`).

Calling the executable with the `-h` flag will produce the following output:

```
./planetaria [-h : print this message]
./planetaria [-e ephemeris_location] : pass location of DE 430 Ephemeris. Defaults to './data/jpleph.430'.
./planetaria [-f finals-data-location] : pass location of finals data. Defaults to './data/finals.data.txt'.
./planetaria [-e ephemeris-location] [-f finals-data-location] -c command [parameters]

Commands:

./planetaria -c planets
./planetaria -c planets [-utc datetime]
./planetaria -c planets [-utc datetime] [-all]
./planetaria -c planets [-utc datetime] [-planet planet-name]

./planetaria -c moon_phases
./planetaria -c moon_phases [-utcstart datetime] [-utcend datetime]
                                -utcstart defaults to the start of the current month
                                -utcend defaults to thirty days after utcstart

./planetaria -c rise_set -lat lat -lon lon
./planetaria -c rise_set -lat lat -lon lon [-utcstart datetime] [-utcend datetime] [-planet planet-name]
                                -utcstart defaults to now
                                -utcend defaults to one day after utcstart
                                -planet defaults to sun

Notes:

The 'datetime' values must be UTC, in ISO 8601 format: YYYY-MM, YYYY-MM-DD, or YYYY-MM-DDTHH:MM:SSZ
Latitude and longitude are in signed degrees (decimal) format: 41.25 or -122.95.
Planet name(s) are case-insensitive, and may be one of the following values:
Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto, Sun, Moon
```

## Building

The `CMakeLists.txt` file is configured to build a static library (from `novas-wrapper`) and then link it against the demo application from `planetaria`.

The `planetaria` executable will print its various command line options when run with the `-h` flag.

The project requires a C++ compiler with **C++17** support. (So, `MSVC` or `G++-8`.)

### On Linux

```
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=g++-8 -D CMAKE_C_COMPILER=gcc-8 ..
make
cd planetaria
./planetaria -h
```

### On Windows

```
mkdir build
cd build
cmake -A x64 ..
devenv planetaria.sln /build
cd planetaria\Debug
planetaria.exe -h
```

## Notes

### JSON Output

The sample program emits output as JSON to support use in a NodeJS Express web application. (See https://nodejs.org/api/child_process.html#child_process_child_process_execfile_file_args_options_callback for details.)

### UT1/UTC Conversion Notes

Obtaining accurate conversions between other times and UT1 times (if you care that much, since the difference is at most a single second) depends on the current UT1/UTC delta values that are found in the weekly Earth Orientation Products from USNO:

  https://www.usno.navy.mil/USNO/earth-orientation/eo-products/weekly

Download the `finals.data` file and place it in the `data` location (or pass the location of the `finals.data` file to the `planetaria` demo application using the `-f` flag).

Note that there is currently no support for _not_ having a `finals.data` file.

### JPL DE 430 Ephemeris

This project requires a copy of the JPL DE430 Ephemeris. It is included in the source directory, but can also be obtained directly from NASA:

  ftp://ssd.jpl.nasa.gov/pub/eph/planets/Linux/de430/linux_p1550p2650.430

