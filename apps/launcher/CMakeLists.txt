# SPDX-License-Identifier: GPL-3.0-or-later
# Qt6 launcher app. Handles installation (from ISOs and/or an existing D2
# folder) today; will handle profile selection + game launch once the engine
# can run. find_package picks up Homebrew qt6, Ubuntu qt6-base-dev, and the
# official Qt SDK on Windows.

find_package(Qt6 6.5 COMPONENTS Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)

# --- version stamp (git describe → config.hpp) ------------------------------
find_package(Git QUIET)
set(D2LAUNCHER_VERSION "0.1.0-dev")
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE _tmp
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(_tmp)
        string(REGEX REPLACE "-[0-9]+-g" "-g" D2LAUNCHER_VERSION "${_tmp}")   # drop commits-since-tag
    endif()
    # Re-stamp on each commit/checkout, not only on a fresh configure.
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git/logs/HEAD")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.git/logs/HEAD")
    endif()
endif()
# Fork: a source tree without .git (the Flatpak build's `git archive`)
# carries the version CI wrote beside it.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git" AND EXISTS "${CMAKE_SOURCE_DIR}/VERSION")
    file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION" D2LAUNCHER_VERSION LIMIT_COUNT 1)
endif()

# A GitHub Releases API URL (or similar); empty = launcher does no update
# check. Fork: our releases, a list since they are prereleases (main.cpp).
set(D2LAUNCHER_UPDATE_URL "https://api.github.com/repos/jarvis-jhk/diablo2-couch/releases?per_page=5" CACHE STRING
    "Optional URL returning JSON with 'tag_name' for update check")
set(D2_PATCH_URL "" CACHE STRING
    "Optional URL to fetch a Blizzard patch installer from")
set(D2D_SENTRY_DSN
    ""
    CACHE STRING "Public Sentry client DSN")

configure_file(config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.hpp @ONLY)

set(icon_dir "${PROJECT_SOURCE_DIR}/apps/icon")
set_source_files_properties("${icon_dir}/d2d.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
add_executable(d2d-launcher WIN32 MACOSX_BUNDLE
    main.cpp
    "${icon_dir}/d2d.icns"
    $<$<PLATFORM_ID:Windows>:${icon_dir}/d2d.rc>
)
set_target_properties(d2d-launcher PROPERTIES MACOSX_BUNDLE_ICON_FILE d2d.icns)
qt_add_resources(d2d-launcher icon PREFIX "/" BASE "${icon_dir}" FILES "${icon_dir}/d2d.png" "${icon_dir}/hero.png")
# Fork: what's new, shown once after an update.
qt_add_resources(d2d-launcher changelog PREFIX "/" BASE "${PROJECT_SOURCE_DIR}" FILES "${PROJECT_SOURCE_DIR}/CHANGELOG.md")

target_include_directories(d2d-launcher PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(d2d-launcher PRIVATE
    Qt6::Widgets
    Qt6::Network
    iso9660
    mpq
    install_detect
    userdir
    couch
)

# The package name (apps/CMakeLists.txt) carries the same version.
set(D2D_VERSION "${D2LAUNCHER_VERSION}" PARENT_SCOPE)
