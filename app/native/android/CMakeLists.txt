# CMakeLists.txt (Android NDK) — compila o dsp-core + a ponte JNI e linka o Oboe.
# Coloque este arquivo em app/android/app/src/main/cpp/ junto com native-lib.cpp.
cmake_minimum_required(VERSION 3.22)
project(altofalante_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Caminho para o dsp-core do repositório. AJUSTE se mover o app para fora do repo.
# A partir de app/android/app/src/main/cpp são 6 níveis até a raiz do repositório.
set(DSP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../dsp-core")

add_library(altofalante SHARED
    native-lib.cpp
    ${DSP_DIR}/src/gain.cpp
    ${DSP_DIR}/src/eq.cpp
    ${DSP_DIR}/src/multiband_comp.cpp
    ${DSP_DIR}/src/exciter.cpp
    ${DSP_DIR}/src/stereo_widen.cpp
    ${DSP_DIR}/src/limiter.cpp
    ${DSP_DIR}/src/presets.cpp
    ${DSP_DIR}/src/pipeline.cpp
)

target_include_directories(altofalante PRIVATE
    ${DSP_DIR}/include
    ${DSP_DIR}/src
)

# Oboe vem via prefab (dependência Gradle com.google.oboe:oboe).
find_package(oboe REQUIRED CONFIG)
find_library(log-lib log)
target_link_libraries(altofalante oboe::oboe ${log-lib})
