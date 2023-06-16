# Perhaps this could be a common cmake macro inside framework
execute_process(
    COMMAND git describe --abbrev=0 --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE LibraryVersion
    ERROR_VARIABLE gitstatuserr)

string(REPLACE "\n" "" LibraryVersion ${LibraryVersion})
message("Track library version: ${LibraryVersion}")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

file(
    GLOB_RECURSE
    addon_src
    "trackMinimization.c"
    "allocrus.c"
    "edgelen.c"
    "edgeutil.c"
    "heldkarp.c")

set(addon_inc ${CMAKE_CURRENT_SOURCE_DIR}/tsp/inc)

compilelib("")

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
install(FILES ${MAC} DESTINATION ./macros/track)
