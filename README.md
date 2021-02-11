# hello-nx
A simple GUI application printing "Hello World!" using [nanovg-deko3d](https://github.com/eXhumer/nanovg-deko3d), a port of nanovg powered by [deko3d](https://github.com/devkitPro/deko3d).

## Building
Dependent libraries in `libs/` must be built before building application.

The following packages are required for building:
* `devkitA64`
* `libnx`
* `deko3d`
* `uam`
* `general-tools`
* `switch-tools`

After installing required packages, run the following command to build application:
> `make -j`
