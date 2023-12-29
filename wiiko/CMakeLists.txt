cmake_minimum_required(VERSION 3.13)
include("/opt/devkitpro/cmake/Wii.cmake")
project (wiiko VERSION 1.0)
include(CMakePrintHelpers)

add_executable(wiiko 
    source/main.cpp 
    source/MiiImage.cpp 
    source/Timer.cpp 
    source/Shirt.cpp 
    $<TARGET_OBJECTS:data_objects>
)
ogc_create_dol(wiiko)

target_include_directories(wiiko PRIVATE /opt/devkitpro/portlibs/wii/include)
target_link_directories(wiiko PRIVATE /opt/devkitpro/portlibs/wii/lib)
target_include_directories(wiiko PRIVATE /opt/devkitpro/libogc/include)
target_link_directories(wiiko PRIVATE /opt/devkitpro/libogc/lib)
target_link_libraries(wiiko -lgrrlib -lfreetype -lbz2 -lpngu -lpng -ljpeg -lz -lfat -lasnd -lwiiuse -lbte -logc -lm)

# Generate data files
# Create folders to hold generated files
set(DATA_INCLUDE_PATH ${CMAKE_BINARY_DIR}/generated/include)
set(DATA_ASM_PATH ${CMAKE_BINARY_DIR}/generated/asm)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${DATA_INCLUDE_PATH}")
file(MAKE_DIRECTORY "${DATA_ASM_PATH}")

# Get list of all data files
file(GLOB_RECURSE DATA_SOURCES CONFIGURE_DEPENDS "data/*.png" "data/*.ttf")

# Generate headers and .s files for binary files
set(DATA_ASM_FILES)
set(DATA_HEADER_FILES)
find_program(BIN2S NAMES bin2s HINTS ${DEVKITPRO}/tools/bin)
foreach(_file ${DATA_SOURCES})
    # image.png -> image_png.h
    cmake_path(GET _file FILENAME _filename)
    string(REPLACE "." "_" _cleaned_file_name ${_filename})
    set(OUTPUT_DATA_HEADER ${DATA_INCLUDE_PATH}/${_cleaned_file_name}.h)
    set(OUTPUT_DATA_ASM ${DATA_ASM_PATH}/${_cleaned_file_name}.s)
    file(TOUCH ${OUTPUT_DATA_ASM})

    # Run bin2s to actually create the .h/.s files
    add_custom_command(
        OUTPUT ${OUTPUT_DATA_HEADER} ${OUTPUT_DATA_ASM}
        COMMAND ${BIN2S} -a 32 -H  ${OUTPUT_DATA_HEADER} ${_file} > ${OUTPUT_DATA_ASM}
        DEPENDS ${_file}
    )

    set_property(SOURCE ${OUTPUT_DATA_ASM} PROPERTY LANGUAGE C)

    # Add generated files to a list
    list(APPEND DATA_HEADER_FILES ${OUTPUT_DATA_HEADER})
    list(APPEND DATA_ASM_FILES ${OUTPUT_DATA_ASM})
endforeach()

# Add dependency on headers
add_custom_target(data_headers DEPENDS ${DATA_HEADER_FILES})
add_dependencies(wiiko data_headers)
target_include_directories(wiiko PRIVATE ${DATA_INCLUDE_PATH})

# Compile data objects
add_library(data_objects OBJECT ${DATA_ASM_FILES})
target_compile_options(data_objects PRIVATE -w)
add_dependencies(data_objects data_headers)
add_dependencies(wiiko data_objects)

# find_program(DOLPHIN NAMES dolphin-emu)
# add_custom_command(
#     OUTPUT RUN_GAME
#     COMMAND ${DOLPHIN} -e ${CMAKE_BINARY_DIR}/wiiko.dol
#     DEPENDS wiiko
# )
# add_custom_target(run DEPENDS RUN_GAME)