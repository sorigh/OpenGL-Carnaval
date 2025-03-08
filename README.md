# 3D Carnival Scene Project

This project involves the creation and loading of a 3D scene in Visual Studio using OpenGL libraries, which interact directly with the graphics card. The user can explore the scene using the mouse and keyboard in various viewing modes. Each object in the scene has textures, and both directional and point light sources are present. The scene is set within a carnival-themed environment, featuring both modern and vintage elements. The night mode introduces a ghostly presence that adds a supernatural touch to the scene.

### Scene and Objects Description

The scene is built on a grassy terrain with textures and models representing grass, hills, and stones. The boundary of the scene is irregular, forming the impression of an extended landscape beyond the carnival’s fence. The scene combines modern elements like mechanical attractions (carousel, roller coaster, ferris wheel) with old motifs such as wooden caravans and a stone fountain at the center. 
As night falls, the atmosphere transforms with the appearance of a friendly ghost, and the caravans begin to float. A secret stone tunnel hints at an exploration beyond the visible scene limits.

![Screenshot 2025-01-13 182253](https://github.com/user-attachments/assets/ad17ccad-cd93-411f-8172-0c5e35fd0266)


### Functions and Algorithms

The OpenGL main shader implemented in this project includes a complex lighting model combining directional and point lighting, shadow mapping, and fog effects. The shader is configurable, allowing the activation or deactivation of it's components for different scene types. The primary features of the shader are:

- **Directional Lighting**: Calculates ambient, diffuse, and specular lighting based on a directional light source, using a uniform variable to toggle its effect.
- **Point Lighting**: Simulates point light sources with distance attenuation, applying ambient, diffuse, and specular components.
- **Shadows**: A shadow map is used to determine whether a fragment is in shadow by comparing its depth with a depth map.
- **Fog**: Fog effects are calculated based on the distance from the camera, blending the fragment color with a fog color.
- **Textures**: Diffuse and specular textures are applied to the fragments, with lighting adjustments based on these textures.

#### Key Functions

- **`initFBO()`**: Initializes a framebuffer for shadow map generation, creating a depth texture bound to the FBO.
- **`computeLightSpaceTrMatrix()`**: Calculates the light space transformation matrix for shadow projection.
- **`initUniforms()`**: Initializes uniform variables used in the shader, such as model, view, normal, and projection matrices.
- **`RenderModels()`**: Renders the objects in the scene, applying specific transformations to each object.
- **`renderScene()`**: The main entry point for rendering the scene, combining the above functions to create the final scene.

### Visualization Modes

- **WASD Controls**: Move the camera in the scene.
  
  - W - Forward
  - A - Left
  - S - Backward
  - D - Right
- **Additional Modes**:
  
  - Z - Wireframe Mode
  - X - Vertices Mode
  - C - Faces Mode (default view mode)
  - V - Depth Map Mode
  - P - Presentation Mode
  - N - Toggle Day/Night Mode
  - M - Toggle Directional Light

### Data Structures

- **Camera**: Defines a virtual camera for viewing the 3D scene, allowing movement and rotation.
- **Mesh**: Represents a 3D mesh for drawing objects, including vertex, index, and texture data.
- **Model3D**: Manages loading and rendering 3D models from .obj files.
- **Shader**: Handles loading, compiling, and linking shaders for rendering.
- **SkyBox**: Manages the cubemap for the skybox effect.


