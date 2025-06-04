# Vulkan Renderer Class Test
This is not meant for public use, but if for whatever reason you want to use it just go for it. This is just meant for me to see how well I could learn vulkan and use it in my other projects thats all, the only reason its public to get opinions from other people

# Current To-do
* Optimize vertices to only include:
* * positions as x, y, z numbers
* * colors as unsigned 8 bits
* * uvs as unsigned 8 bits

* * batching (store filepath in dict and return the already created model and then just use 1 draw call for each mesh)
* * see how to render an entire scene with one draw call (if thats even possible)

* * dynamic render: this removes renderpasses, framebuffers, and pipelines can render into any image (only thing is they gotta match the image format they'll render into),
* * bindless: instead of binding individual pieces of data like textures/materials, bind only indices, and then along that bind whole arrays of textures and arrays of buffers which hold arrays of materials; this'll save on per-draw costs and also make it super dynamic,
* * bda: if you're going to stick with vulkan, look into BDA (buffer device address), you basically can pass the gpu memory address of a buffer to the gpu and read it directly, this bypasses the need for bindless for buffers (although apis like DX12 don't get such things),
* * instancing: you can pass an instance buffer as a vertex buffer so instead of drawing 1 thing per draw call, u can tell it to draw 1000 things at once,
* * indirect drawing: similar to instancing, u can pass an array of draw cmds (still as a buffer lik instancing) of all the things u want to draw with the current pipeline... these draw cmds can also specify the instances (from start to end) to draw... this makes it super dynamic because instead of having to remove an instance in the middle of the array and update some information, u can just update this... it basically helps doing some CPU calculations and makes things a little more dynamic

* add textures
* add imgui
* make a descriptor pool manager, ref: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
* add specialization constants to point light shaders
* in the actual game engine, add a "world" or "game" system that allows you to change the lighting properties and everything else