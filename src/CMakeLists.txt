# save spexpert_source_dir
set(spexpert_source_dir ${CMAKE_CURRENT_LIST_DIR})

# build external libaries
# this have to come first because the other parts of cmake script depends on it
include(${CMAKE_CURRENT_LIST_DIR}/external/CMakeLists.txt)

# spexpert
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/biomolecules/spexpert)
