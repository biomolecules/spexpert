project(${spexpert_project_name}_src)

# collect files
set(${spexpert_project_name}_hdr)
set(${spexpert_project_name}_tpp)
set(${spexpert_project_name}_qt_hdr
    appcore.h
    appstate.h
    centralwidget.h
    experimentsetup.h
    exptask.h
    exptasklist.h
    exptasks.h
    lockableqvector.h
    mainwindow.h
    neslab.h
    neslabusmainwidget.h
    plotproxy.h
    stagecontrol.h
    stagesetup.h
    timespan.h
    usmcvb_com.h
    waittask.h
    waittasklist.h
    waittasks.h
    winspec.h
    winspecvb.h)
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
    stagecontrol.cpp
    stagesetup.cpp
    timespan.cpp
    usmcvb_com.cpp
    waittask.cpp
    waittasklist.cpp
    waittasks.cpp
    winspec.cpp
    winspecvb.cpp)
set(${spexpert_project_name}_ui)

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
qt5_wrap_cpp(${spexpert_project_name}_hdr_moc_build ${${spexpert_project_name}_qt_hdr_build})
qt5_wrap_ui(${spexpert_project_name}_ui_moc_build ${${spexpert_project_name}_ui_build})

# create the executable
add_executable(${spexpert_project_name}
    ${${spexpert_project_name}_src_build}
    ${${spexpert_project_name}_hdr_moc_build}
    ${${spexpert_project_name}_ui_moc_build})
target_link_libraries(${spexpert_project_name}
    Qt5::AxContainer
    Qt5::Core
    Qt5::Gui
    Qt5::PrintSupport
    Qt5::Widgets
    Threads::Threads)
target_include_directories(${spexpert_project_name} PRIVATE
    ${spexpert_source_dir}/biomolecules/spexpert
    ${spexpert_source_dir}/external)
set_target_properties(${spexpert_project_name} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_target_properties(${spexpert_project_name} PROPERTIES INSTALL_RPATH "../${CMAKE_INSTALL_LIBDIR}")

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
add_custom_target(copy_spexpert_translations ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages
    COMMAND ${CMAKE_COMMAND} -E copy
        ${spexpert_translations}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages
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
