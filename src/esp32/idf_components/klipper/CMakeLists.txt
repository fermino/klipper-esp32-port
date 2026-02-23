#[[
    Register klipper as an IDF component (everything except the CTR).

    Sources, include directories and ESP-IDF required components come from
    Klipper makefiles.

    ESP-IDF components will be included as-needed to reduce build times.
]]

# ESPIDF_SOURCES
if($ENV{ESPIDF_SOURCES} STREQUAL "")
    message(FATAL_ERROR "ESPIDF_SOURCES must be defined")
endif()
string(REPLACE " " ";" ESPIDF_SOURCES "$ENV{ESPIDF_SOURCES}")
list(TRANSFORM ESPIDF_SOURCES PREPEND "../../../")

# ESPIDF_INCLUDE_DIRS
if($ENV{ESPIDF_INCLUDE_DIRS} STREQUAL "")
    message(FATAL_ERROR "ESPIDF_INCLUDE_DIRS must be defined")
endif()
string(REPLACE " " ";" ESPIDF_INCLUDE_DIRS "$ENV{ESPIDF_INCLUDE_DIRS}")
list(TRANSFORM ESPIDF_INCLUDE_DIRS PREPEND "../../../../")

# ESPIDF_COMPONENTS
if($ENV{ESPIDF_COMPONENTS} STREQUAL "")
    message(FATAL_ERROR "ESPIDF_COMPONENTS must be defined")
endif()
string(REPLACE " " ";" ESPIDF_COMPONENTS "$ENV{ESPIDF_COMPONENTS}")

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
