
kde4_add_executable(samba_helper  sambahelper.cpp)
target_link_libraries(samba_helper ${KDE4_KDECORE_LIBS})
install(TARGETS samba_helper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(samba_helper org.kde.sambamounter root)
kde4_install_auth_actions(org.kde.sambamounter samba-mounter.actions)