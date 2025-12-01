#[[
    Main component. Takes care of generating compile_time_request.c and linking
    the whole project into the final binary. See the main CMakeLists.txt for
    more details on how this process works; some variables are expected to be
    defined.

    Note that the steps are not specified in order here due to how CMake
    interprets this file [1].

    [1] https://esp32.com/viewtopic.php?t=29402
]]

#[[
    Step 3
    Build the generated source file and link it with klipper. This is the final
    file that will be flashed to the MCU.
]]
idf_component_register(
    SRCS "${CTR_PATH_SRC}"
    INCLUDE_DIRS
        "${KLIPPER_ROOT}/out"
    LDFRAGMENTS "noflash.lf"
)

#[[
    Step 1
    Build a TXT file with the contents of the extracted sections. It directly
    depends on __idf_klipper, which is the internal name that ESP-IDF gives
    to our klipper component.
]]
add_custom_command(
    OUTPUT "${CTR_PATH_TXT}"
    DEPENDS __idf_klipper
    COMMAND "${CTR_SCRIPT_FIND_OBJECT_FILES}" "${CTR_PATH_EXTRACT_FROM}"
        | "${CTR_SCRIPT_EXTRACT_SECTIONS}" "${CMAKE_OBJCOPY}"
        > "${CTR_PATH_TXT}"
    COMMENT "Extracting compile time request sections from object files"
    VERBATIM
)

#[[
    Step 2
    Generate the C source file and the dictionary.
]]
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(
    OUTPUT "${CTR_PATH_SRC}"
    DEPENDS "${CTR_PATH_TXT}"
    COMMAND "${Python3_EXECUTABLE}" "${CTR_SCRIPT_BUILD_COMMANDS}"
        -t "${CMAKE_C_COMPILER};${CMAKE_ASM_COMPILER};${CMAKE_LINKER};${CMAKE_OBJCOPY};${CMAKE_OBJDUMP};${CMAKE_STRIP}"
        -d "${CTR_PATH_DICT}" "${CTR_PATH_TXT}" "${CTR_PATH_SRC}"
    WORKING_DIRECTORY "${KLIPPER_ROOT}"
    COMMENT "Generating compile_time_request.c and dictionary"
    VERBATIM
)
