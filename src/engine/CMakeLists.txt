set(NAME "Core")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(${NAME} STATIC
  Core/Log.cpp Core/Log.hpp Core/Debug/Instrumentor.hpp
  Core/Application.cpp Core/Application.hpp Core/Window.cpp Core/Window.hpp
  Core/Resources.hpp Core/DPIHandler.hpp
        Core/Engine.cpp
        Core/Engine.h
        Core/Renderer.cpp
        Core/Renderer.h
        Core/SceneManager.cpp
        Core/SceneManager.h
        Core/EngineUtils.h
        Core/ECS.cpp
        Core/ECS.h
        Core/EventBus.cpp
        Core/EventBus.h
        Core/CameraManager.cpp
        Core/CameraManager.h
        Core/ContactListener.cpp
        Core/ContactListener.h
        Core/Contact.h
        Core/InputManager.cpp
        Core/InputManager.h
        Core/Raycaster.cpp
        Core/Raycaster.h
        Core/RaycastResult.h
        Core/Rigidbody.cpp
        Core/Rigidbody.h
        Core/Actor.cpp
        Core/Actor.h
        Core/ActorTemplate.cpp
        Core/ActorTemplate.h
        Core/AudioManager.cpp
        Core/AudioManager.h
        Core/AudioHelper.h
        Core/Helper.h
        )

# Define set of OS specific files to include
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_sources(${NAME} PRIVATE
    Platform/Windows/Resources.cpp Platform/Windows/DPIHandler.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_sources(${NAME} PRIVATE
    Platform/Mac/Resources.cpp Platform/Mac/DPIHandler.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(${NAME} PRIVATE
    Platform/Linux/Resources.cpp Platform/Linux/DPIHandler.cpp)
endif ()

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${NAME} PUBLIC ${CMAKE_SOURCE_DIR}/vendor)
target_compile_features(${NAME} PRIVATE cxx_std_17)

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/sdl2)

# find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

target_link_libraries(${NAME}
  PRIVATE project_warnings
  PUBLIC fmt spdlog SDL2::SDL2 imgui Settings box2d
  PUBLIC ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES}
  ${LUA_LIBRARIES}
)

add_subdirectory(Tests)
