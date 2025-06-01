# generate-umbrella.cmake
#   Variables fournies en ligne de commande :
#     OUT    -> chemin du fichier à écrire
#     INPUTS -> liste des headers internes (séparés par ;)

# Écrire le préambule
file(WRITE "${OUT}"
        "// Auto-généré par CMake — ne pas modifier\n"
        "#pragma once\n\n"
)

# Inclure chaque header interne depuis include/parser/
foreach (h IN LISTS INPUTS)
    get_filename_component(nom ${h} NAME)
    file(APPEND "${OUT}"
            "#include \"parser/${nom}\"\n"
    )
endforeach ()
