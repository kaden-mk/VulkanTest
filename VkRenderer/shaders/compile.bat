@echo off
cd /d "%~dp0"

E:\VulkanSDK\1.4.304.0\Bin\glslc.exe shader.vert -o compiled\vert.spv
E:\VulkanSDK\1.4.304.0\Bin\glslc.exe shader.frag -o compiled\frag.spv