# Host tests of the Arduino-independent core (tier T0). Build and run:
#   cmake -S tests/host -B tests/host/build && cmake --build tests/host/build && tests/host/build/phyphox_tests
# `gen_examples <dir>` writes the example configurations as .phyphox files (conformance step).
cmake_minimum_required(VERSION 3.16)
project(phyphox_ble_core_tests CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
add_compile_options(-Wall -Wextra)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
add_library(phyphox_core STATIC
  ${SRC}/phyphoxBleExperiment.cpp
  ${SRC}/core/Crc32.cpp ${SRC}/core/Sink.cpp ${SRC}/core/Errors.cpp
  ${SRC}/core/ChannelStore.cpp ${SRC}/core/Transfer.cpp
  ${SRC}/core/ExperimentStore.cpp ${SRC}/core/Serializer.cpp ${SRC}/core/Server.cpp)
target_include_directories(phyphox_core PUBLIC ${SRC})

add_executable(phyphox_tests
  test_main.cpp test_crc_sink.cpp test_text.cpp test_builder_store.cpp
  test_serializer.cpp test_transfer.cpp test_channels.cpp test_server.cpp
  test_nina_protocol.cpp)
target_link_libraries(phyphox_tests phyphox_core)
target_include_directories(phyphox_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(phyphox_tests PRIVATE GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")

add_executable(gen_examples gen_examples.cpp)
target_link_libraries(gen_examples phyphox_core)
target_include_directories(gen_examples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME core COMMAND phyphox_tests)
