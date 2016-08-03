cmake_minimum_required(VERSION 2.8.12)

project(samba-mounter)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_sambamounter\")

# ECM setup
find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

find_package(KF5 REQUIRED COMPONENTS KCMUtils I18n KIO Auth Solid Config)

add_subdirectory(src)
