# Le tokeniser est généré par Flex à partir de tokeniser.l
find_package(FLEX REQUIRED)

# Fichier d’entrée (.l) et fichier généré (.cpp)
set(FLEX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tokeniser.l)

# Crée la cible Tokeniser qui génère tokeniser.cpp (option -d pour debug)
FLEX_TARGET(
        Tokeniser              # nom logique de la cible
        ${FLEX_SOURCE}         # .l d’entrée
        ${CMAKE_CURRENT_BINARY_DIR}/tokeniser.cpp   # sortie .cpp
        COMPILE_FLAGS "-d"
)

# Librairie statique à partir du code généré
add_library(tokeniser STATIC
        ${FLEX_Tokeniser_OUTPUTS}
)

# Inclusions publiques et privées
#   - include/            (headers API publics)
#   - ${FLEX_INCLUDE_DIRS} (FlexLexer.h)
#   - chemin d’installation

target_include_directories(tokeniser PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)

# Dépendance vers utils et éventuelles libs Flex
# (FLEX_* variables contiennent déjà le bon linker flag sous Windows/Mingw)

target_link_libraries(tokeniser PUBLIC utils ${FLEX_LIBRARY})

install(TARGETS tokeniser
        EXPORT pascalTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/tokeniser.h DESTINATION include)