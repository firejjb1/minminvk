## Minimal Vulkan 3D renderer

The goal of this project is to render a GLTF scene with animated humanoid character, learning Vulkan and rendering / simulation techniques in the process.

### Build
The project is built using CMake. 

#### Requirements
- CMake
- Vulkan SDK 1.3.280+
- C++20 compiler

#### Platforms
- Windows
- MacOS (tentative)
- Linux (tentative)

### Features
- Vulkan abstraction, all Vulkan functionalities inside graphics/backend/VulkanImpl.cpp
- graphics pipeline, compute pipeline
- choose between forward or deferred rendering with local read (TBDR). search "USE_DEFERRED" in CMakeLists.txt
- parse OBJ files
- parse GLTF files 
- support GLTF animation, including skinned animation and morph targets
- PBR (GLTF implementation)
- normal mapping
- keyboard and mouse controlled camera
- ImGUI integration
- hair simulation shaders
- dancing Lain

#### Tested [GLTF Models](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0)
- Cube
- BoxAnimated
- BoxVertexColors
- AnimatedCube
- RiggedSimple
- RiggedFigure
- CesiumMan
- CesiumMilkTruck
- Sponza
- NormalTangentMirrorTest
- TextureSettingsTest
- AlphaBlendModeTest
- SimpleMorph
- AnimatedMorphCube

see graphics/Graphics.h for example 


![Dancing Lain](https://github.com/firejjb1/minminvk/blob/master/MinminVk/assets/Animation.gif)

