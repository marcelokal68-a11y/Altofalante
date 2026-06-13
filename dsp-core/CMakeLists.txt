# Biblioteca do nucleo de DSP (portatil: mesma fonte no PC, iOS e Android).
add_library(altofalante_dsp STATIC
    src/gain.cpp
    src/eq.cpp
    src/multiband_comp.cpp
    src/exciter.cpp
    src/stereo_widen.cpp
    src/limiter.cpp
    src/presets.cpp
    src/pipeline.cpp
    src/analyzer.cpp
)

target_include_directories(altofalante_dsp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(altofalante_dsp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(altofalante_dsp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
