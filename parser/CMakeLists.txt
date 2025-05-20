# Plusieurs .cpp / .h internes ; un **seul** header public `parser.h` installé

# 1. Récolter les sources & headers internes
file(GLOB PARSER_SOURCES  CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB INTERNAL_HEADERS CONFIGURE_DEPENDS "include/*.h")

# 2. Générer le header "ombrelle" dans <root>/include/parser.h
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "${PROJECT_ROOT}/include/parser")
set(UMBRELLA_HDR "${PROJECT_ROOT}/include/parser.h")
file(WRITE  "${UMBRELLA_HDR}" "// Auto‑généré par CMake ‑‑ ne pas modifier
#pragma once

")

foreach(h ${INTERNAL_HEADERS})
    # Inclure chaque header interne
    get_filename_component(rel ${h} NAME)          # ex: function.h
    file(APPEND "${UMBRELLA_HDR}" "#include \"parser/${rel}\"\n"
    )
endforeach()

# 3. Bibliothèque statique
add_library(parser STATIC
        ${PARSER_SOURCES}
        ${INTERNAL_HEADERS}
)

# 4. Inclusions
#    - <build>/include  : expose parser.h (public)
#    - parser/include   : headers internes visibles pour la lib, mais pas installés

# Public : les clients voient uniquement <root>/include
# Private : compilation de la lib voit aussi parser/include

target_include_directories(parser
        PUBLIC  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/parser>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        INTERFACE $<INSTALL_INTERFACE:include/parser>
)

target_link_libraries(parser PUBLIC tokeniser utils)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include/parser
        FILES_MATCHING PATTERN "*.h")

# 5. Installation : *un seul* header public
install(TARGETS parser
        EXPORT pascalTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES "${UMBRELLA_HDR}" DESTINATION include)