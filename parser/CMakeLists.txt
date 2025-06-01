# parser/CMakeLists.txt ── génère la lib parser + header ombrelle

# 1) Récupérer les sources & headers internes
file(GLOB PARSER_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB INTERNAL_HEADERS
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# 2) Préparer le dossier include/parser/ à la racine
#    et y copier une seule fois tous les headers internes
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
file(MAKE_DIRECTORY "${PROJECT_ROOT}/include/parser")
file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "${PROJECT_ROOT}/include/parser"
)

# 3) Chemin du header ombrelle (auto-généré)
file(TOUCH "${PROJECT_ROOT}/include/parser.h")
set(UMBRELLA_HDR "${PROJECT_ROOT}/include/parser.h")

file(WRITE "${UMBRELLA_HDR}"
        "// Auto-généré par CMake — ne pas modifier\n"
        "#pragma once\n\n"
)
foreach (h IN LISTS INTERNAL_HEADERS)
    get_filename_component(nom ${h} NAME)
    file(APPEND "${UMBRELLA_HDR}"
            "#include \"parser/${nom}\"\n"
    )
endforeach ()

# 5) Cible factice pour s’assurer que parser.h est généré avant compilation
add_custom_target(parser_header_gen
        DEPENDS ${UMBRELLA_HDR}
)

# 6) Lib statique parser (avec le header ombrelle)
add_library(parser STATIC
        ${PARSER_SOURCES}
        ${UMBRELLA_HDR}
)
add_dependencies(parser parser_header_gen)

# 7) Include paths
target_include_directories(parser
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 8) Liaisons
target_link_libraries(parser
        PUBLIC tokeniser utils
)

# 9) Installation
install(TARGETS parser
        EXPORT pascalTargets
        ARCHIVE DESTINATION lib
)
install(DIRECTORY
        ${PROJECT_ROOT}/include/parser/
        DESTINATION include/parser
        FILES_MATCHING PATTERN "*.h"
)
install(FILES
        ${UMBRELLA_HDR}
        DESTINATION include
)
