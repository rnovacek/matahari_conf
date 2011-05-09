cmake_minimum_required(VERSION 2.8)

project(matahari-conf)

option(WITH-DBUS "Build the DBus daemon" ON)
option(WITH-QMF "Build the QMF daemon" ON)

set(CMAKE_C_FLAGS "-Wall -g")

include(FindPkgConfig)

add_library(mconf SHARED conf.c)
pkg_check_modules(AUGEAS REQUIRED augeas)
target_link_libraries(mconf ${AUGEAS_LIBRARIES})

find_package(Matahari REQUIRED)

set(SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schema.xml)

include(MatahariMacros)

if(WITH-QMF)
    find_package(QPID REQUIRED)

    set(SCHEMA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qmf/org/matahariproject/QmfPackage.cpp)

    generate_qmf_schemas(${SCHEMAS} ${SCHEMA_SOURCES})

    add_executable(matahari-qmf-conf matahari-qmf-conf.cpp ${SCHEMA_SOURCES})
    target_link_libraries(matahari-qmf-conf mconf ${MATAHARI_LIBRARY} ${MATAHARI_QMF_LIBRARY} ${QPIDCOMMON_LIBRARY} ${QMF2_LIBRARY} ${QPIDTYPES_LIBRARY} ${QPIDMESSAGING_LIBRARY})
endif(WITH-QMF)

if(WITH-DBUS)
    pkg_check_modules(dbus-glib REQUIRED dbus-glib-1)
    include_directories(${dbus-glib_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

    generate_dbus_interface(${CMAKE_SOURCE_DIR}/schema.xml)
    generate_dbus_headers(conf ${CMAKE_CURRENT_BINARY_DIR}/dbus/org.matahariproject.Config.xml)

    add_executable(matahari-dbus-conf matahari-dbus-conf.c conf-dbus-glue.h)
    target_link_libraries(matahari-dbus-conf mconf ${MATAHARI_LIBRARY} ${MATAHARI_DBUS_LIBRARIES} ${dbus-glib_LIBRARIES})

    install(FILES org.matahariproject.Config.conf DESTINATION /etc/dbus-1/system.d)
    install(FILES org.matahariproject.Config.service DESTINATION share/dbus-1/system-services)
    install(FILES org.matahariproject.Config.policy DESTINATION share/polkit-1/actions)
endif(WITH-DBUS)
