cmake_minimum_required(VERSION 3.20)
project(pascal-ceri LANGUAGES CXX)

# -------- Options globales --------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Répertoires de sortie FINAUX (bin/, lib/) **dans l’arbre source**
set(_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${_LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${_LIB_DIR})
set(CMAKE_INSTALL_PREFIX C:/msys64/home/Titiplex/pascal-ceri)

# S’assurer que chaque configuration (Debug/Release/RelWithDebInfo/MinSizeRel)
foreach (cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${cfg} CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} ${_BIN_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG} ${_LIB_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG} ${_LIB_DIR})
endforeach ()

# Sous‑projets()

add_subdirectory(utils)       # contient utils.cpp / CMakeLists.txt
add_subdirectory(tokeniser)   # génère libtokeniser.a
add_subdirectory(parser)      # génère libparser.a
add_subdirectory(compiler)    # crée l’exécutable pascal
