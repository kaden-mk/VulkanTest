# Vulkan Renderer Class Test
This is not meant for public use, but if for whatever reason you want to use it just go for it. This is just meant for me to see how well I could learn vulkan and use it in my other projects thats all, the only reason its public to get opinions from other people

# Current To-do
* Optimize vertices to only include:
* positions as x, y, z numbers
* colors as unsigned 8 bits
* uvs as unsigned 8 bits

* batching (store filepath in dict and return the already created model and then just use 1 draw call for each mesh)
* see how to render an entire scene with one draw call (if thats even possible)

* reorganize the entire project
* quick note: make it so that the vulkanapp is the main class you create, assign a window to it and you add rendering objects and stuff inside that loop outside
* add textures
* make a descriptor pool manager, ref: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
* add specialization constants to point light shaders
* add imgui
* in the actual game engine, add a "world" or "game" system that allows you to change the lighting properties and everything else