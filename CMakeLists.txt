cmake_minimum_required(VERSION 3.20)
project(lpc55s69_online_update C)

add_library(ota_core STATIC
    source/ota/ota_crc32.c
    source/ota/ota_update.c)
target_include_directories(ota_core PUBLIC include)
target_compile_features(ota_core PUBLIC c_std_11)
target_compile_options(ota_core PRIVATE -Wall -Wextra -Wpedantic)

option(OTA_BUILD_TESTS "Build portable host tests" ON)
if(OTA_BUILD_TESTS)
    enable_testing()
    add_executable(test_ota tests/test_ota.c)
    target_link_libraries(test_ota PRIVATE ota_core)
    add_test(NAME ota_core_tests COMMAND test_ota)
endif()
