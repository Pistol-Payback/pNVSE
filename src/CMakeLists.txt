set(PLUGIN_HEADERS
        fn_intro_to_script_functions.h
        fn_typed_functions.h
        ppNVSE.h
        ppNVSE_Functions.h
        WeapInstFunct.h
)

set(PLUGIN_SOURCES
        main.cpp
        NiStreamFunctions.cpp
)

add_library("${PROJECT_NAME}" SHARED "${PLUGIN_HEADERS}" "${PLUGIN_SOURCES}")

include("${PROJECT_SOURCE_DIR}/cmake/Macros.cmake")

target_add_nvse("${PROJECT_NAME}")
