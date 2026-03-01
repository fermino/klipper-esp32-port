#[[
    Register klipper as an IDF component (everything except the CTR).

    Sources, include directories and ESP-IDF required components come from
    Klipper makefiles.

    ESP-IDF components will be included as-needed to reduce build times.
]]
execute_process(
    WORKING_DIRECTORY "${PROJECT_DIR}/../.."
    COMMAND make env_for_cmake
    COMMAND grep -A9 ^__ENV_START__
    COMMAND grep -B9 ^__ENV_END__
    COMMAND grep -v -e ^__ENV_START__ -e ^__ENV_END__
    COMMAND_ERROR_IS_FATAL ANY
    OUTPUT_VARIABLE ENV_FROM_MAKE
)

string(REPLACE "\n" ";" ENV_FROM_MAKE "${ENV_FROM_MAKE}")
foreach (VAR ${ENV_FROM_MAKE})
    string(REPLACE "="  ";" VAR "${VAR}")
    list(GET VAR 0 KEY)
    list(GET VAR 1 VALUE)
    string(REPLACE "'" "" VALUE "${VALUE}")
    string(REPLACE " " ";" VALUE "${VALUE}")
    set("${KEY}" "${VALUE}")
endforeach ()

if("${ESPIDF_SOURCES}" STREQUAL "")
    message(FATAL_ERROR "ESPIDF_SOURCES must be defined")
endif()
if("${ESPIDF_INCLUDE_DIRS}" STREQUAL "")
    message(FATAL_ERROR "ESPIDF_INCLUDE_DIRS must be defined")
endif()
if("${ESPIDF_COMPONENTS}" STREQUAL "")
    message(FATAL_ERROR "ESPIDF_COMPONENTS must be defined")
endif()

list(TRANSFORM ESPIDF_SOURCES PREPEND "../../../")
list(TRANSFORM ESPIDF_INCLUDE_DIRS PREPEND "../../../../")

idf_component_register(
    SRCS ${ESPIDF_SOURCES}
    INCLUDE_DIRS ${ESPIDF_INCLUDE_DIRS}
    PRIV_REQUIRES ${ESPIDF_COMPONENTS}

    # Map the whole app to IRAM on startup to avoid possible flash cache misses
    # that would introduce jitter. You can verify this by by running
    # `idf.py size` and making sure IRAM and Flash Code `.text` sections match
    # with the smallest difference possible (usually around 1KiB).
    LDFRAGMENTS "noflash.lf"
)
