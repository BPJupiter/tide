#!/bin/bash

set -eu
cd "$(dirname "$0")"

# --- Usage Notes --------------------------------------------------------------
#
# This is a central build script for the RAD Debugger project, for use in
# Linux development environments. It takes a list of simple alphanumeric-
# only arguments which control (a) what is built, (b) which compiler is
# used, and (c) extra high-level build options. By default, if no options
# are passed, the `torment` test runner is built.
#
# Below is a non-exhaustive list of possible ways to use the script:
#   ./build.sh torment
#   ./build.sh torment gcc
#   ./build.sh torment release
#   ./build.sh torment asan telemetry
#
# For a full list of possible build targets and their build command lines,
# search for @build_targets in this file.
#
# Below is a list of all possible non-target command line options:
#
# - `asan`: enable address sanitizer
# - `ubsan`: enable undefined-behavior sanitizer
# - `telemetry`: enable RAD telemetry profiling support
# - `spall`: enable spall profiling support
# - `opengl`: force the OpenGL render backend
# - `pgo`: enable profile-guided optimization (clang only, two-pass: run once
#          to instrument, run the resulting binary, then rerun with `pgo_run`)

# --- Default Target When No Args Given -----------------------------------------
first_arg="${1:-}"
second_arg="${2:-}"
if [[ -z "$first_arg" ]] || { [[ "$first_arg" == "release" ]] && [[ -z "$second_arg" ]]; }; then
  echo "[default mode, assuming \`torment\` build]"
  set -- "$@" torment
fi

# --- Unpack Arguments -----------------------------------------------------------
for arg in "$@"; do declare $arg='1'; done

# --- Unpack Command Line Build Options -------------------------------------------
auto_compile_flags=""
if [[ "${telemetry:-0}" == "1" ]]; then
  echo "[telemetry profiling enabled]"
  auto_compile_flags="$auto_compile_flags -DPROFILE_TELEMETRY=1"
fi
if [[ "${spall:-0}" == "1" ]]; then
  echo "[spall profiling enabled]"
  auto_compile_flags="$auto_compile_flags -DPROFILE_SPALL=1"
fi
if [[ "${asan:-0}" == "1" ]]; then
  echo "[asan enabled]"
  auto_compile_flags="$auto_compile_flags -fsanitize=address"
fi
if [[ "${ubsan:-0}" == "1" ]]; then
  echo "[ubsan enabled]"
  auto_compile_flags="$auto_compile_flags -fsanitize=undefined"
fi
if [[ "${opengl:-0}" == "1" ]]; then
  echo "[opengl render backend]"
  auto_compile_flags="$auto_compile_flags -DR_BACKEND=R_BACKEND_OPENGL"
fi
if [[ "${pgo:-0}" == "1" ]]; then
  command -v llvm-profdata > /dev/null 2>&1 || { echo "llvm-profdata is not in the PATH"; exit 1; }
  if [[ "${clang:-1}" == "1" ]]; then
    if [[ "${pgo_run:-0}" == "1" ]]; then
      llvm-profdata merge build/build.profraw -output=build/build.profdata
      auto_compile_flags="$auto_compile_flags -fprofile-use=build/build.profdata"
    else
      echo "[pgo enabled]"
      auto_compile_flags="$auto_compile_flags -fprofile-generate -mllvm -vp-counters-per-site=5"
      export LLVM_PROFILE_FILE="build/build.profraw"
    fi
  else
    echo "ERROR: PGO build is not supported with current compiler"
    exit 1
  fi
fi

# --- Get Current Git Commit Id ---------------------------------------------------
git_hash=$(git describe --always --dirty)
git_hash_full=$(git rev-parse HEAD)

# --- Compile/Link Line Definitions ------------------------------------------------
cc_cflags_gcc=""
cc_cflags_clang="-fdiagnostics-absolute-paths -Wno-for-loop-analysis -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-compare-distinct-pointer-types -Wno-single-bit-bitfield-constant-conversion -Wno-deprecated-declarations -Wno-writable-strings -Wno-unknown-warning-option -Wno-deprecated-register -Wno-unused-local-typedef -msse2"
cc_common="-mcx16 -I../src/ -I../local/ -D_GNU_SOURCE -g -Wall -Wno-missing-braces -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf ${auto_compile_flags}"
cc_debug="-O0 -DBUILD_DEBUG=1 ${cc_common}"
cc_release="-O2 -DBUILD_DEBUG=0 ${cc_common}"
cc_link="-lpthread -lm -lrt -ldl"

# --- Per-Build Settings -----------------------------------------------------------
cc_link_dll="-fPIC"

# --- External Libraries -------------------------------------------------------------
# sudo apt install -y pkg-config libfreetype6-dev libx11-dev libxext-dev libgl-dev libegl-dev
if [[ -x "$(command -v pkg-config)" ]]; then
  cc_font_provider="$(pkg-config --cflags --libs freetype2)"
  cc_os_gfx="$(pkg-config --cflags --libs x11 xext)"
  cc_render="$(pkg-config --cflags --libs gl egl)"
else
  cc_font_provider="-I/usr/include/freetype2 -lfreetype"
  cc_os_gfx="-lX11 -lXext"
  cc_render="-lGL -lEGL"
fi

# --- Choose Compile/Link Lines -------------------------------------------------------
if   [[ "${gcc:-0}"   == "1" ]]; then compiler="${CC:-gcc}   $cc_cflags_gcc";   echo "[gcc compile]";
elif [[ "${clang:-1}" == "1" ]]; then compiler="${CC:-clang} $cc_cflags_clang"; echo "[clang compile]";
fi
compile_debug="$compiler $cc_debug"
compile_release="$compiler $cc_release"
if   [[ "${release:-0}" == "1" ]]; then echo "[release mode]"; compile="$compile_release";
elif [[ "${debug:-1}"   == "1" ]]; then echo "[debug mode]";   compile="$compile_debug";
fi
compile="$compile -DBUILD_GIT_HASH=\"$git_hash\" -DBUILD_GIT_HASH_FULL=\"$git_hash_full\""

# --- Prep Directories -----------------------------------------------------------------
mkdir -p build local

# --- Build & Run Metaprogram ------------------------------------------------------------
pushd build > /dev/null
if [[ "${meta:-0}" == "1" ]]; then
  echo "[building metagen]"
  $compile_debug ../src/metagen/metagen_main.c $cc_link -o metagen
fi
if [[ "${no_meta:-0}" == "0" ]] && [[ -x ./metagen ]]; then
  echo "[running metagen]"
  ./metagen
fi
popd > /dev/null

# --- Build Everything (@build_targets) ----------------------------------------------------
cd build
if [[ "${torment:-0}"  == "1" ]];          then didbuild=1 && $compile ../src/torment/torment_main.c $cc_link -o torment; fi
if [[ "${tide:-0}"     == "1" ]];          then didbuild=1 && $compile ../src/tide/tide_main.c $cc_link $cc_os_gfx $cc_render $cc_font_provider -o tide; fi
if [[ "${critters:-0}" == "1" ]];          then didbuild=1 && $compile ../src/scratch/critters.c $cc_link $cc_os_gfx $cc_render $cc_font_provider -o critters; fi
if [[ "${borked_dns_server:-0}" == "1" ]]; then didbuild=1 && $compile ../src/scratch/borked_dns_server.c $cc_link -o borked_dns_server; fi
cd ..

# --- Warn On No Builds -----------------------------------------------------------------
if [[ "${didbuild:-0}" == "0" ]]
then
  echo "[WARNING] no valid build target specified; must use build target names as arguments to this script, like \`./build.sh torment\`."
  exit 1
fi
