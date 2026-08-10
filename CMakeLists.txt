cmake_minimum_required(VERSION 3.20)

project(SDLHelloWorld VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# SDL is built from the vendored source tree and linked statically.
set(SDL_SHARED OFF CACHE BOOL "Build the SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build the SDL static library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build the SDL test programs" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL test library" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "Generate SDL installation targets" FORCE)
set(SDL_UNINSTALL OFF CACHE BOOL "Generate SDL uninstall target" FORCE)

add_subdirectory(SDL EXCLUDE_FROM_ALL)

# Build SDL_ttf and FreeType into the executable. HarfBuzz and SVG support are
# unnecessary for this simple text demo, so disabling them keeps the build small.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "Use SDL_ttf vendored dependencies" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "Use HarfBuzz text shaping" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "Use PlutoSVG color emoji support" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "Build SDL_ttf samples" FORCE)
set(SDLTTF_INSTALL OFF CACHE BOOL "Generate SDL_ttf install targets" FORCE)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

set(EMBEDDED_FONT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/generated/embedded_font.c")
add_custom_command(
    OUTPUT "${EMBEDDED_FONT_SOURCE}"
    COMMAND "${CMAKE_COMMAND}"
        "-DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/resources/font/MicroSoft_YaHei.ttf"
        "-DOUTPUT_FILE=${EMBEDDED_FONT_SOURCE}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_binary.cmake"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/font/MicroSoft_YaHei.ttf"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_binary.cmake"
    COMMENT "Embedding MicroSoft_YaHei.ttf into the executable"
    VERBATIM
)

add_executable(sdl_hello src/main.c "${EMBEDDED_FONT_SOURCE}")
target_link_libraries(sdl_hello PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static)

if(MINGW)
    # Keep MinGW runtime libraries out of the deployment package.
    target_link_options(sdl_hello PRIVATE -static -static-libgcc)
endif()

set_target_properties(sdl_hello PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${config}" config_upper)
        set_target_properties(sdl_hello PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/bin"
        )
    endforeach()
endif()
