include_directories(include)
#include_directories(../deps/SFML/include)

file(GLOB_RECURSE KraPunchClient_SRC
    "include/*.h"
    "source/*.c"
    "include/*.hpp"
    "source/*.cpp"
)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})

add_executable(
    KraPunchClient
    ${KraPunchClient_SRC}
)

target_link_libraries(KraPunchClient sfml-network)