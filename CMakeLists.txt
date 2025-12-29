cmake_minimum_required(VERSION 3.30)
project(ThreeBody LANGUAGES C CXX)


# Include directories for GLFW, CUDA, and GLAD
include_directories(${CMAKE_SOURCE_DIR}/libs/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)  # Ensure glad.c is available

link_directories(${CMAKE_SOURCE_DIR}/libs/mingw)

# Add GLAD to the project as a STATIC library
add_library(glad STATIC src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Create the executable
add_executable(ThreeBody main.cpp
        Implementations/ThreeBodyGL.cpp
        Implementations/ThreeBodyGL.h
        include/stb_image_write.h
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_opengl3_loader.h
        Implementations/UIWrapper.cpp
        Implementations/UIWrapper.h
)

# Link GLAD and other libraries
target_link_libraries(ThreeBody PRIVATE glad glfw3 opengl32
        kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)


add_custom_command(TARGET SlimeMold POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders
        $<TARGET_FILE_DIR:SlimeMold>/Shaders
)
target_include_directories(ThreeBody PRIVATE imgui imgui/backends)
