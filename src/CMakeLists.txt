set(sambamount_SRCS
    sambamount.cpp
    mountinfo.cpp
)

ki18n_wrap_ui(sambamount_SRCS kcm.ui mount.ui)

set_property(SOURCE org.kde.KPasswdServer.xml PROPERTY INCLUDE "KIO/AuthInfo")
qt5_add_dbus_interface(sambamount_SRCS org.kde.KPasswdServer.xml kpasswdserver_interface)

add_library(kcm_sambamount MODULE ${sambamount_SRCS})

target_link_libraries(kcm_sambamount
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::Auth
    KF5::Solid
    KF5::ConfigCore
    Qt5::DBus
)

install(TARGETS kcm_sambamount DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES sambamount.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(PROGRAMS sambamounterapp.desktop DESTINATION ${KDE_INSTALL_APPDIR})

add_subdirectory(helper)
add_subdirectory(onstart)
