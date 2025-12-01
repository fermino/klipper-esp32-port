cmake_minimum_required(VERSION 3.19)

#[[
    Klipper relies on "compile time requests" for build-time configuration such
    as defining commands, pin aliases, what tasks must run in the main loop and
    other things.

    For this, the developer relies on DECL_* macros that add information to the
    '.compile_time_request' section in the resulting object files. Those files
    are then parsed and a 'compile_time_request.c' source file is generated.
    This file is linked against the whole project. It is the "glue" that holds
    the project together and make it easy to have build-time configuration that
    can be easily optimizable by the compiler.

    Because ESP-IDF uses CMake, care needs to be taken so that klipper is
    built before the object files are parsed. For this we take a slightly
    different approach than [1] and use two components that live inside
    idf_components.

    klipper: acts as a library but in reality holds everything from the
        peripheral implementations to the firmware entry point. It is not
        freestanding, as it requires some symbols to later be defined by
        libmain.
    main: the "by-default" primary component in ESP-IDF. It takes care of
        generating the C file *after* libklipper is built. It is then linked
        against libklipper and this is the final binary that will be flashed to
        the MCU

    Both reference code outside their respective paths, following the usual
    klipper structure. It should be noted that the compile time request files
    and the resulting dictionary will be located in out/ to keep things
    consistent with other platforms. Everything else is located inside cmake's
    build path.

    Both components use a linker fragment [2] that make all code and data go to
    IRAM and DRAM respectively. This is done to avoid any possible jitter due
    to from flash cache misses. See [3] for more information.

    Some variables are provided from this file to modify the behavior of the
    CTR generation process.

    Additional settings [4] are set here so that the project is built with the
    minimal amount of dependencies possible.

    [1] https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-guides/build-system.html#source-code-generation
    [2] https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-guides/linker-script-generation.html
    [3] https://github.com/fermino/klipper-esp32-port/issues/1
    [4] https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-guides/build-system.html#optional-project-variables
]]

# Root klipper path
get_filename_component(KLIPPER_ROOT "../.." ABSOLUTE)

# Directories ESP-IDF will look up for components
set(EXTRA_COMPONENT_DIRS "${KLIPPER_ROOT}/src/esp32/idf_components")
# List of components to be built
set(COMPONENTS "klipper" "main")

# CTR scripts and file paths
set(CTR_SCRIPT_FIND_OBJECT_FILES    "${KLIPPER_ROOT}/scripts/ctr/find_object_files")
set(CTR_SCRIPT_EXTRACT_SECTIONS     "${KLIPPER_ROOT}/scripts/ctr/extract_sections")
set(CTR_SCRIPT_BUILD_COMMANDS       "${KLIPPER_ROOT}/scripts/buildcommands.py")
set(CTR_PATH_EXTRACT_FROM   "${CMAKE_BINARY_DIR}/esp-idf/klipper/")
set(CTR_PATH_TXT            "${KLIPPER_ROOT}/out/compile_time_request.txt")
set(CTR_PATH_SRC            "${KLIPPER_ROOT}/out/compile_time_request.c")
set(CTR_PATH_DICT           "${KLIPPER_ROOT}/out/klipper.dict")

# ESP-IDF :)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(klipper)
