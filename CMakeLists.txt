cmake_minimum_required(VERSION 3.19)
project(Secrets LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TS_FILES
    translations/secrets_ru.ts
)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    LinguistTools
    Svg
    Network
)

find_package(OpenSSL REQUIRED)

# Platform-specific dependencies
if(WIN32)
    set(ARGON2_INCLUDE_DIRS
        $ENV{ARGON2_INCLUDE_DIR}
    )

    set(ARGON2_LIBRARIES
        $ENV{ARGON2_LIB}
    )
else()
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(ARGON2 REQUIRED libargon2)

    set(ARGON2_INCLUDE_DIRS
        ${ARGON2_INCLUDE_DIRS}
    )

    set(ARGON2_LIBRARIES
        ${ARGON2_LIBRARIES}
    )
endif()

qt_standard_project_setup()

set(HEADERS
    src/ui/mainwindow.h
    src/domain/node.h
    src/model/treemodel.h
    src/model/treefilterproxymodel.h
    src/infrastructure/xmlstorage.h
    src/application/vaultservice.h
    src/infrastructure/cryptoservice.h
    src/infrastructure/vaultformat.h
    src/infrastructure/vaultheader.h
    src/application/vaultsession.h
    src/application/appsettings.h
    src/infrastructure/settingsstorage.h
    src/shared/systemcontext.h
    src/shared/applicationcontext.h
    src/ui/settingsdialog.h
    src/application/singleinstance.h
    src/application/autostartmanager.h
    src/ui/icons/iconprovider.h
    src/ui/theme/stylesprovider.h
    src/ui/localization/languagemanager.h
    src/ui/theme/palettemode.h
    src/ui/localization/languagemode.h
    src/ui/icons/iconid.h
    src/ui/theme/iconcolor.h
    src/ui/theme/themestyle.h
    src/ui/theme/thememode.h
    src/ui/icons/iconcache.h
    src/ui/icons/iconcachekey.h
)

set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/model/treemodel.cpp
    src/domain/node.cpp
    src/model/treefilterproxymodel.cpp
    src/infrastructure/xmlstorage.cpp
    src/application/vaultservice.cpp
    src/infrastructure/cryptoservice.cpp
    src/infrastructure/vaultformat.cpp
    src/application/vaultsession.cpp
    src/infrastructure/settingsstorage.cpp
    src/ui/settingsdialog.cpp
    src/application/appsettings.cpp
    src/application/autostartmanager.cpp
    src/ui/icons/iconprovider.cpp
    src/ui/theme/stylesprovider.cpp
    src/ui/localization/languagemanager.cpp
    src/ui/icons/iconcache.cpp
)

set(UI_FILES
    src/ui/mainwindow.ui
    src/ui/settingsdialog.ui
)

if(WIN32)
    qt_add_executable(
        Secrets
        WIN32
        ${HEADERS}
        ${SOURCES}
        ${UI_FILES}
        resources.qrc
    )
else()
    qt_add_executable(
        Secrets
        ${HEADERS}
        ${SOURCES}
        ${UI_FILES}
        resources.qrc
    )
endif()

# Windows icon resource
if(WIN32)
    target_sources(
        Secrets
        PRIVATE
        ${CMAKE_SOURCE_DIR}/app_icon.rc
    )
endif()

qt_add_translations(
    Secrets
    TS_FILES ${TS_FILES}
)

target_include_directories(
    Secrets
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${ARGON2_INCLUDE_DIRS}
)

target_link_libraries(
    Secrets
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Svg
    Qt::Network
    OpenSSL::Crypto
    OpenSSL::SSL
    ${ARGON2_LIBRARIES}
)

install(
    TARGETS Secrets
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Linux packaging
if(UNIX AND NOT APPLE)
    install(FILES assets/icons/app/icon_256.png
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME wlfhrtd-secrets.png)

    install(FILES packaging/secrets.desktop
            DESTINATION share/applications)
endif()

# Windows deployment
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET Secrets
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    install(
        SCRIPT ${deploy_script}
    )
endif()
