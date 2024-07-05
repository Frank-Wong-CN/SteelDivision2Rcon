cmake_minimum_required(VERSION 3.5)

project(SteelDiv2Rcon VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

include_directories(BEFORE rcon)

set(TS_FILES SteelDiv2Rcon_zh_CN.ts)
set(ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.rc")

set(PROJECT_SOURCES
        main.cpp
        main_window.cpp
        main_window.h
        main_window.ui
        src/macro.h
        src/config.h
        src/config.inc
        src/controller.h
        src/macro_qjson.h
        src/notification_delegate.h src/notification_delegate.cpp
        src/network_runner.h src/network_runner.cpp
        src/view_models.h src/view_models.cpp
        src/server_variables_model.h src/server_variables_model.cpp
        rcon/rcon.h rcon/rcon.cpp
        ${TS_FILES}
)

qt_add_executable(SteelDiv2Rcon
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${ICON_RC}
)

# qt_add_lupdate(SteelDiv2Rcon TS_FILES ${TS_FILES} SOURCES ${PROJECT_SOURCES})
# qt_wrap_ui(SteelDiv2Rcon main_window.ui)
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

qt6_add_resources(SteelDiv2Rcon "translation" PREFIX "/" FILES "SteelDiv2Rcon_zh_CN.qm")

target_link_libraries(SteelDiv2Rcon PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(SteelDiv2Rcon PRIVATE "ws2_32" "mswsock" "iphlpapi")

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SteelDiv2Rcon)
endif()
set_target_properties(SteelDiv2Rcon PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SteelDiv2Rcon
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SteelDiv2Rcon)
endif()
