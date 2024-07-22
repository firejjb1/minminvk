cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(MinMinVK)

set (CMAKE_CSS_FLAGS "-std=c++20")
set(CMAKE_CXX_STANDARD 20)

include_directories(external/glm)

set(GLFW_BUILD_DOCS OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(GLFW_INSTALL OFF CACHE BOOL OFF FORCE)
set(GLFW_LIBRARY_TYPE OFF CACHE BOOL "STATIC" FORCE)
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

include_directories(external/tinyobjloader)

# STB (header only library): Load images
include_directories(external/stbImage)

include_directories(external/tinygltf)

if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

IF(UNIX AND NOT APPLE)
	set(LINUX TRUE)
ENDIF()

IF(WIN32)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(Threads REQUIRED)
	IF(USE_D2D_WSI)
		MESSAGE("Using direct to display extension...")
		add_definitions(-D_DIRECT2DISPLAY)
	ELSEIF(USE_DIRECTFB_WSI)
		find_package(DirectFB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
		include_directories(${DIRECTFB_INCLUDE_DIR})
	ELSEIF(USE_WAYLAND_WSI)
		find_program(PKG_CONFIG pkg-config)
		if (NOT PKG_CONFIG)
			message(FATAL_ERROR "pkg-config binary not found")
		endif ()
		find_package(Wayland REQUIRED)
		if (NOT WAYLAND_FOUND)
			message(FATAL_ERROR "Wayland development package not found")
		endif ()
		pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
		if (NOT WAYLAND_PROTOCOLS_FOUND)
			message(FATAL_ERROR "Wayland protocols package not found")
		endif ()
		find_program(WAYLAND_SCANNER wayland-scanner)
		if (NOT WAYLAND_SCANNER)
			message(FATAL_ERROR "wayland-scanner binary not found")
		endif ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
		include_directories(${WAYLAND_INCLUDE_DIR})
		pkg_get_variable(protocol_dir wayland-protocols pkgdatadir)
		execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
				COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
		include_directories(${CMAKE_BINARY_DIR})
	ELSEIF(USE_HEADLESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_HEADLESS_EXT")
	ELSE(USE_D2D_WSI)
		find_package(XCB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
	ENDIF(USE_D2D_WSI)
ELSEIF(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
	# Todo : android?
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_INCLUDE_DIR})
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

include_directories(${Vulkan_INCLUDE_DIR})

include_directories(external/imgui external/imgui/backends)
add_library(imgui
        external/imgui/imgui.cpp
        external/imgui/imgui.h
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_internal.h
        external/imgui/imstb_rectpack.h
        external/imgui/imstb_textedit.h
        external/imgui/imstb_truetype.h
		# Here implementation
        external/imgui/backends/imgui_impl_vulkan.cpp
        external/imgui/backends/imgui_impl_vulkan.h
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_glfw.h

	    external/imgui/imconfig.h)

set(LIBS glfw imgui)

set(SOURCE_FILES 
	"Main.cpp"
	"graphics/backend/VulkanImpl.cpp"
	"util/IO.cpp"
	"graphics/Import.cpp"
	"graphics/Node.cpp"
)

set(HEADER_FILES 
	"Input.h"
	"UI.h" 
	"util/Type.h"
	"util/IO.h"
	"util/Math.h"
	"graphics/Buffer.h"
	"graphics/Device.h"
	"graphics/Graphics.h"
	"graphics/Pipeline.h"
	"graphics/Presentation.h"
	"graphics/Shader.h"
	"graphics/Texture.h"
	"graphics/Geometry.h"
	"graphics/Import.h"
	"graphics/Resource.h"
	"graphics/UIRender.h"
	"graphics/Node.h"
	"graphics/Animation.h"
)

IF(WIN32)
	# Nothing here (yet)
ELSEIF(APPLE)
	link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(WIN32)
	link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
 	.
 )

IF(APPLE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC APPLE)
ENDIF(APPLE)

target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders/")
target_compile_definitions(${PROJECT_NAME} PUBLIC IMAGES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/images/")
target_compile_definitions(${PROJECT_NAME} PUBLIC OBJ_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/obj/")
target_compile_definitions(${PROJECT_NAME} PUBLIC HAIR_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/hair/")
target_compile_definitions(${PROJECT_NAME} PUBLIC GLTF_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/gltf/")

# disable validation layers, etc
#target_compile_definitions(${PROJECT_NAME} PUBLIC NODEBUG)

# define vulkan implementation. only supports vulkan for now
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_IMPL)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY} ${LIBS})

if(WIN32)
	add_custom_target(run_bat_file
					  COMMAND "compile.bat"
					  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders/")

	add_dependencies(${PROJECT_NAME} run_bat_file)
endif()

if (APPLE)
	add_custom_target(run_bat_file
	COMMAND "./compile.bat"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders/")

	add_dependencies(${PROJECT_NAME} run_bat_file)
endif()