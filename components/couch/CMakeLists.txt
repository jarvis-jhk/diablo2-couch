# SPDX-License-Identifier: GPL-3.0-or-later
# Couch co-op (fork only): the shared group camera and its leash, and the
# controller seats and stick walk. Header only, no SDL and no game state, so
# the rules are unit-tested alone.
add_library(couch INTERFACE)
target_include_directories(couch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(couch INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
d2d_check_headers(couch_headers LINK couch)
