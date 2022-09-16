
add_executable(samba-onstart main.cpp onstart.cpp)
target_link_libraries(samba-onstart Qt5::Widgets Qt5::Network KF5::ConfigCore KF5::Auth)

install(TARGETS samba-onstart DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS sambamounter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(PROGRAMS sambamounter.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
