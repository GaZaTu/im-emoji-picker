cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
project(im-emoji-picker VERSION 0.1.0 LANGUAGES CXX)

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

set(CMAKE_INSTALL_PREFIX /usr)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME_FULL})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_ORGANIZATION})

if (DEFINED PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
endif ()

set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  ${CMAKE_INSTALL_PREFIX}/share
  ${CMAKE_INSTALL_PREFIX}/share/applications
  ${CMAKE_INSTALL_PREFIX}/share/icons
  ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor
  ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/72x72
  ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/72x72/apps
)
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${PROJECT_SOURCE_DIR}/postinst")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${PROJECT_ORGANIZATION})
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PROJECT_SOURCE_DIR}/postinst")

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(ICU REQUIRED COMPONENTS uc)

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SRC_FILES_COMMON
  src/logging.cpp
  src/emojis.cpp
  src/emojis.qrc
  src/kaomojis.cpp
  src/EmojiPickerSettings.cpp
  src/EmojiPickerWindow.cpp
  src/EmojiPickerWindow.qrc
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
    include_directories(/usr/lib/glib-2.0/include)
    include_directories(/usr/lib64/glib-2.0/include)
    include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include)

    set(SRC_FILES_IBUS
      ${SRC_FILES_COMMON}
      src/ibus_main.cpp
      src/IBusImEmojiPickerEngine.cpp
    )

    add_executable(ibusimemojipicker ${SRC_FILES_IBUS})

    set_target_properties(ibusimemojipicker PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
    )

    target_link_libraries(ibusimemojipicker
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      ICU::uc
      ${IBUS_LIB}
      ${GLIB_LIB}
      ${GOBJECT_LIB}
    )

    install(TARGETS ibusimemojipicker RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ibus)
    install(FILES ${PROJECT_SOURCE_DIR}/ibus-component.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component RENAME ibusimemojipicker.xml)
    install(FILES ${PROJECT_SOURCE_DIR}/src/res/im-emoji-picker_32x32.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps RENAME im-emoji-picker.png)
    install(CODE "execute_process(COMMAND touch /usr/share/icons/hicolor)")
    install(CODE "execute_process(COMMAND ibus write-cache)")

    # TODO maybe:
    # add_custom_target(appimage
    #   COMMAND make install DESTDIR=${CMAKE_BINARY_DIR}/_AppDir
    #   COMMAND VERSION=dev linuxdeployqt.AppImage ${CMAKE_BINARY_DIR}/_AppDir/usr/share/applications/${PROJECT_NAME_FULL}.desktop -appimage -no-translations
    # )
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
      src/Fcitx5ImEmojiPickerEngine.cpp
    )

    add_library(fcitx5imemojipicker SHARED ${SRC_FILES_FCITX5})

    set_target_properties(fcitx5imemojipicker PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
      PREFIX ""
    )

    target_link_libraries(fcitx5imemojipicker
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      ICU::uc
      Fcitx5::Core
    )

    install(TARGETS fcitx5imemojipicker RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/fcitx5)
    install(FILES ${PROJECT_SOURCE_DIR}/fcitx5-addon.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx5/addon RENAME fcitx5imemojipicker.conf)
    install(FILES ${PROJECT_SOURCE_DIR}/fcitx5-inputmethod.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx5/inputmethod RENAME fcitx5imemojipicker.conf)
    install(FILES ${PROJECT_SOURCE_DIR}/src/res/im-emoji-picker_32x32.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps RENAME im-emoji-picker.png)
    install(CODE "execute_process(COMMAND touch /usr/share/icons/hicolor)")
  endif ()
endif ()

include(CPack)
