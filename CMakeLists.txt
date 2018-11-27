cmake_minimum_required(VERSION 3.3.2)
project(spexpert VERSION 1.0.0 LANGUAGES C CXX)

option(MOCK_CALIBRATION_STAGE
    "Build the project with mock calibration stage. Set it if you don't have or don't want to use real calibration
        stage."
    OFF)

option(MOCK_NESLAB_BATH
    "Build the project with mock Neslab bath. Set it if you don't have or don't want to use real Neslab bath."
    OFF)

option(MOCK_WINSPEC
    "Build the project with mock Winspec. Set it if you don't have or don't want to use real WinSpec."
    OFF)

option(MAKE_ENUM_FLAGS
    "Forces internal enum_flags library usage. Otherwise the library is searched if sytem path. You can also specify
    its location by enum_flags_ROOT_DIR cache variable."
    OFF)

if (NOT MAKE_ENUM_FLAGS)
    set(enum_flags_ROOT_DIR "" CACHE PATH "Location hint for enum_flags library search.")
endif()

option(MAKE_SPRELAY
    "Forces internal sprelay library usage. Otherwise the library is searched if sytem path. You can also specify
    its location by sprelay_ROOT_DIR cache variable."
    OFF)

if (NOT MAKE_SPRELAY)
    set(sprelay_ROOT_DIR "" CACHE PATH "Location hint for sprelay library search.")
endif()

set(CPPREFERENCE_TAGS_ROOT_DIR "" CACHE PATH "Location hint for the cppreference doxygen tags search.")

# set some globals
set(spexpert_project_name spexpert)
set(spexpert_root_binary_dir ${CMAKE_BINARY_DIR})
set(spexpert_root_source_dir ${PROJECT_SOURCE_DIR})
set(spexpert_install_doc_dir "${CMAKE_INSTALL_DATAROOTDIR}/doc/${spexpert_project_name}")
if (MOCK_CALIBRATION_STAGE AND MOCK_WINSPEC)
    set(omit_microsoft_com TRUE)
endif()

# object files with dependencies works well only for cmake version 3.12 or later else the object files do not propagate
# dependencies and cant use target_link_libraries
if (NOT CMAKE_VERSION VERSION_LESS "3.12")
    set(use_object_targets TRUE)
endif()

# setup compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-pipe -frtti -Wall -Wextra -Wpedantic -fexceptions)
endif()

if (MSVC)
    add_compile_options(/W4)
endif()

if (MINGW)
    add_compile_options(-mthreads -fno-keep-inline-dllexport -DUNICODE)
    list(APPEND spexpert_standalone_application_link_flags "-Wl,-subsystem,windows")
    list(APPEND spexpert_standalone_console_link_flags "-Wl,-subsystem,console")
endif()

# Add Qt
if (omit_microsoft_com)
    find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort PrintSupport REQUIRED)
else()
    find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort PrintSupport AxContainer REQUIRED)
endif()

# Add threading
find_package(Threads REQUIRED)

# check for Doxygen
find_package(Doxygen)

# install correct libraries into correct locations on all platforms
include(GNUInstallDirs)

# enable running program from build directory by copying all binary staff into bin subdirectory
if (WIN32)
    if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${spexpert_root_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${spexpert_root_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${spexpert_root_binary_dir}/${CMAKE_INSTALL_BINDIR}")
    endif()
endif()

# execute all parts of projects placed in subdirectories
include(src/CMakeLists.txt)

# documentation
if (DOXYGEN_FOUND)
    # find cppreference file
    find_file(cppreference_tag_file cppreference-doxygen-web.tag.xml
        HINTS
            ${CPPREFERENCE_TAGS_ROOT_DIR}
            $ENV{CPPREFERENCE_TAGS_ROOT_DIR})
    if (cppreference_tag_file)
        set(cppreference_tags "\"${cppreference_tag_file}=http://en.cppreference.com/w/\"")
    else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml
            COMMAND ${CMAKE_COMMAND}
                -D DEST_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cppreference_tags.cmake
            COMMENT "Getting cppreference.com doxygen tags...")
        add_custom_target(get_ccpreference_tags DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml)
        set(cppreference_tags
            "\"${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/\"")
    endif()

    # get Qt documentation location
    get_target_property(qmake_executable Qt5::qmake LOCATION)
    execute_process(
        COMMAND ${qmake_executable} -query QT_INSTALL_DOCS
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt_docs_dir
    )

    # link in qts own documentation
    string(STRIP ${qt_docs_dir} qt_docs_dir)
    set(qt_core_tags "${qt_docs_dir}/qtcore/qtcore.tags=http://doc.qt.io/qt-5/")
    set(qt_gui_tags "${qt_docs_dir}/qtgui/qtgui.tags=http://doc.qt.io/qt-5/")
    set(qt_widgets_tags "${qt_docs_dir}/qtwidgets/qtwidgets.tags=http://doc.qt.io/qt-5/")
    set(qt_print_support_tags "${qt_docs_dir}/qtprintsupport/qtprintsupport.tags=http://doc.qt.io/qt-5/")

    # enum_flags documentation
    get_target_property(enum_flags_tags enum_flags_tag_file TAG_FILE)
    get_target_property(enum_flags_doc_dir enum_flags_tag_file DOC_DIR)
    if (NOT "${enum_flags_doc_dir}" STREQUAL "")
        set(enum_flags_tags "${enum_flags_tags}=${enum_flags_doc_dir}")
    elseif (NOT "${enum_flags_tags}" STREQUAL "")
        set(enum_flags_tags "${enum_flags_tags}=../../${CMAKE_INSTALL_DATAROOTDIR}/doc/enum_flags/html/")
    else()
        set(enum_flags_tags "")
    endif()

    # sprelay documentation
    get_target_property(sprelay_tags sprelay_tag_file TAG_FILE)
    get_target_property(sprelay_doc_dir sprelay_tag_file DOC_DIR)
    if (NOT "${sprelay_doc_dir}" STREQUAL "")
        set(sprelay_tags "${sprelay_tags}=${sprelay_doc_dir}")
    elseif (NOT "${sprelay_tags}" STREQUAL "")
        set(sprelay_tags "${sprelay_tags}=../../${CMAKE_INSTALL_DATAROOTDIR}/doc/sprelay/html/")
    else()
        set(sprelay_tags "")
    endif()
    get_filename_component(spexpert_parent_dir ${spexpert_root_source_dir} DIRECTORY)

    # configure doxygen
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(doxy_main_page ${CMAKE_CURRENT_SOURCE_DIR}/src/biomolecules/spexpert/index.md)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
    add_dependencies(doc enum_flags_tag_file)
    add_dependencies(doc sprelay_tag_file)
    if (NOT cppreference_tag_file)
        add_dependencies(doc get_ccpreference_tags)
    endif()

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html DESTINATION ${spexpert_install_doc_dir} OPTIONAL)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.tags
        DESTINATION ${spexpert_install_doc_dir} OPTIONAL)
endif()
