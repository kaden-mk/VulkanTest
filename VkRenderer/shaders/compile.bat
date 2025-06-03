@echo off
cd /d "%~dp0"

E:\VulkanSDK\1.4.304.0\Bin\glslc.exe shader.vert -o compiled\vert.spv
E:\VulkanSDK\1.4.304.0\Bin\glslc.exe shader.frag -o compiled\frag.spv
E:\VulkanSDK\1.4.304.0\Bin\glslc.exe point_light.vert -o compiled\point_light_vert.spv
E:\VulkanSDK\1.4.304.0\Bin\glslc.exe point_light.frag -o compiled\point_light_frag.spv