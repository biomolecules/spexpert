##################
##  enum_flags  ##
##################

if (NOT MAKE_ENUM_FLAGS)
    message(STATUS "Searching for enum_flags...")
    find_package(enum_flags 3
        COMPONENTS Develop
        HINTS
            ${enum_flags_ROOT_DIR}
            $ENV{enum_flags_ROOT_DIR})
    if (enum_flags_FOUND)
        message(STATUS "enum_flags found!")
        if (DOXYGEN_FOUND)
            get_filename_component(enum_flags_tags_root ${enum_flags_DIR} DIRECTORY)
            get_filename_component(enum_flags_tags_root ${enum_flags_tags_root} DIRECTORY)
            set(enum_flags_tags_root ${enum_flags_tags_root}/doc/enum_flags)
            find_file(enum_flags_tag_file_file enum_flags.tags HINTS ${enum_flags_tags_root})
            add_custom_target(enum_flags_tag_file)
            if (enum_flags_tag_file_file)
                set_property(TARGET enum_flags_tag_file
                    PROPERTY TAG_FILE "${enum_flags_tag_file_file}")
                get_filename_component(enum_flags_tags_root ${enum_flags_tag_file_file} DIRECTORY)
                if (EXISTS "${enum_flags_tags_root}/html")
                    set_property(TARGET enum_flags_tag_file
                        PROPERTY DOC_DIR "file://${enum_flags_tags_root}/html")
                else()
                    set_property(TARGET enum_flags_tag_file
                        PROPERTY DOC_DIR "")
                endif()
            else()
                set_property(TARGET enum_flags_tag_file
                    PROPERTY TAG_FILE "")
                set_property(TARGET enum_flags_tag_file
                    PROPERTY DOC_DIR "")
            endif()
        endif()
    endif()
endif()
if (NOT enum_flags_FOUND)
    message(STATUS "Configuring internal enum_flags...")
    # setup enum_flags at configure time
    # The following commands configures external project without ExternalProject_Add
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/enum_flagsBuild")
    execute_process(COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_LIST_DIR}/enum_flags
            -G "${CMAKE_GENERATOR}"
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/enum_flagsBuild"
        RESULT_VARIABLE result)
    if (result)
        message(FATAL_ERROR "CMake step for enum_flags failed: ${result}")
    endif()
    # The following commands uses ExternalProject_add through at configure time invoked cmake project.
#    configure_file(
#        ${CMAKE_CURRENT_LIST_DIR}/enum_flags.CMakeLists.txt.in
#        ${CMAKE_BINARY_DIR}/enum_flagsCreate/CMakeLists.txt)
#    execute_process(COMMAND ${CMAKE_COMMAND} .
#            -G "${CMAKE_GENERATOR}"
#            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsCreate)
#    if (result)
#        message(FATAL_ERROR "CMake step for enum_flags failed: ${result}")
#    endif()
#    execute_process(COMMAND ${CMAKE_COMMAND} --build .
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsCreate)
#    if (result)
#        message(FATAL_ERROR "Build step for enum_flags failed: ${result}")
#    endif()
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsBuild)")
    message(STATUS "Subproject: enum_flags... DONE")
endif()

if (DOXYGEN_FOUND AND NOT enum_flags_FOUND)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/enum_flagsBuild/enum_flags.tags
        COMMAND ${CMAKE_COMMAND} --build . --target doc
        # copy the enum_flags documentation inside main build folder to make it discoverable also before
        # installation
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/enum_flagsBuild/doc
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/doc/enum_flags
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsBuild
        COMMENT "Building enum_flags documentation...")
    add_custom_target(enum_flags_tag_file DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/enum_flagsBuild/enum_flags.tags)
    set_property(TARGET enum_flags_tag_file
        PROPERTY TAG_FILE ${CMAKE_CURRENT_BINARY_DIR}/enum_flagsBuild/enum_flags.tags)
    set_property(TARGET enum_flags_tag_file
        PROPERTY DOC_DIR "")
endif()

if (NOT enum_flags_FOUND)
    find_package(enum_flags 3
        COMPONENTS Develop
        HINTS ${CMAKE_BINARY_DIR}/enum_flagsBuild
        REQUIRED)
endif()
