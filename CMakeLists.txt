cmake_minimum_required(VERSION 3.16)

project(NotesApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Sql OpenGL PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Sql OpenGL PrintSupport)

# Core modules
set(CORE_SOURCES
    src/core/note.cpp
    src/core/page.cpp
    src/core/document.cpp
    src/core/storage.cpp
    src/core/object.cpp
    src/core/textobject.cpp
    src/core/drawingobject.cpp
    src/core/imageobject.cpp
    src/core/pdfobject.cpp
)

set(CORE_HEADERS
    src/core/note.h
    src/core/page.h
    src/core/document.h
    src/core/storage.h
    src/core/object.h
    src/core/textobject.h
    src/core/drawingobject.h
    src/core/imageobject.h
    src/core/pdfobject.h
)

# GUI modules
set(GUI_SOURCES
    src/gui/mainwindow.cpp
    src/gui/pagecanvas.cpp
    src/gui/texteditor.cpp
    src/gui/drawingwidget.cpp
    src/gui/objectselector.cpp
    src/gui/toolbar.cpp
    src/gui/markdownrenderer.cpp
)

set(GUI_HEADERS
    src/gui/mainwindow.h
    src/gui/pagecanvas.h
    src/gui/texteditor.h
    src/gui/drawingwidget.h
    src/gui/objectselector.h
    src/gui/toolbar.h
    src/gui/markdownrenderer.h
)

# UI files
set(UI_FILES
    src/gui/mainwindow.ui
    src/gui/texteditor.ui
)

set(PROJECT_SOURCES
    main.cpp
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    ${CORE_HEADERS}
    ${GUI_HEADERS}
    ${UI_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NotesApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET NotesApp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(NotesApp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(NotesApp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(NotesApp PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Sql 
    Qt${QT_VERSION_MAJOR}::OpenGL 
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NotesApp)
endif()
set_target_properties(NotesApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS NotesApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NotesApp)
endif()
