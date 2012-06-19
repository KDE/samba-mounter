
kde4_add_executable(samba_helper  sambahelper.cpp)
target_link_libraries(samba_helper ${KDE4_KDECORE_LIBS})
install(TARGETS samba_helper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(samba_helper org.kde.sambamounter root)
kde4_install_auth_actions(org.kde.sambamounter samba-mounter.actions)

kde4_add_executable(samba-realmounter realmounter.cpp)
target_link_libraries(samba-realmounter ${KDE4_KDECORE_LIBS})
install(TARGETS samba-realmounter DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

kde4_add_executable(samba-realumounter realumounter.cpp)
target_link_libraries(samba-realumounter ${KDE4_KDECORE_LIBS})
install(TARGETS samba-realumounter DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})