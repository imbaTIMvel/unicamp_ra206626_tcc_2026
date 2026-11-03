cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(e4_kws C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

file(GLOB_RECURSE EI_SRC
  "edge-impulse-sdk/dsp/*.cpp"
  "edge-impulse-sdk/classifier/*.cpp"
  "edge-impulse-sdk/tensorflow/*.c"
  "edge-impulse-sdk/tensorflow/*.cc"
  "tflite-model/*.cpp"
)
# CMSIS-DSP e CMSIS-NN assumem Cortex-M4 ou superior: 661 arquivos que nao servem aqui
list(FILTER EI_SRC EXCLUDE REGEX "/CMSIS/")
# nenhuma camada de porting do SDK atende o RP2040 (ver 6.1)
list(FILTER EI_SRC EXCLUDE REGEX "/porting/")

add_executable(e4_kws main.cpp ei_porting_pico.cpp ${EI_SRC})

target_include_directories(e4_kws PRIVATE .)

target_compile_definitions(e4_kws PRIVATE
  EI_CLASSIFIER_USE_FULL_TFLITE=0
  EIDSP_USE_CMSIS_DSP=0
  EIDSP_QUANTIZE_FILTERBANK=0
  EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=0
)

target_link_libraries(e4_kws pico_stdlib)
pico_enable_stdio_usb(e4_kws 1)
pico_enable_stdio_uart(e4_kws 0)
pico_add_extra_outputs(e4_kws)

list(LENGTH EI_SRC EI_SRC_N)
message(STATUS "fontes EI: ${EI_SRC_N}")