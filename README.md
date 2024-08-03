## Minimal Mine Vulkan 3D renderer

The goal of this project is to render a GLTF scene with animated humanoid character, learning Vulkan and rendering / simulation techniques in the process.

### Build
The project is built using CMake. 

#### Requirements
- CMake
- Vulkan SDK 1.1+

#### Platforms
- Windows
- MacOS

### Features
#### Done
- Vulkan abstraction, all Vulkan functionalities inside graphics/backend/VulkanImpl.cpp
- graphics pipeline, compute pipeline
- parse OBJ files
- parse GLTF files 
- support GLTF animation, including skinned animation
- PBR (GLTF implementation)
- normal mapping
- keyboard and mouse controlled camera
- ImGUI integration
- hair simulation shaders

#### TODO
- cloth simulation
- hair generation
- hair rendering
- skin rendering
- morph target animation

#### Nice to Have
- optimize the GLTF importer
- post-processing pipelines
- implement the interfaces in another graphics API
- ray tracing pipeline
- sky

see graphics/Graphics.h for example 
