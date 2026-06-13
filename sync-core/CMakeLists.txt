# Biblioteca de sincronizacao multi-celular (portatil).
add_library(altofalante_sync STATIC
    src/clock_sync.cpp
    src/scheduler.cpp
    src/net.cpp
)
target_include_directories(altofalante_sync PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(altofalante_sync PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(altofalante_sync PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON
)

# Simulacao/teste de sincronizacao (roda no PC/CI, sem device).
add_executable(sync_sim sync_sim.cpp)
target_link_libraries(sync_sim PRIVATE altofalante_sync)

# No real de sincronizacao sobre UDP (lider/seguidor).
add_executable(sync_node sync_node.cpp)
target_link_libraries(sync_node PRIVATE altofalante_sync)
target_include_directories(sync_node PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
