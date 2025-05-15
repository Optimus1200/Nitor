cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)
project(Nitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

find_package(OpenGL REQUIRED)

include(FetchContent)

# =================== GLFW ===================
set(USE_FETCHED_GLFW OFF)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "glfw3 not found, using FetchContent")
    set(USE_FETCHED_GLFW ON)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest
    )
    FetchContent_MakeAvailable(glfw)
endif()

# =================== GLM ===================
set(USE_FETCHED_GLM OFF)
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "glm not found, using FetchContent")
    set(USE_FETCHED_GLM ON)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.7
    )
    FetchContent_MakeAvailable(glm)
endif()

# =================== Assimp ===================
set(USE_FETCHED_ASSIMP OFF)
find_package(assimp QUIET)
if(NOT assimp_FOUND)
    message(STATUS "assimp not found, using FetchContent")
    set(USE_FETCHED_ASSIMP ON)
    FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.3.1
    )
    FetchContent_MakeAvailable(assimp)
endif()

# =================== EnTT ===================
set(USE_FETCHED_ENTT OFF)
find_package(EnTT QUIET)
if(NOT EnTT_FOUND)
    message(STATUS "EnTT not found, using FetchContent")
    set(USE_FETCHED_ENTT ON)
    FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(entt)
endif()

# =================== ImGui ===================
# ImGui has no find_package, always fetch
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90
)
FetchContent_MakeAvailable(imgui)

# =================== stb ===================
# stb has no find_package, always fetch
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# =================== Source Files ===================
file(GLOB_RECURSE NITOR_SRC CONFIGURE_DEPENDS src/*.cpp src/*.hpp src/*.c)

add_executable(Nitor ${NITOR_SRC})

set_target_properties(Nitor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Nitor>"
)

# =================== ImGui Sources ===================
set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_sources(Nitor PRIVATE ${IMGUI_SOURCES})

# =================== Include Directories ===================
target_include_directories(Nitor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
)

if(USE_FETCHED_GLM)
    target_include_directories(Nitor PRIVATE ${glm_SOURCE_DIR})
endif()

# =================== Link Libraries ===================
target_link_libraries(Nitor PRIVATE OpenGL::GL)

if(USE_FETCHED_GLFW)
    target_link_libraries(Nitor PRIVATE glfw)
else()
    target_link_libraries(Nitor PRIVATE glfw)
endif()

if(USE_FETCHED_ASSIMP)
    target_link_libraries(Nitor PRIVATE assimp)
else()
    target_link_libraries(Nitor PRIVATE assimp::assimp)
endif()

if(USE_FETCHED_ENTT)
    target_link_libraries(Nitor PRIVATE EnTT::EnTT) # Most likely still provides the target
else()
    target_link_libraries(Nitor PRIVATE EnTT::EnTT)
endif()

# =================== Post-Build: Copy Assets ===================
add_custom_command(TARGET Nitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/models
            $<TARGET_FILE_DIR:Nitor>/models
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:Nitor>/shaders
)
