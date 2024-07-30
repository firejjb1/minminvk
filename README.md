## Wrapper around Vulkan and 3D renderer (WIP)

The goal of this project is to render a fully animated humanoid character with decent simulation and rendering. Trying to learn Vulkan and rendering / simulation techniques in the process.

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
- parse OBJ files
- parse GLTF files 
- support GLTF animation, including skinned animation
- hair simulation shaders
- PBR
- normal mapping

#### TODO
- keyboard and mouse controlled camera
- get a nice GLTF model (skinned, hair, cloth)
- cloth simulation
- hair rendering
- skin rendering

#### Nice to Have
- optimize the GLTF importer
- post-processing pipelines
- implement the interfaces in another graphics API
- ray tracing pipeline
  
see graphics/Graphics.h for example 
