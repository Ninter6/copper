file(GLOB src ./*/*.cpp)

add_library(copper STATIC ${src})
target_include_directories(copper PUBLIC
    core math pipeline print
)
target_link_libraries(copper PUBLIC se_tools)
