add_executable(wodox main.c)

link_directories (
  ${LLR_SOURCE_DIR}/core 
  ${LLR_SOURCE_DIR}/media 
  ${LLR_SOURCE_DIR}/menu
  ${LLR_SOURCE_DIR}/play
  ${LLR_SOURCE_DIR}/edit
) 

target_link_libraries (
  wodox 
  menu play edit media core
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${SDL_gfx_LIBRARY}
    ${M_LIB}
)

install(TARGETS wodox DESTINATION bin)

