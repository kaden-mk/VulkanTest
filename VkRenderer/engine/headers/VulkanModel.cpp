#include "VulkanModel.hpp"

#include "VulkanUtils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "openfbx/ofbx.h"

#include <cassert>
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

namespace std {
	template <>
	struct hash<VkRenderer::VulkanModel::Vertex> {
		size_t operator()(VkRenderer::VulkanModel::Vertex const& vertex) const {
			size_t seed = 0;
			VkRenderer::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace VkRenderer {
	VulkanModel::VulkanModel(VulkanDevice& device, const VulkanModel::Builder &builder)
		: device{device}
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	VulkanModel::~VulkanModel() {}

	std::unique_ptr<VulkanModel> VulkanModel::createModelFromFile(VulkanDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);

		return std::make_unique<VulkanModel>(device, builder);
	}

	void VulkanModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void VulkanModel::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer) 
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		else
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void VulkanModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be atleast 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		VulkanBuffer stagingBuffer{
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<VulkanBuffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	
		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void VulkanModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		VulkanBuffer stagingBuffer{
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<VulkanBuffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	std::vector<VkVertexInputBindingDescription> VulkanModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
		attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent) });

		return attributeDescriptions;
	}

    void VulkanModel::Builder::loadModel(const std::string& filepath) {
        vertices.clear();
        indices.clear();
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        const std::string extension = fs::path(filepath).extension().string();

        if (extension == ".obj") {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
                throw std::runtime_error(warn + err);

            for (auto& shape : shapes) {
                for (auto& idx : shape.mesh.indices) {
                    Vertex v{};
                    if (idx.vertex_index >= 0) {
                        v.position = {
                            attrib.vertices[3 * idx.vertex_index + 0],
                            attrib.vertices[3 * idx.vertex_index + 1],
                            attrib.vertices[3 * idx.vertex_index + 2]
                        };
                        size_t ci = 3 * idx.vertex_index;
                        if (attrib.colors.size() >= ci + 3) {
                            v.color = {
                                attrib.colors[ci + 0],
                                attrib.colors[ci + 1],
                                attrib.colors[ci + 2]
                            };
                        }
                        else {
                            v.color = glm::vec3(1.0f);
                        }
                    }
                    if (idx.normal_index >= 0) {
                        v.normal = {
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        };
                    }
                    if (idx.texcoord_index >= 0) {
                        v.uv = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        };
                    }

                    if (!uniqueVertices.count(v)) {
                        uniqueVertices[v] = (uint32_t)vertices.size();
                        vertices.push_back(v);
                    }
                    indices.push_back(uniqueVertices[v]);
                }
            }
        }
        else if (extension == ".gltf" || extension == ".glb") {
            tinygltf::Model model;
            tinygltf::TinyGLTF loader;
            std::string err, warn;

            bool ok = (extension == ".glb")
                ? loader.LoadBinaryFromFile(&model, &err, &warn, filepath)
                : loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
            if (!ok)
                throw std::runtime_error("Failed to load glTF: " + warn + err);

            for (auto& mesh : model.meshes) {
                for (auto& prim : mesh.primitives) {
                    std::vector<glm::vec3> positions, normals;
                    std::vector<glm::vec2> uvs;

                    auto extractVec3 = [&](const std::string& attr, auto& out) {
                        if (!prim.attributes.count(attr)) return;
                        auto& acc = model.accessors[prim.attributes.at(attr)];
                        auto& bv = model.bufferViews[acc.bufferView];
                        auto& buf = model.buffers[bv.buffer];
                        const float* data = reinterpret_cast<const float*>(
                            buf.data.data() + bv.byteOffset + acc.byteOffset
                            );
                        for (size_t i = 0; i < acc.count; ++i)
                            out.emplace_back(data[i * 3 + 0], data[i * 3 + 1], data[i * 3 + 2]);
                        };
                    auto extractVec2 = [&](const std::string& attr, auto& out) {
                        if (!prim.attributes.count(attr)) return;
                        auto& acc = model.accessors[prim.attributes.at(attr)];
                        auto& bv = model.bufferViews[acc.bufferView];
                        auto& buf = model.buffers[bv.buffer];
                        const float* data = reinterpret_cast<const float*>(
                            buf.data.data() + bv.byteOffset + acc.byteOffset
                            );
                        for (size_t i = 0; i < acc.count; ++i)
                            out.emplace_back(data[i * 2 + 0], data[i * 2 + 1]);
                        };

                    extractVec3("POSITION", positions);
                    extractVec3("NORMAL", normals);
                    extractVec2("TEXCOORD_0", uvs);

                    for (size_t i = 0; i < positions.size(); ++i) {
                        Vertex v{};
                        v.position = positions[i];
                        v.normal = (i < normals.size() ? normals[i] : glm::vec3(0.0f));
                        v.uv = (i < uvs.size() ? glm::vec2(uvs[i].x, 1.0f - uvs[i].y)
                            : glm::vec2(0.0f));
                        v.color = glm::vec3(1.0f);

                        if (!uniqueVertices.count(v)) {
                            uniqueVertices[v] = (uint32_t)vertices.size();
                            vertices.push_back(v);
                        }
                    }

                    auto& acc = model.accessors[prim.indices];
                    auto& bv = model.bufferViews[acc.bufferView];
                    auto& buf = model.buffers[bv.buffer];
                    const uint8_t* idxData = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    for (size_t i = 0; i < acc.count; ++i) {
                        uint32_t srcIdx;
                        switch (acc.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            srcIdx = reinterpret_cast<const uint8_t*>(idxData)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            srcIdx = reinterpret_cast<const uint16_t*>(idxData)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            srcIdx = reinterpret_cast<const uint32_t*>(idxData)[i]; break;
                        default:
                            throw std::runtime_error("Unsupported index type");
                        }

                        Vertex v{};
                        v.position = positions[srcIdx];
                        v.normal = (srcIdx < normals.size() ? normals[srcIdx] : glm::vec3(0.0f));
                        v.uv = (srcIdx < uvs.size()
                            ? glm::vec2(uvs[srcIdx].x, 1.0f - uvs[srcIdx].y)
                            : glm::vec2(0.0f));
                        v.color = glm::vec3(1.0f);

                        auto it = uniqueVertices.find(v);
                        if (it == uniqueVertices.end())
                            throw std::runtime_error("Vertex dedupe mismatch");
                        indices.push_back(it->second);
                    }
                }
            }
        }
        /*else if (extension == ".fbx") {
            std::ifstream file(filepath, std::ios::binary | std::ios::ate);
            if (!file)
                throw std::runtime_error("Failed to open FBX file: " + filepath);

            size_t fileSize = (size_t)file.tellg();
            file.seekg(0);
            std::vector<uint8_t> buffer(fileSize);
            file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

            ofbx::LoadFlags flags = ofbx::LoadFlags::IGNORE_ANIMATIONS | ofbx::LoadFlags::IGNORE_SKIN | ofbx::LoadFlags::IGNORE_BONES | ofbx::LoadFlags::IGNORE_MATERIALS;
            const ofbx::IScene* scene = ofbx::load(buffer.data(), static_cast<int>(fileSize), (ofbx::u16)flags);

            if (!scene)
                throw std::runtime_error(std::string("Failed to parse FBX file: ") + ofbx::getError());

            for (int meshIndex = 0; meshIndex < scene->getMeshCount(); ++meshIndex) {
                const ofbx::Mesh* mesh = scene->getMesh(meshIndex);
                if (!mesh) continue;

                const ofbx::GeometryData& geom = mesh->getGeometryData();
                const ofbx::Vec3Attributes positions = geom.getPositions();
                const ofbx::Vec3Attributes normals = geom.getNormals();
                const ofbx::Vec2Attributes uvs = geom.getUVs();

                for (int partitionIndex = 0; partitionIndex < geom.getPartitionCount(); ++partitionIndex) {
                    const ofbx::GeometryPartition& partition = geom.getPartition(partitionIndex);

                    for (int polyIndex = 0; polyIndex < partition.polygon_count; ++polyIndex) {
                        const ofbx::GeometryPartition::Polygon& poly = partition.polygons[polyIndex];

                        for (int i = 2; i < poly.vertex_count; ++i) {
                            int idx0 = poly.from_vertex;
                            int idx1 = poly.from_vertex + i - 1;
                            int idx2 = poly.from_vertex + i;

                            int face[] = { idx0, idx1, idx2 };

                            for (int j = 0; j < 3; ++j) {
                                int idx = face[j];

                                Vertex v{};
                                ofbx::Vec3 pos = positions.get(idx);
                                v.position = glm::vec3((float)pos.x, (float)pos.y, (float)pos.z);

                                if (normals.values) {
                                    ofbx::Vec3 norm = normals.get(idx);
                                    v.normal = glm::vec3((float)norm.x, (float)norm.y, (float)norm.z);
                                }
                                else 
                                    v.normal = glm::vec3(0.0f);
                                

                                if (uvs.values) {
                                    ofbx::Vec2 uv = uvs.get(idx);
                                    v.uv = glm::vec2((float)uv.x, 1.0f - (float)uv.y);
                                }
                                else 
                                    v.uv = glm::vec2(0.0f);
                                

                                v.color = glm::vec3(1.0f); 

                                if (!uniqueVertices.count(v)) {
                                    uniqueVertices[v] = static_cast<uint32_t>(vertices.size());
                                    vertices.push_back(v);
                                }

                                indices.push_back(uniqueVertices[v]);
                            }
                        }
                    }
                }
            }
        }*/
        else {
            throw std::runtime_error("Unsupported model format: " + extension);
        }

        // Tangents
        std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
        for (size_t i = 0; i < indices.size(); i += 3) {
            auto i0 = indices[i + 0], i1 = indices[i + 1], i2 = indices[i + 2];
            auto& p0 = vertices[i0].position;
            auto& p1 = vertices[i1].position;
            auto& p2 = vertices[i2].position;
            auto& uv0 = vertices[i0].uv;
            auto& uv1 = vertices[i1].uv;
            auto& uv2 = vertices[i2].uv;

            auto edge1 = p1 - p0;
            auto edge2 = p2 - p0;
            auto duv1 = uv1 - uv0;
            auto duv2 = uv2 - uv0;

            float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);
            glm::vec3 tangent = f * (duv2.y * edge1 - duv1.y * edge2);

            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            auto& N = vertices[i].normal;
            auto& T = tangents[i];
            glm::vec3 t = glm::normalize(T - N * glm::dot(N, T));
            float handed = (glm::dot(glm::cross(N, T), glm::cross(N, t)) < 0.0f) ? -1.0f : 1.0f;
            vertices[i].tangent = glm::vec4(t, handed);
        }
    }
}