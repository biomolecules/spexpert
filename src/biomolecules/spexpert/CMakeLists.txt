project(${spexpert_project_name}_src)

# collect files
set(${spexpert_project_name}_hdr
    lockableqvector.h
    relay.h
    timespan.h
    winspec.h)
set(${spexpert_project_name}_tpp)
set(${spexpert_project_name}_qt_hdr
    appcore.h
    appstate.h
    centralwidget.h
    experimentsetup.h
    exptask.h
    exptasklist.h
    exptasks.h
    mainwindow.h
    neslab.h
    neslabusmainwidget.h
    plotproxy.h
    relay_options_widgets.h
    stagecontrol.h
    stagesetup.h
    waittask.h
    waittasklist.h
    waittasks.h)
set(${spexpert_project_name}_src
    appcore.cpp
    appstate.cpp
    centralwidget.cpp
    experimentsetup.cpp
    exptask.cpp
    exptasklist.cpp
    exptasks.cpp
    lockableqvector.cpp
    main.cpp
    mainwindow.cpp
    neslab.cpp
    neslabusmainwidget.cpp
    plotproxy.cpp
    relay_options_widgets.cpp
    stagecontrol.cpp
    stagesetup.cpp
    timespan.cpp
    waittask.cpp
    waittasklist.cpp
    waittasks.cpp
    winspec.cpp)
set(${spexpert_project_name}_ui)
if (NOT omit_microsoft_com)
    set(${spexpert_project_name}_hdr ${${spexpert_project_name}_hdr}
        usmcvb_com.h
        winspecvb.h)
    set(${spexpert_project_name}_src ${${spexpert_project_name}_src}
        usmcvb_com.cpp
        winspecvb.cpp)
else()
    set(${spexpert_project_name}_hdr ${${spexpert_project_name}_hdr}
        mock_winspec.h)
    set(${spexpert_project_name}_src ${${spexpert_project_name}_src}
        mock_winspec.cpp)
endif()

