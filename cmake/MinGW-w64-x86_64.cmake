# Cross-compile Natron+ for 64-bit Windows (MSVCRT MinGW) from Linux.
# Pair with an MSYS2 mingw64 sysroot (Qt5 / Boost / Cairo / PySide2) and a
# Wine wrapper so moc/rcc/shiboken2/python.exe can run at generate time.
#
#   cmake -S . -B build -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/MinGW-w64-x86_64.cmake \
#     -DNATRON_MINGW64=/mingw64 \
#     -DNATRON_WINE_RUN=/abs/path/to/tools/mingw-wine-run.sh
#
# Native windows-latest MSYS2 builds do not use this file.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

if(NOT NATRON_MINGW64)
    if(DEFINED ENV{NATRON_MINGW64})
        set(NATRON_MINGW64 "$ENV{NATRON_MINGW64}")
    else()
        set(NATRON_MINGW64 "/mingw64")
    endif()
endif()
file(TO_CMAKE_PATH "${NATRON_MINGW64}" NATRON_MINGW64)
set(NATRON_MINGW64 "${NATRON_MINGW64}" CACHE PATH "MSYS2 mingw64 sysroot")

if(NOT NATRON_WINE_RUN AND DEFINED ENV{NATRON_WINE_RUN})
    set(NATRON_WINE_RUN "$ENV{NATRON_WINE_RUN}")
endif()
if(NATRON_WINE_RUN)
    set(CMAKE_CROSSCOMPILING_EMULATOR "${NATRON_WINE_RUN}" CACHE FILEPATH
        "Wine wrapper used to run mingw64 build tools")
endif()

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_RC_COMPILER_ARG1 "-I${NATRON_MINGW64}/include")

set(CMAKE_FIND_ROOT_PATH "${NATRON_MINGW64}" "/usr/x86_64-w64-mingw32")
set(CMAKE_PREFIX_PATH "${NATRON_MINGW64}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config" CACHE FILEPATH "Host pkg-config")
set(ENV{PKG_CONFIG_LIBDIR} "${NATRON_MINGW64}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "${NATRON_MINGW64}/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSTEM_INCLUDE_PATH} "${NATRON_MINGW64}/include")
set(ENV{PKG_CONFIG_SYSTEM_LIBRARY_PATH} "${NATRON_MINGW64}/lib")

# Prefer the sysroot Python for Development; Interpreter is overridden by
# NATRON_WINE_PYTHON (a Unix wrapper around python.exe) when set.
if(NOT Python3_EXECUTABLE AND DEFINED ENV{NATRON_WINE_PYTHON})
    set(Python3_EXECUTABLE "$ENV{NATRON_WINE_PYTHON}" CACHE FILEPATH
        "Wine-wrapped mingw64 Python 3")
endif()
set(Python3_ROOT_DIR "${NATRON_MINGW64}")
set(Python3_FIND_REGISTRY NEVER)
set(Python3_USE_STATIC_LIBS OFF)

# Shiboken2Config still calls the deprecated FindPythonInterp/FindPythonLibs
# pair. Pin them to mingw64 CPython 3.14, not the Linux host interpreter.
if(NOT PYTHON_EXECUTABLE AND Python3_EXECUTABLE)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
endif()
if(NOT PYTHON_LIBRARY)
    set(PYTHON_LIBRARY "${NATRON_MINGW64}/lib/libpython3.14.dll.a")
endif()
if(NOT PYTHON_INCLUDE_DIR)
    set(PYTHON_INCLUDE_DIR "${NATRON_MINGW64}/include/python3.14")
endif()
set(PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}" CACHE FILEPATH "mingw64 Python (Wine wrapper or native)")
set(PYTHON_LIBRARY "${PYTHON_LIBRARY}" CACHE FILEPATH "mingw64 Python import library")
set(PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}" CACHE PATH "mingw64 Python headers")
