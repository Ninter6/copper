file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)

find_package(raylib REQUIRED)

add_library(copper STATIC ${src}
        ext/gui.cpp)
target_include_directories(copper PUBLIC
    core math pipeline print ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(copper PUBLIC se_tools raylib)
target_compile_options(copper PUBLIC -O3 -march=native)
