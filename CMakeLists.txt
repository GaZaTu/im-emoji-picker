cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
project(dank-emoji-picker VERSION 0.1.0 LANGUAGES CXX)

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(ICU REQUIRED COMPONENTS uc)

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SRC_FILES_COMMON
  src/logging.cpp
  src/emojis.cpp
  src/emojis.qrc
  src/EmojiWindow.cpp
  src/EmojiLabel.cpp
)

if ("${ONLY_IBUS}" STREQUAL "1")
  set(SKIP_FCITX5 "1")
endif ()

if ("${ONLY_FCITX5}" STREQUAL "1")
  set(SKIP_IBUS "1")
endif ()

if (NOT ("${SKIP_IBUS}" STREQUAL "1"))
  find_library(IBUS_LIB ibus-1.0)
  find_library(GLIB_LIB glib-2.0)
  find_library(GOBJECT_LIB gobject-2.0)

  if (NOT ("${IBUS_LIB}" STREQUAL "IBUS_LIB-NOTFOUND"))
    add_library(IBUS STATIC SHARED IMPORTED)
    set_target_properties(IBUS PROPERTIES
      IMPORTED_LOCATION ${IBUS_LIB}
    )

    add_library(GLIB STATIC SHARED IMPORTED)
    set_target_properties(GLIB PROPERTIES
      IMPORTED_LOCATION ${GLIB_LIB}
    )

    add_library(GOBJECT STATIC SHARED IMPORTED)
    set_target_properties(GOBJECT PROPERTIES
      IMPORTED_LOCATION ${GOBJECT_LIB}
    )

    include_directories(/usr/include/ibus-1.0)
    include_directories(/usr/include/glib-2.0)
    include_directories(/usr/lib64/glib-2.0/include)

    set(SRC_FILES_IBUS
      ${SRC_FILES_COMMON}
      src/main.cpp
      src/IBusEmojiEngine.cpp
    )

    add_executable(${PROJECT_NAME}-ibus ${SRC_FILES_IBUS})

    set_target_properties(${PROJECT_NAME}-ibus PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
    )

    target_link_libraries(${PROJECT_NAME}-ibus
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      ICU::uc
      ${IBUS_LIB}
      ${GLIB_LIB}
      ${GOBJECT_LIB}
    )
  endif ()
endif ()

if (NOT ("${SKIP_FCITX5}" STREQUAL "1"))
  find_package(Fcitx5Core)

  if ("${Fcitx5Core_FOUND}" STREQUAL "1")
    include_directories(/usr/include/Fcitx5/Core)
    include_directories(/usr/include/Fcitx5/Config)
    include_directories(/usr/include/Fcitx5/Utils)

    set(SRC_FILES_FCITX5
      ${SRC_FILES_COMMON}
      src/Fcitx5EmojiEngine.cpp
    )

    add_library(${PROJECT_NAME}-fcitx5 SHARED ${SRC_FILES_FCITX5})

    set_target_properties(${PROJECT_NAME}-fcitx5 PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
      PREFIX ""
    )

    target_link_libraries(${PROJECT_NAME}-fcitx5
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      ICU::uc
      Fcitx5::Core
    )
  endif ()
endif ()
