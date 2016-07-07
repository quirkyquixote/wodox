Wodox - A puzzle game influenced by Sokoban
===========================================

Requirements
------------
Wodox requires:

- C compiler supporting the C99 standard
- SDL
- SDL_image
- SDL_mixer
- SDL_ttf
- SDL_gfx

Installation
------------
Wodox provides CMake scripts. To build from scratch execute the following:

    mkdir build
    cd build
    cmake ..
    make
    make install

Running 
-------

    Usage: wodox --help wodox --version
           wodox [<options>] --edit <file> ...
           wodox [<options>] --play <file> ...
           wodox [<options>]
    
    See aditional options with wodox --help
