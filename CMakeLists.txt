cmake_minimum_required(VERSION 3.16)

# This firmware is hardware-specific to ESP32-S3-WROOM-1-N16R8.
# Keep the target in the repository itself so a plain `idf.py build`
# cannot silently configure the project as classic ESP32 (which lacks GPIO40-48).
set(IDF_TARGET "esp32s3" CACHE STRING "ESP-IDF build target" FORCE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(xiaomeng_mizhixitong)
