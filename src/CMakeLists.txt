add_executable(wodox core.c draw.c edit.c lang.c list.c main.c menu.c play.c user.c)

target_link_libraries (
  wodox 
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${SDL_gfx_LIBRARY}
    ${M_LIB}
)

install(TARGETS wodox DESTINATION bin)
