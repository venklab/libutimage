# libutimage

A shared dynamic library to speed up few image process sub routines in Python.


- resize boolean mask
- change pixel intensity
- highlight mask border
- correct gamma
- measure distance between irregular shaped masks


## Build

First, go to src, build the shared dynamic library with:

    make

Tested on Debian 10/11, CentOS 7 and AlmaLinux 8.


## Usage

Create a symbolic link to `utimage` in the root directory of Python application,
then `import utimage`.


