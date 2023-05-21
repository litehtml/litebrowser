cmake_minimum_required(VERSION 3.11)
project(litebrowser VERSION "0.1.0" LANGUAGES CXX)
include(FetchContent)
find_library(SHLWAPI Shlwapi.lib) 

option(NO_TOOLBAR "No Toolbar" OFF)

# build: release build
#if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#    set(CMAKE_BUILD_TYPE "Release")
#    message(STATUS "Build type not specified: defaulting to release")
#endif()

# module: add cairo
FetchContent_Declare(cairo GIT_REPOSITORY https://github.com/tordex/cairo)
FetchContent_GetProperties(cairo)
if(NOT cairo_POPULATED)
    FetchContent_Populate(cairo)
    add_library(cairo SHARED IMPORTED)
    set_target_properties(cairo PROPERTIES IMPORTED_LOCATION ${cairo_SOURCE_DIR}/64/cairo.lib)
    set_target_properties(cairo PROPERTIES IMPORTED_IMPLIB   ${cairo_SOURCE_DIR}/64/cairo.lib)
    target_include_directories(cairo INTERFACE ${cairo_SOURCE_DIR}/src)
endif()
set(LB_LIBS_INCLUDE_DIRS ${cairo_SOURCE_DIR}/src)
set(LB_LIBS_LIBRARIES cairo)

# module: add txdib simpledib
FetchContent_Declare(txdib GIT_REPOSITORY https://github.com/torde/txdib)
FetchContent_Declare(simpledib GIT_REPOSITORY https://github.com/torde/simpledib)
FetchContent_MakeAvailable(txdib simpledib)
set(LB_LIBS_INCLUDE_DIRS ${LB_LIBS_INCLUDE_DIRS} ${txdib_SOURCE_DIR} ${simpledib_SOURCE_DIR})
set(LB_LIBS_LIBRARIES ${LB_LIBS_LIBRARIES} txdib simpledib)

# module: add litehtml
set(BUILD_TESTING 0)
add_subdirectory(libs/litehtml)

# set source and headers
set(CONTAINERS libs/litehtml/containers)
set(SOURCE
    ${CONTAINERS}/cairo/cairo_container.cpp
    ${CONTAINERS}/cairo/cairo_font.cpp
    src/BrowserWnd.cpp
    src/el_omnibox.cpp
    src/HtmlViewWnd.cpp
    src/litebrowser.cpp
    src/litebrowser.rc
    src/sl_edit.cpp
    src/ToolbarWnd.cpp
    src/tordexhttp.cpp
    src/TxThread.cpp
    src/web_history.cpp
    src/web_page.cpp
)
set(HEADERS
    ${CONTAINERS}/cairo/cairo_container.cpp
    ${CONTAINERS}/cairo/cairo_font.cpp
    src/BrowserWnd.h
    src/ctrl_container.h
    src/el_omnibox.h
    src/globals.h
    src/HtmlViewWnd.h
    src/litebrowser.h
    src/resource.h
    src/sl_edit.h
    src/targetver.h
    src/ToolbarWnd.h
    src/tordexhttp.h
    src/TxThread.h
    src/web_history.h
    src/web_page.h
)
set(LB_LIBS_LIBRARIES ${LB_LIBS_LIBRARIES} gdiplus shlwapi Msimg32)

# executable
add_executable(litebrowser WIN32 ${SOURCE} ${HEADERS})
target_compile_definitions(litebrowser PUBLIC UNICODE PUBLIC _UNICODE)
if (NO_TOOLBAR)
    target_compile_definitions(litebrowser PUBLIC NO_TOOLBAR)
endif()
include_directories(litebrowser
    ${litehtml_SOURCE_DIR}/include
    ${LB_LIBS_INCLUDE_DIRS})
target_link_options(litebrowser
    PRIVATE ${LB_LIBS_LDFLAGS})
target_link_libraries(litebrowser
    litehtml
    ${LB_LIBS_LIBRARIES})
set_target_properties(litebrowser PROPERTIES
    CXX_STANDARD 11
    C_STANDARD 9
)

# msvc: add incremental builds
if(MSVC)
    #target_compile_options(litebrowser PUBLIC "/ZI")
    target_link_options(litebrowser PUBLIC "/INCREMENTAL")
endif()

# copy dll
add_custom_command(TARGET litebrowser POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${cairo_SOURCE_DIR}/64/cairo.dll"
    $<TARGET_FILE_DIR:litebrowser>)
add_custom_command(TARGET litebrowser POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${txdib_SOURCE_DIR}/../freeimage-src/64/freeimage.dll"
    $<TARGET_FILE_DIR:litebrowser>)
