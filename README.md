# BGPGrep

Simple GitHub clone from https://gitlab.com/AlphaCogs/ubgpsuite.git. Presentation about BGPgrep features and use cases is available [here](bgp_scanner.pdf)

Original README follows...


# Micro BGP Suite

The micro BGP suite (`ubgpsuite` or `μbgpsuite`) is a performance oriented
toolkit to access, analyze, filter and write BGP and MRT data.

For developers, `ubgpsuite` provides various low level C libraries to access,
BGP and MRT data, as well as a dynamic and extensible filtering engine to
analyze the data in interesting ways.

For network analysts, `ubgpsuite` makes available a number of tools to ease
network analysis and diagnostics.
Every tool is implemented using the libraries themselves and, in fact,
can be used by a developer as usage reference for them.

This suite is based on the BSD licensed work from the [Isolario Project](https://isolario.it),
in particular [bgpscanner](https://gitlab.com/Isolario/bgpscanner) and [isocore](https://gitlab.com/Isolario/isocore).

## Contacts

An IRC channel is available for support and development discussions regarding
`ubgpsuite` at `#ubgpsuite` on [freenode](https://freenode.net/).

You can join with any IRC client or using a webclient:
[![IRC channel](https://kiwiirc.com/buttons/chat.freenode.net:6667/ubgpsuite.png)](https://kiwiirc.com/client/chat.freenode.net:6667/?nick=guest|?&theme=cli#ubgpsuite)

## Building

`ubgpsuite` uses [Meson](https://mesonbuild.com) to manage the build process.

The basic steps for configuring and building BGP Scanner look like this:

```bash
$ git clone https://gitlab.com/AlphaCogs/ubgpsuite.git
$ cd ubgpsuite
$ meson build
$ cd build && ninja
```

In case you want to build with the *release* configuration, you have to
enable the *release* build type, like this:

```bash
$ git clone https://gitlab.com/AlphaCogs/ubgpsuite.git
$ cd ubgpsuite
$ meson --buildtype=release build
$ cd build && ninja
```

Or run the following inside the build directory:

```bash
$ meson configure -Dbuildtype=release
$ ninja
```

## Installation Guide

For Ubuntu and Debian Based System

First make sure you have git and meson installed (if not please install them):

```bash
$ sudo apt install git meson
```

Make sure to install the necessary dependencies:

```bash
$ sudo apt install zlib1g-dev libbz2-dev liblzma-dev liblz4-dev
```

Now, let's clone the repository and build it:

```bash
$ git clone https://gitlab.com/AlphaCogs/ubgpsuite.git
$ cd ubgpsuite
$ meson --buildtype=release build
$ cd build && ninja
```

If you need to install `ubgpsuite` globally:

```bash
$ sudo ninja install
```

After the installation phase, you may need to update
the linker cache, to do that run the following command:

```bash
$ sudo ldconfig
```

## Why a different project?

`ubgpsuite` is based on code and previous tools from the [Isolario Project](https://isolario.it),
in particular [bgpscanner](https://gitlab.com/Isolario/bgpscanner) and [isocore](https://gitlab.com/Isolario/isocore),
in fact, the `bgpgrep` utility has 100% command line compatibility with Isolario's `bgpscanner`.

The main difference between the projects is that `ubgpsuite` is meant to be a stand-alone project.
Having complete API freedom and no constraint leaves more freedom to experiment
and develop new strategies not necessarily backward compatible with pre-existing code.

Each tool and library in `ubgpsuite` strives to be minimal and self-contained,
in order to be flexible and usable in different contexts.

