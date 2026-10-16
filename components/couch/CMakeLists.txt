# SPDX-License-Identifier: GPL-3.0-or-later
# Couch co-op (fork only): the shared group camera and its leash, and the
# controller seats and stick walk — header only, no SDL and no game state, so
# the rules are unit-tested alone (`couch`); and the party, several local
# heroes in one game (`couch_party`, over the game's World).
add_library(couch INTERFACE)
target_include_directories(couch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(couch INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(couch_party STATIC party.cpp)
target_link_libraries(couch_party PUBLIC couch game)

d2d_check_headers(couch_headers LINK couch_party)
