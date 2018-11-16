###############
##  sprelay  ##
###############

if (NOT MAKE_SPRELAY)
    message(STATUS "Searching for sprelay...")
    find_package(sprelay 1
        HINTS
            ${sprelay_ROOT_DIR}
            $ENV{sprelay_ROOT_DIR})
    if (sprelay_FOUND)
        message(STATUS "sprelay found!")
        if (DOXYGEN_FOUND)
            get_filename_component(sprelay_tags_root ${sprelay_DIR} DIRECTORY)
            get_filename_component(sprelay_tags_root ${sprelay_tags_root} DIRECTORY)
            set(sprelay_tags_root ${sprelay_tags_root}/doc/sprelay)
            find_file(sprelay_tag_file_file sprelay.tags HINTS ${sprelay_tags_root})
            add_custom_target(sprelay_tag_file)
            if (sprelay_tag_file_file)
                set_property(TARGET sprelay_tag_file
                    PROPERTY TAG_FILE "${sprelay_tag_file_file}")
                get_filename_component(sprelay_tags_root ${sprelay_tag_file_file} DIRECTORY)
                if (EXISTS "${sprelay_tags_root}/html")
                    set_property(TARGET sprelay_tag_file
                        PROPERTY DOC_DIR "file://${sprelay_tags_root}/html")
                else()
                    set_property(TARGET sprelay_tag_file
                        PROPERTY DOC_DIR "")
                endif()
            else()
                set_property(TARGET sprelay_tag_file
                    PROPERTY TAG_FILE "")
                set_property(TARGET sprelay_tag_file
                    PROPERTY DOC_DIR "")
            endif()
        endif()
    endif()
endif()
if (NOT sprelay_FOUND)
    message(STATUS "Configuring internal sprelay...")
    if (NOT MAKE_SPRELAY)
        set(MAKE_SPRELAY ON CACHE BOOL "" FORCE)
    endif()
    # setup sprelay at configure time
    # The following commands configures external project without ExternalProject_Add
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/sprelayBuild")
    execute_process(COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_LIST_DIR}/sprelay
            -G "${CMAKE_GENERATOR}"
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
            -Denum_flags_ROOT_DIR=${enum_flags_DIR}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/sprelayBuild"
        RESULT_VARIABLE result)
    if (result)
        message(FATAL_ERROR "CMake step for sprelay failed: ${result}")
    endif()
    add_custom_target(sprelay_build
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/sprelayBuild
        COMMENT "Building sprelay...")
    # The following commands uses ExternalProject_add through at configure time invoked cmake project.
#    configure_file(
#        ${CMAKE_CURRENT_LIST_DIR}/sprelay.CMakeLists.txt.in
#        ${CMAKE_BINARY_DIR}/sprelayCreate/CMakeLists.txt)
#    execute_process(COMMAND ${CMAKE_COMMAND} .
#            -G "${CMAKE_GENERATOR}"
#            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/sprelayCreate)
#    if (result)
#        message(FATAL_ERROR "CMake step for sprelay failed: ${result}")
#    endif()
#    execute_process(COMMAND ${CMAKE_COMMAND} --build .
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/sprelayCreate)
#    if (result)
#        message(FATAL_ERROR "Build step for sprelay failed: ${result}")
#    endif()
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/sprelayBuild)")
    message(STATUS "Subproject: sprelay... DONE")
endif()

if (DOXYGEN_FOUND AND NOT sprelay_FOUND)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sprelayBuild/sprelay.tags
        COMMAND ${CMAKE_COMMAND} --build . --target doc
        # copy the sprelay documentation inside main build folder to make it discoverable also before
        # installation
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/sprelayBuild/doc
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/doc/sprelay
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/sprelayBuild
        COMMENT "Building sprelay documentation...")
    add_custom_target(sprelay_tag_file DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sprelayBuild/sprelay.tags)
    set_property(TARGET sprelay_tag_file
        PROPERTY TAG_FILE ${CMAKE_CURRENT_BINARY_DIR}/sprelayBuild/sprelay.tags)
    set_property(TARGET sprelay_tag_file
        PROPERTY DOC_DIR "")
endif()

if (NOT sprelay_FOUND)
    find_package(sprelay 1
        HINTS ${CMAKE_BINARY_DIR}/sprelayBuild/cmake
        REQUIRED)
endif()
