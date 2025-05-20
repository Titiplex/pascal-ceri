add_library(utils STATIC
        utils.cpp         # ajoutez/retirez vos sources
)

# Headers publics => include/
# Headers privés (si besoin) restent dans utils/src ou utils/include_internal

target_include_directories(utils
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(utils PROPERTIES OUTPUT_NAME utils)

install(TARGETS utils
        EXPORT pascalTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/utils.h DESTINATION include)