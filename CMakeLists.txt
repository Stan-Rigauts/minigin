cmake_minimum_required(VERSION 3.26)
project(MsPacman)
cmake_policy(SET CMP0144 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# SDL3
find_package(SDL3 3.4 CONFIG QUIET)
if(NOT SDL3_FOUND)
    if(WIN32)
        FetchContent_Declare(
            SDL3
            URL https://www.libsdl.org/release/SDL3-devel-3.4.0-VC.zip
        )
        FetchContent_MakeAvailable(SDL3)
        set(SDL3_DIR "${sdl3_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)
        find_package(SDL3 CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.4.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

# SDL3_ttf
if(EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_FREETYPE=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_FREETYPE=1")
    set(FREETYPE_INCLUDE_DIRS "${EMSCRIPTEN_SYSROOT}/include/freetype2")
    set(FREETYPE_FOUND TRUE)
    set(SDL3TTF_HARFBUZZ OFF CACHE INTERNAL "" FORCE)
endif()

find_package(SDL3_ttf 3.2.2 CONFIG QUIET)
if(NOT SDL3_ttf_FOUND)
    if(WIN32)
        FetchContent_Declare(
            SDL3_ttf
            URL https://www.libsdl.org/projects/SDL_ttf/release/SDL3_ttf-devel-3.2.2-VC.zip
        )
        FetchContent_MakeAvailable(SDL3_ttf)
        set(SDL3_ttf_DIR "${sdl3_ttf_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)
        find_package(SDL3_ttf CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3_ttf
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
            GIT_TAG release-3.2.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3_ttf)
    endif()
endif()

# SDL3_mixer
set(SDL3MIXER_VENDORED ON CACHE BOOL "" FORCE)
set(SDL3MIXER_OGG OFF CACHE BOOL "" FORCE)
set(SDL3MIXER_MP3 OFF CACHE BOOL "" FORCE)
set(SDL3MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL3MIXER_MOD OFF CACHE BOOL "" FORCE)

find_package(SDL3_mixer 3.2.0 CONFIG QUIET)
if(NOT SDL3_mixer_FOUND)
    if(WIN32)
        FetchContent_Declare(
            SDL3_mixer
            URL https://www.libsdl.org/projects/SDL_mixer/release/SDL3_mixer-devel-3.2.0-VC.zip
        )
        FetchContent_MakeAvailable(SDL3_mixer)
        set(SDL3_mixer_DIR "${sdl3_mixer_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)
        find_package(SDL3_mixer CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            SDL3_mixer
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
            GIT_TAG release-3.2.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3_mixer)
    endif()
endif()
# IMGUI
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.92.5-docking
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${SDL3_SOURCE_DIR}/include
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_compile_features(imgui PUBLIC cxx_std_20)

# ENGINE (static library)
add_library(Engine STATIC
    Engine/CacheTrasher.cpp
    Engine/CacheTrasher.h
    Engine/Font.cpp
    Engine/Font.h
    Engine/FPSComponent.cpp
    Engine/FPSComponent.h
    Engine/GameObject.cpp
    Engine/GameObject.h
    Engine/imgui_plot.cpp
    Engine/imgui_plot.h
    Engine/InputManager.cpp
    Engine/InputManager.h
    Engine/Minigin.cpp
    Engine/Minigin.h
    Engine/null_sound_system.h
    Engine/Observer.h
    Engine/Renderer.cpp
    Engine/Renderer.h
    Engine/ResourceManager.cpp
    Engine/ResourceManager.h
    Engine/RotatorComponent.h
    Engine/Scene.cpp
    Engine/Scene.h
    Engine/SceneManager.cpp
    Engine/SceneManager.h
    Engine/sdl_sound_system.cpp
    Engine/sdl_sound_system.h
    Engine/servicelocator.cpp
    Engine/servicelocator.h
    Engine/Singleton.h
    Engine/sound_system.h
    Engine/Subject.cpp
    Engine/Subject.h
    Engine/TextComponent.cpp
    Engine/TextComponent.h
    Engine/Texture2D.cpp
    Engine/Texture2D.h
    Engine/ThrashTheCacheComponent.cpp
    Engine/ThrashTheCacheComponent.h
    Engine/Transform.cpp
    Engine/Transform.h
    Engine/BaseCommand.h
    Engine/Component.h
)

target_include_directories(Engine PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
    ${glm_SOURCE_DIR}
    ${SDL3_SOURCE_DIR}/include
    ${SDL3_ttf_SOURCE_DIR}/include
    ${SDL3_mixer_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
)

target_link_libraries(Engine PUBLIC
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    imgui
    glm::glm
)

target_compile_features(Engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(Engine PRIVATE /W4 /WX)
else()
    target_compile_options(Engine PRIVATE -Wall -Wextra -Werror)
endif()

# GAME (executable)
if(EMSCRIPTEN)
    message("Using Emscripten!")
    set(TARGET_NAME minigin_web)
    set(TARGET_PARAM "")
else()
    set(TARGET_NAME MsPacman)
    set(TARGET_PARAM WIN32)
endif()

add_executable(${TARGET_NAME} ${TARGET_PARAM}
    MsPacman/Main.cpp
    MsPacman/Command.cpp
    MsPacman/Command.h
    MsPacman/MoveComponent.cpp
    MsPacman/MoveComponent.h
    MsPacman/GameManagerComponent.cpp
    MsPacman/GameManagerComponent.h
    MsPacman/HealthComponent.cpp
    MsPacman/HealthComponent.h
    MsPacman/PelletComponent.cpp
    MsPacman/PelletComponent.h
    MsPacman/ScoreComponent.cpp
    MsPacman/ScoreComponent.h
    MsPacman/GhostState.h
    MsPacman/ChaseState.h
    MsPacman/ChaseState.cpp
    MsPacman/ScatterState.h
    MsPacman/ScatterState.cpp
    MsPacman/FrightenedState.h
    MsPacman/FrightenedState.cpp
    MsPacman/DeadState.h
    MsPacman/DeadState.cpp
    MsPacman/ExitHouseState.h
    MsPacman/Ghost.h
    MsPacman/Ghost.cpp
    MsPacman/SpriteAnimatorComponent.h
    MsPacman/SpriteAnimatorComponent.cpp
    MsPacman/GameState.h
    MsPacman/GameEvent.h
    MsPacman/GridComponent.h
    MsPacman/HighScoreManager.h
    MsPacman/LevelLoader.h
    MsPacman/LivesDisplayComponent.h
    MsPacman/NameEntryComponent.cpp
    MsPacman/NameEntryComponent.h
    MsPacman/Observer.h
    MsPacman/PlayerControllerComponent.h
    MsPacman/ScoreDisplayComponent.h
    MsPacman/SoundIds.h
    MsPacman/SteamAchievementsObserver.h
    MsPacman/TileType.h
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/MsPacman
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Engine
    $<$<PLATFORM_ID:Windows>:xinput>
)

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

# VISUAL STUDIO STARTUP
if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
endif()

# RUNTIME DLL COPY (Windows)
if(WIN32)
    set(SDL_RUNTIME_DLLS
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        SDL3_mixer::SDL3_mixer
    )
    foreach(dllTarget IN LISTS SDL_RUNTIME_DLLS)
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${dllTarget}>"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        )
    endforeach()
endif()

# EMSCRIPTEN
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${TARGET_NAME} PRIVATE
        -sASYNCIFY
        -sALLOW_MEMORY_GROWTH=1
        "SHELL:-sUSE_FREETYPE=1"
        --preload-file ${CMAKE_SOURCE_DIR}/Data@/Data
    )
endif()

# DATA FOLDER COPY (non-Emscripten only)
if(NOT EMSCRIPTEN)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${TARGET_NAME}>/Data
    )
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Data
            $<TARGET_FILE_DIR:${TARGET_NAME}>/Data
    )
endif()