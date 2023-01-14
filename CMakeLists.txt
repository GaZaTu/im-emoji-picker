cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
project(ibus-test VERSION 0.1.0 LANGUAGES CXX)

set(PROJECT_NAME_FULL ${PROJECT_NAME})
set(PROJECT_ORGANIZATION gazatu.xyz)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
  cmake_policy(SET CMP0074 NEW)
endif ()

add_definitions(
  -DPROJECT_ORGANIZATION="${PROJECT_ORGANIZATION}"
  -DPROJECT_NAME="${PROJECT_NAME}"
  -DPROJECT_VERSION="${PROJECT_VERSION}"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

find_package(ICU REQUIRED COMPONENTS uc)

find_library(IBUS_LIB ibus-1.0 REQUIRED)
add_library(IBUS STATIC SHARED IMPORTED)
set_target_properties(IBUS PROPERTIES
  IMPORTED_LOCATION ${IBUS_LIB}
)

find_library(GLIB_LIB glib-2.0 REQUIRED)
add_library(GLIB STATIC SHARED IMPORTED)
set_target_properties(GLIB PROPERTIES
  IMPORTED_LOCATION ${GLIB_LIB}
)

find_library(GOBJECT_LIB gobject-2.0 REQUIRED)
add_library(GOBJECT STATIC SHARED IMPORTED)
set_target_properties(GOBJECT PROPERTIES
  IMPORTED_LOCATION ${GOBJECT_LIB}
)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(/usr/include/ibus-1.0)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib64/glib-2.0/include)

set(COMMON_SRC_FILES
  src/rime_main.cpp
  src/rime_engine.cpp
)

set(SRC_FILES
  ${COMMON_SRC_FILES}
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  ICU::uc
  ${IBUS_LIB}
  ${GLIB_LIB}
  ${GOBJECT_LIB}
)
