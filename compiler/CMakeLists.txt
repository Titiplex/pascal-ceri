# Ce fichier est inclus par le CMakeLists.txt racine

# 1. Exécutable du compilateur Pascal
add_executable(pascal
        src/main.cpp          # point d’entrée
)

target_link_libraries(pascal PRIVATE utils tokeniser parser)

# 2. Alias importé facultatif
add_executable(pascal::exe ALIAS pascal)

add_dependencies(pascal parser_header_gen parser)

# 3. Installation (copie dans <source>/bin)
install(TARGETS pascal
        RUNTIME DESTINATION bin)
