#ifndef MYPROJECT_IMPORTER_HPP
#define MYPROJECT_IMPORTER_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

enum class TexUsage : uint32_t{
  ALBEDO,
  NORMAL,
  METALLIC,
  ROUGHNESS,
  AO,
  EMISSIVE,
};

struct TextureDesc{
  std::string path;
  int embeddedIndex = -1;
  TexUsage flag;
};

struct Vertex{
  glm::vec3 position = glm::vec3(0); // location = 0
  glm::vec3 normal; // location = 1
  glm::vec2 uv; // location = 2
  glm::vec3 tangent; // location = 3
  glm::vec3 bitangent; // location = 4
  glm::vec4 color; // location = 5
};

enum DescriptorBindFlags : uint32_t{
  // 플래그(머티리얼 텍스처 유무/모드)
  ImageBaseColor = 0x00000001,
  ImageNormalMap = 0x00000002,
  ImageMetallicMap = 0x00000004,
  ImageRoughnessMap = 0x00000008,
  ImageAoMap = 0x00000010,
  ImageEmissionMap = 0x00000020,
  UseConstantBaseColor = 0x00000040,
  AlphaTest = 0x00000100,
  AlphaBlend = 0x00000200,
  DoubleSided = 0x00000400,
  UseVertexColor = 0x00000800,
};

inline DescriptorBindFlags operator|(const uint32_t &flag, const DescriptorBindFlags &desc)
{
  return static_cast<DescriptorBindFlags>(flag | static_cast<uint32_t>(desc));
}

inline DescriptorBindFlags operator|(const DescriptorBindFlags &l, const DescriptorBindFlags &r)
{
  return static_cast<DescriptorBindFlags>(static_cast<uint32_t>(l) | static_cast<uint32_t>(r));
}

inline DescriptorBindFlags operator&(const uint32_t &flag, const DescriptorBindFlags &desc)
{
  return static_cast<DescriptorBindFlags>(flag & static_cast<uint32_t>(desc));
}

struct alignas(16) MaterialDesc{
// x=metallic, y=roughness, z=ao, w=emissionScale
  glm::vec4 baseColor = glm::vec4(1.0f);
  glm::vec4 params = glm::vec4(0.0f);
  uint32_t flags = 0;
  int32_t texAlbedo = -1;
  int32_t texNormal = -1;
  int32_t texMetallic = -1;
  int32_t texRoughness = -1;
  int32_t texAo = -1;
  int32_t texEmissive = -1;
  int32_t padding = 0;
};

enum class MaterialMap : uint32_t{
  Albedo = 0,
  Normal,
  Metallic,
  Roughness,
  Ao,
  Emissive,
  Count
};

struct MeshDesc{
  std::string name;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  uint32_t primitives;
  uint32_t numVertices;
  uint32_t numIndices;
  uint32_t uvChanels;
  int materialIndex = -1;
  unsigned int mPrimitiveTypes;
};

struct ImportResult{
  std::vector<MeshDesc> meshes;
  std::vector<MaterialDesc> materials;
  std::vector<TextureDesc> textures;

  struct EmbeddedTex{
    std::string name;
    const uint8_t *data;
    size_t size;
    bool isCompressed;
  };

  std::vector<EmbeddedTex> embedded;
};

class ImporterEx{
public:
  ImporterEx() = default;
  ImportResult loadScene(const char *filepath);

private:
  void processMaterialsAndTextures(const aiScene *scene, const std::string &folder, ImportResult &out);
  void processMeshes(const aiScene *scene, ImportResult &out);
  void processMeshesWithOnlyTriangles(const aiScene *scene, ImportResult &out);
  int addTexture(ImportResult &out, TexUsage slot, const std::string &path, int embeddedIndex);

private:
  static glm::mat4 ToGlm(const aiMatrix4x4 &m);
  static glm::vec3 ToGlm(const aiVector3D &v);
  static glm::quat ToGlm(const aiQuaternion &q);
};

#endif //MYPROJECT_IMPORTER_HPP