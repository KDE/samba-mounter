
add_executable(samba_helper  sambahelper.cpp)
target_link_libraries(samba_helper Qt5::Core KF5::Auth)
install(TARGETS samba_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(samba_helper org.kde.sambamounter root)
kauth_install_actions(org.kde.sambamounter samba-mounter.actions)

add_executable(samba-realmounter realmounter.cpp)
target_link_libraries(samba-realmounter Qt5::Core)
install(TARGETS samba-realmounter DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_executable(samba-realumounter realumounter.cpp)
target_link_libraries(samba-realumounter Qt5::Core)
install(TARGETS samba-realumounter DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
