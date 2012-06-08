set(sambamount_SRCS
    sambamount.cpp
    mountinfo.cpp
)

qt4_automoc(${sambamount_SRCS})

kde4_add_ui_files(sambamount_SRCS kcm.ui mount.ui)

kde4_add_plugin(sambamount ${sambamount_SRCS})

target_link_libraries(sambamount ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_SOLID_LIBS})

install(TARGETS sambamount DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES sambamount.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(PROGRAMS sambamounterapp.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
add_subdirectory(helper)
add_subdirectory(onstart)