# create build file paths
foreach(hdr ${${spexpert_project_name}_hdr})
    list(APPEND ${spexpert_project_name}_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(hdr ${${spexpert_project_name}_tpp})
    list(APPEND ${spexpert_project_name}_tpp_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(hdr ${${spexpert_project_name}_qt_hdr})
    list(APPEND ${spexpert_project_name}_qt_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(src ${${spexpert_project_name}_src})
    list(APPEND ${spexpert_project_name}_src_build "${CMAKE_CURRENT_LIST_DIR}/${src}")
endforeach()
foreach(src ${${spexpert_project_name}_ui})
    list(APPEND ${spexpert_project_name}_ui_build "${CMAKE_CURRENT_LIST_DIR}/${src}")
endforeach()

list(APPEND ${spexpert_project_name}_qt_hdr_build "${spexpert_source_dir}/external/qcustomplot/qcustomplot.h")
list(APPEND ${spexpert_project_name}_src_build "${spexpert_source_dir}/external/qcustomplot/qcustomplot.cpp")

# call qt moc
if (MOCK_CALIBRATION_STAGE)
    set(${spexpert_project_name}_moc_defines ${${spexpert_project_name}_moc_defines}
        -DSPEXPERT_MOCK_CALIBRATION_STAGE)
endif()
if (MOCK_NESLAB_BATH)
    set(${spexpert_project_name}_moc_defines ${${spexpert_project_name}_moc_defines}
        -DSPEXPERT_MOCK_NESLAB_BATH)
endif()
if (MOCK_WINSPEC)
    set(${spexpert_project_name}_moc_defines ${${spexpert_project_name}_moc_defines}
        -DSPEXPERT_MOCK_WINSPEC)
endif()
if (${spexpert_project_name}_moc_defines)
    qt5_wrap_cpp(${spexpert_project_name}_hdr_moc_build ${${spexpert_project_name}_qt_hdr_build}
        OPTIONS ${${spexpert_project_name}_moc_defines})
    qt5_wrap_ui(${spexpert_project_name}_ui_moc_build ${${spexpert_project_name}_ui_build}
        OPTIONS ${${spexpert_project_name}_moc_defines})
else()
    qt5_wrap_cpp(${spexpert_project_name}_hdr_moc_build ${${spexpert_project_name}_qt_hdr_build})
    qt5_wrap_ui(${spexpert_project_name}_ui_moc_build ${${spexpert_project_name}_ui_build})
endif()

# create the executable
add_executable(${spexpert_project_name}
    ${${spexpert_project_name}_src_build}
    ${${spexpert_project_name}_hdr_moc_build}
    ${${spexpert_project_name}_ui_moc_build})
target_link_libraries(${spexpert_project_name}
    Qt5::Core
    Qt5::Gui
    Qt5::PrintSupport
    Qt5::Widgets
    Threads::Threads
    biomolecules::sprelay::sprelay_core
    biomolecules::sprelay::sprelay)
if (NOT omit_microsoft_com)
    target_link_libraries(${spexpert_project_name}
        Qt5::AxContainer)
endif()
target_include_directories(${spexpert_project_name} PRIVATE
    ${spexpert_source_dir}/biomolecules/spexpert
    ${spexpert_source_dir}/external)
if (MAKE_SPRELAY)
    add_dependencies(${spexpert_project_name} sprelay_build)
endif()
set_target_properties(${spexpert_project_name} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_target_properties(${spexpert_project_name} PROPERTIES INSTALL_RPATH "../${CMAKE_INSTALL_LIBDIR}")
if (MOCK_CALIBRATION_STAGE)
    target_compile_definitions(${spexpert_project_name} PRIVATE SPEXPERT_MOCK_CALIBRATION_STAGE)
endif()
if (MOCK_NESLAB_BATH)
    target_compile_definitions(${spexpert_project_name} PRIVATE SPEXPERT_MOCK_NESLAB_BATH)
endif()
if (MOCK_WINSPEC)
    target_compile_definitions(${spexpert_project_name} PRIVATE SPEXPERT_MOCK_WINSPEC)
endif()
# add possible link flags
if (spexpert_standalone_application_link_flags)
    set_target_properties(${spexpert_project_name} PROPERTIES LINK_FLAGS
        ${spexpert_standalone_application_link_flags})
endif()

# attach header files to the library (mainly to display them in IDEs)
target_sources(${spexpert_project_name} PRIVATE
    $<BUILD_INTERFACE:${${spexpert_project_name}_hdr_build}>
    $<BUILD_INTERFACE:${${spexpert_project_name}_tpp_build}>
    $<BUILD_INTERFACE:${${spexpert_project_name}_qt_hdr_build}>)

# install the binary
install(TARGETS ${spexpert_project_name}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(spexpert_translations
    ${spexpert_source_dir}/biomolecules/spexpert/languages/spexpert_en.qm
    ${spexpert_source_dir}/biomolecules/spexpert/languages/spexpert_cs.qm)

# copy translations to binary dir
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(${spexpert_project_name}_languages_dir ${CMAKE_BINARY_DIR}/languages)
else()
    set(${spexpert_project_name}_languages_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages)
endif()
add_custom_target(copy_spexpert_translations ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${${spexpert_project_name}_languages_dir}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${spexpert_translations}
        ${${spexpert_project_name}_languages_dir}
    COMMENT "Copying translations...")

install(
    DIRECTORY
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages"
    DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Documentation sources - Qt creator does not support interface libraries so the documentation sources are added to
# normal targets
if (DOXYGEN_FOUND)
    set(${spexpert_project_name}_doc_src
        ${PROJECT_SOURCE_DIR}/index.md)
    target_sources(${spexpert_project_name} PRIVATE ${${spexpert_project_name}_doc_src})
    set_source_files_properties(${${spexpert_project_name}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

