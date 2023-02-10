set(LibraryVersion "1.4")
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
