# The TIDE Project

**NOTE:** This README does not document usage instructions and tips for the
proxy itself, and is intended as a technical overview or the project.

The TIDE Proxy is a native, graphical SOCKS5 proxy. It is built to simualte
macro-scale internet outages. It is currently a work in progress.

TIDE is an acryonym. It stands for **T**elemetry for **I**nternet **D**isruption **E**stimation.

## License

This project is licnsed under the **PolyForm Noncommercial License 1.0.0**.
You Are free to use, modify, and distribute this software for non-commercial, academic, and researched purposes. **Commercial use is strictly prohibited** without prior written permission.
For commercial licensing inquiries, please contact: [Frances Telfar: atel215@aucklanduni.ac.nz]
See the [LICENSE](LICENSE) for more details.

# Project Development Setup Instructions

**NOTE: Currently, only x64 Windows and x64 Linux development is supported for the project.**

## Windows x64

### 1. Installing the Required Tools (MSVC & Windows SDK)

In order to work with the codebase, you'll need the [Microsoft C/C++ Build Tools
v15 (2017) or later](https://aka.ms/vs/17/release/vs_BuildTools.txt), for both
the Windows SDK and the MSVC compiler and linker.

If the Windows SDK is installed (e.g. via installation of the Microsoft C/C++
Build Tools), you may also build with [Clang](https://releases.llvm.org/).

### 2. Build Environment Setup

Building the codebase can be done in a terminal which is equipped with the
ability to call either MSVC or Clang from command line.

This is generally done by calling `vcvarsall.bat x64`, which is included in the
Microsoft C/C++ Build Tools. This script is automatically called by the `x64
Native Tools Command Prompt for VS <year>` variant of the vanilla `cmd.exe`. If
you've installed the build tools, this command prompt may be eaisly located by
searching for `Native` from the Windows Start Menu search.

You can ensure that the MSVC compiler is accessible from your command line by
running:

```
cl
```

If everything is set up correctly, you should have output very similar to the
following:

```
Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30151 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

usage: cl [ option... ] filename... [ /link linkoption... ]
```

### 3. Building

Within this terminal, `cd` to the root directory of the codebase, and just run
the `build.bat` script:

```
build
```

You should see the following output:

```
[debug mode]
[msvc compile]
[default mode, assuming `tide` build]
metagen_main.c
searching P:/tide/src... 458 files found
parsing metadesk... 16 metadesk files parsed
gathering tables... 97 tables found
generating layer code...
tide_main.c
```

If everything worked correctly, there will be a `build` folder in the root
level of the codebase, and it will contain a freshly-built `tide.exe`.

This `tide.exe` will have been built in **debug mode**, which is not built
with optimisations, and may perform worse. To produce a
**release mode executable**, run `build.bat` with a `release` argument:

```
build release
```

This build will take significantly longer.

By default, `build.bat` only builds tide if no arguments (or just
`release`) are passed.

## Linux x64

### 1. Installing the Required Tools (GCC or Clang, Libraries)

First you'll need either GCC or Clang, if you don't already have them. They
can be obtained by running one of the following commands, depending on your
toolchain of choice and distribution:

### GCC on Ubuntu / Debian / Mint
```
sudo apt update && sudo apt install build-essential
```

### Clang on Ubuntu / Debian / Mint
```
sudo apt update && sudo apt install clang llvm
```

### GCC on Arch / Manjaro
```
sudo pacman -S base-devel
```

### Clang on Arch / Manjaro
```
sudo pacman -S clang lvvm
```

If you've installed the Clang and LLVM tooling required, you can run:

```
clang --version && llvm-ar --version
```

You should see output similar to the following:

```
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/bin
Ubuntu LLVM version 18.1.3
  Optimized build.
```

If you've installed the GCC tooling required, you can run:

```
gcc --version && gcc-ar --version
```

You should see output similar to the following:


```
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

GNU ar (GNU Binutils for Ubuntu) 2.42
Copyright (C) 2024 Free Software Foundation, Inc.
This program is free software; you may redistribute it under the terms of
the GNU General Public License version 3 or (at your option) any later version.
This program has absolutely no warranty.
```

### 2. Installing Dependencies

The tide project relies on a few dynamically linked libraries being present
on the system:

- `libfreetype`
- `libx11`
- `libxext`
- `libxfixes`
- `libxrandr`
- `libgl`
- `libegl`

You'll need the development packages for them in order to build. These can be
installed using one of the following commands, depending on your distribution:

### Ubuntu / Debian / Mint

```
sudo apt install -y libfreetype6-dev libx11-dev libxext-dev libxfixes-dev libxrandr-dev libgl-dev libegl-dev
```

#### Arch / Manjaro

```
sudo pacman -S --needed freetype2 libx11 libxext libxfixes libxrandr libglvnd
```

### 3. Building

To build, `cd` to the root directory of the codebase, and run the `build.sh`
script:

```
./build.sh
```

You should see something similar to the following output:

```
[clang compile]
[debug mode]
[building metagen]
searching /mnt/p/tide/src... 562 files found
parsing metadesk... 23 metadesk files parsed
gathering tables... 138 tables found
generating layer code...
```

If everything worked correctly, there will be a `build` folder in the root
level of the codebase, and it will contain a freshly-build `tide` binary.

This `tide` will have been build in **debug mode**, which is not build with
optimizations, and may perform worse. To produce a **release mode executable**,
run `build.sh` with a `release` argument:

```
./build.sh release
```

This build will take significantly longer.

By default, `build.sh` only builds the program if no arguments (or just `release`) are passed, but additional arguments can be passed.

---

# Codebase Introduction

## Top-Level Directory Descriptions

- `data`: Small binary files which are used when building, either to embded
  within build artefacts, or to package with them.
- `src`: All source code.

After setting up the codebase and building, the following directories will
also exist:

- `build`: All build artifacts. Not checked in to version control.
- `local`: Local files, used for local build configuration input files. Not
  checked into version control.

## Layer Descriptions

The codebase is organised into *layers*. Layers are separated either to isolate
certain problems, and to allow inclusion into various builds without needing to
pull everything in the codebase into a build. Layers correspond with folders
inside of the `src` directory. Sometimes, one folder inside of the `src`
directory will include multiple sub-layers, but the structure is intended to be
fairly flat.

Layers correspond roughly 1-to-1 with *namespaces*. The term "namespaces" in
this context does not refer to specific namespace language features, but rather
a naming convention for C-style namespaces, which are written in the codebase as
a short prefix, usually 1-3 characters, followed by an underscore. These
namespaces are used such that the layer to which certain code belongs may be
quickly understood by glancing at code. The namespaces are generally quite short
to ensure that they aren't much of a hassle to write. Sometimes, multiple sub-
layers will share a namespace. A few layers do not have a namespace, but most
do. Namespaces are either all-caps or lowercase depending on the context in
which they're used. For types, enum values, and some macros, they are
capitalised. For functions and global variables, they are lowercase.

Layers depend on other layers, but ciruclar dependencies would break the
separability and isolation utility of layers (in effect, forming one big layer),
so in other words, layers are arranged into a directed acyclic graph.

A list of the layers in the codebase and their associated namespaces is below:
- `artifact_cache` (`AC_`): Implements an asynchronously-filled cache of
  computation artifacts, which are automatically evicted when not accessed. Used
  for asyncrhonously streaming and caching process memory and file system
  contents, as well as asynchronously preparing visualiser data.
- `base` (no namespace): Universla, codebase-wide constructs. Strings, math,
  memory allocators, helper macros, command-line parsing, and so on. Requires
  no other codebase layers.
- `config` (`CFG_`): General configuration layer for process settings.
  The layer takes configuration as text, and turns it into a mutable tree
  of nodes. Other systems can query this tree and build more convenient
  runtime structures from it.
- `content` (`C_`): Implements a cache for general data blobs, key by a
  128-bit hash of the data. Also implements a keying system on top, where keys
  refer to a unique identity which corresponds to a history of 128-bit hashes.
  User as a general data store by other layers.
- `dns` (`DNS_`): This layer implements both high-level DNS funtions for DNS message
  parsing, packing, and unpacking. This layer is intended to be used in-tandem with the
  `net` layer.
- `draw` (`DR_`): Implements a high-level grapihcs drawing API for the proxy's
  purposes, using the underlying `render` abstraction layer. Provides
  high-level APIs for various draw commands, but takes care of batching them,
  and so on.
- `file_stream` (`FS_`): Implements asynchronous file streaming, storing the
  artifacts inside of the cache implemented by the `content` and `artifact_cache`
  layers, hot-reloading the contents of files when they change.
  Allows callers to map file paths to data hashes, which can be used to obtain
  the file's data.
- `font_cache` (`FNT_`): Implements a cache of rasterised font data, both in
  CPU-side data for text shaping, and in GPU texture atlases for rasterise
  glyphs. All cache information is sourced from the `font_provider` abstraction
  layer
- `font_provider` (`FP_`): An abstraction layer for various font file decoding
  and font rasterisation backends.
- `linux` (`LNX_`): The OS platform layer for Linux-based operating systems.
- `mdesk` (`MD_`): Code for parsing Metadesk files (stored as `.mdesk`), which
  is the JSON-like (technically a JSON superset) text format used for the proxy's
  user the project configuration files and metacode, which is parsed and used to
  generate code with the `metagen` layer.
- `metagen` (`MG_`): A metaprogram which is used to generate primarily code and
  data tables. Consumes Metadesk files, stored with the extension `.mdesk`, and
  generates C code which is then included by hand-written C code. Currently, it
  does not analyse the codebase's hand-written C code, but in principle this is
  possible. This allows easier & less-error-prone management of large data
  tables, which are then used to produce e.g. C `enum`s and a number of
  associated data tables. There are also a number of other generation features,
  like embedding binary files or complex multi-line strings into source code.
- `mutable_text` (`MTX_`): Implements an asynchronously-filled-and-mutated
  cache for text buffers which are mutated across time.
- `net` (`NET_`): Implements a basic platform abstraction layer for networked
  applications. Sending and receiving is asynchronous and user-code is intended
  to be largely stateless.
- `render` (`R_`): An abstraction layer providing an abstract API for rendering
  using various GPU APIs under a common interface. Does not implement a high
  level drawing API - this layer is strictly for minimally abstracting on an
  as-needed basis. Higher level drawing features are implemented in the `draw`
  layer.
- `scratch` (no namespace): Scratch space for small and transient test program.
- `shell` (`SH_`): Implements a platform abstraction layer for shell operations.
- `socks5` (`S5_`): Implements a (not-yet) RFC-compliant SOCKS5 protocol.
- `text` (`TXT_`): Implements text processing functions, like parsing line
  breaks, and lexing and parsing source code. Also offers an API to do this
  asynchronously.
- `third_party` (no namespace): External code from other projects, which some
  layers in the codebase depend on. All external code is included and built
  directly within the codebase.
- `tide` (`TI_`): The layer which ties everything together for the main
  graphical proxy executable. Implements the proxy's graphical frontend,
  all of the proxy-specific UI, and all built-in visualisers.
- `torment` (`T_`): A program use for automated testing.
- `ui` (`UI_`): Machineery for building graphical user interfaces. Provides a
  core immediate mode hierarchical user interface data structure building
  API, and has helper layers for building some higher-level widgets.
- `win32` (`W32_`): The OS platform layer for the Windows operating system.
- `window_manager` (`WM_`): Implements a basic window creation and management
  system to be used by other layers such as `render`.

