option(COPPER_INCLUDE_EXT "should include extensions" OFF)

if(COPPER_INCLUDE_EXT)
    file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
    find_package(raylib REQUIRED)
else()
    file(GLOB src
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/math/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pipeline/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/print/*.cpp)
endif()

add_library(copper STATIC ${src})
target_include_directories(copper PUBLIC
    core math pipeline print ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(copper PUBLIC se_tools)
#target_compile_options(copper PUBLIC -O3 -march=native)
if(COPPER_INCLUDE_EXT)
    target_link_libraries(copper PUBLIC raylib)
    target_compile_definitions(copper PUBLIC "COPPER_INCLUDE_EXT")
endif()
