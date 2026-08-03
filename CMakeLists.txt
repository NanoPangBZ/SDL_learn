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

add_executable(sdl_hello src/main.c)
target_link_libraries(sdl_hello PRIVATE SDL3::SDL3-static)

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
