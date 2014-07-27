
add_executable(samba-onstart main.cpp onstart.cpp)
target_link_libraries(samba-onstart Qt5::Widgets KF5::ConfigCore KF5::Auth)

install(TARGETS samba-onstart DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS sambamounter.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(PROGRAMS sambamounter.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
