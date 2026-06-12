cmake_minimum_required(VERSION 3.16)

project(PeaksPeaksPeaks VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TUTAJ DODANO 'Charts'
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

set(PROJECT_SOURCES
        src/main.cpp
        src/peakspeakspeaks.cpp
        src/peakspeakspeaks.ui
        src/FileManager.cpp 
        src/SpectrumLoader.cpp 
        src/MathEngine.cpp 
        include/peakspeakspeaks.h
        include/FileManager.h
        include/SpectrumLoader.h
        include/MathEngine.h
        resources/app.qrc
)

# Application icon (window/taskbar icon comes from app.qrc at runtime).
# macOS: bundle icon (.icns). Windows: executable icon via .rc (.ico).
set(APP_ICON_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/resources/AppIcon.icns)
set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

if(APPLE)
    list(APPEND PROJECT_SOURCES ${APP_ICON_MACOS})
elseif(WIN32)
    list(APPEND PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PeaksPeaksPeaks
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PeaksPeaksPeaks APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PeaksPeaksPeaks SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PeaksPeaksPeaks
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# TUTAJ TEŻ DODANO 'Charts' na samym końcu linijki
target_link_libraries(PeaksPeaksPeaks PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts)

target_include_directories(PeaksPeaksPeaks PRIVATE include)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PeaksPeaksPeaks)
endif()
set_target_properties(PeaksPeaksPeaks PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE AppIcon.icns
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PeaksPeaksPeaks
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PeaksPeaksPeaks)
endif()