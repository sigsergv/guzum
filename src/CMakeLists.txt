find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5Test)
find_package(Qt5LinguistTools)
#find_package(Qt5Svg REQUIRED)
#find_package(Qt5Xml REQUIRED)
#find_package(Gpgmepp)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgpgme ")

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE guzum.icns)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${GUZUM_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "© Copyright 2017 Sergey Stolyarov")
    set(MACOSX_BUNDLE_BUNDLE_NAME "Guzum")

    set(myApp_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../resources/icons/guzum.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../resources/icons/guzum.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa")
endif()

set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG_OUTPUT>)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)

set(HEADERS settings.h
    controlpeer.h
    gpgmewrapper.h
    traymanager.h
    prefsdialog.h
    aboutdialog.h
    managehistorydialog.h
    encryptedtextwindow.h
    secureplaintexteditor.h
    passphrasedialog.h
    )

set(SOURCES main.cpp
    controlpeer.cpp
    gpgmewrapper.cpp
    settings.cpp
    traymanager.cpp
    prefsdialog.cpp
    aboutdialog.cpp
    managehistorydialog.cpp
    encryptedtextwindow.cpp
    secureplaintexteditor.cpp
    passphrasedialog.cpp
    )


# hack to add generated qm-files into compiled binary
set(TRANSLATIONS_QRC "translations.qrc")
configure_file(${TRANSLATIONS_QRC} ${TRANSLATIONS_QRC} COPYONLY)

qt5_add_resources(RESOURCES ../resources/application.qrc ${CMAKE_BINARY_DIR}/src/${TRANSLATIONS_QRC})

set(LANGUAGES guzum-ru)
foreach (LANGUAGE ${LANGUAGES})
    qt5_add_translation(TRANSLATIONS ${LANGUAGE}.ts)
    list(APPEND QMS ${LANGUAGE}.qm)
endforeach()

set(ALL_SOURCES ${SOURCES} ${HEADERS} ${TRANSLATIONS} ${RESOURCES})

if(APPLE)
    add_executable(guzum MACOSX_BUNDLE ${myApp_ICON} ${SSH_ASKPASS_BIN} ${ALL_SOURCES} macos.h macos.mm)
endif()

if(WIN32)
    add_executable(guzum WIN32 ${ALL_SOURCES})
endif()

if(UNIX AND NOT APPLE)
    add_executable(guzum ${ALL_SOURCES})
endif()

# Use the Widgets module from Qt 5.
target_link_libraries(guzum Qt5::Widgets Qt5::Network Qt5::Test)
#target_link_libraries(guzum Qt5::Widgets Qt5::Svg Qt5::Xml)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
install(TARGETS guzum
    RUNTIME DESTINATION /usr/bin
)
endif()
