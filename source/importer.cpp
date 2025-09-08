#include "importer.hpp"
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/postprocess.h>
#include <filesystem>
#include <spdlog/spdlog.h>

using std::string;
namespace fs = std::filesystem;

ImportResult ImporterEx::loadScene(const char *filepath)
{
  ImportResult out;
  Assimp::Importer importer;
  // aiProcess_ValidateDataStructure // 디버그용
  const aiScene *scene = importer.ReadFile(
  filepath,
  aiProcess_Triangulate |
  aiProcess_JoinIdenticalVertices |
  aiProcess_GenNormals |
  aiProcess_CalcTangentSpace |
  aiProcess_LimitBoneWeights |
  aiProcess_ImproveCacheLocality |
  aiProcess_GenUVCoords |
  aiProcess_SortByPType
  );

  if (!scene || !scene->HasMeshes())
  {
    throw std::runtime_error("Assimp load failed");
  }
  const string folder = fs::path(filepath).parent_path().string();
  if (scene->HasTextures())
  {
    out.embedded.reserve(scene->mNumTextures);
    //embedded texture
    for (unsigned i = 0; i < scene->mNumTextures; ++i)
    {
      const aiTexture *t = scene->mTextures[i];
      ImportResult::EmbeddedTex e{};
      e.name = t->mFilename.C_Str();
      if (t->mHeight == 0)
      {
        e.data = reinterpret_cast<const uint8_t *>(t->pcData);
        e.size = static_cast<size_t>(t->mWidth);
        e.isCompressed = true;
      } else
      {
        // 비압축 BGRA32 배열
        e.data = reinterpret_cast<const uint8_t *>(t->pcData);
        e.size = static_cast<size_t>(t->mWidth * t->mHeight * 4);
        e.isCompressed = false;
      }
      out.embedded.push_back(e);
    }
  }
  processMaterialsAndTextures(scene, folder, out);
  processMeshesWithOnlyTriangles(scene, out);
  spdlog::info("Import done: meshes={}, materials={}, textures={}",
               out.meshes.size(),
               out.materials.size(),
               out.textures.size());
  return out;
}

void ImporterEx::processMaterialsAndTextures(const aiScene *scene, const string &folder, ImportResult &out)
{
  //embedded texture
  out.materials.resize(scene->mNumMaterials);
  auto fetchTexture = [&](aiMaterial *mtl, aiTextureType type, TexUsage slot) -> int
  {
    if (mtl->GetTextureCount(type) == 0) return -1;
    aiString path;
    if (AI_SUCCESS != mtl->GetTexture(type, 0, &path)) return -1;
    std::string str = path.C_Str();

    size_t pos = str.find_last_of("/\\");
    std::string filename = (pos == std::string::npos) ? str : str.substr(pos + 1);
    std::string txtfolder = "/texture/";
    filename = txtfolder + filename;
    string full = folder + filename;
    if (str[0] == '*')
    {
      //embedded
      std::string emb = "embedded:";
      unsigned int index = std::stoi(path.C_Str());
      str = emb + str;
      return addTexture(out, slot, str, index);
    }
    return addTexture(out, slot, full, -1);
  };

  for (unsigned i = 0; i < scene->mNumMaterials; ++i)
  {
    aiMaterial *mtl = scene->mMaterials[i];
    MaterialDesc md{};
    // BaseColor (PBR) 또는 Diffuse
    aiColor4D base{};
    if (AI_SUCCESS == mtl->Get(AI_MATKEY_BASE_COLOR, base) ||
        AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_DIFFUSE, base))
    {
      md.baseColor = glm::vec4(base.r, base.g, base.b, base.a);
    }

    // Metallic/Roughness/AO/Emissive scale
    float metallic = 0.f, roughness = 1.f, ao = 1.f, emissive = 0.f;
    mtl->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    mtl->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
    aiColor3D emiColor(0, 0, 0);
    if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_EMISSIVE, emiColor))
    {
      emissive = std::max({emiColor.r, emiColor.g, emiColor.b});
    }
    md.params = glm::vec4(metallic, roughness, ao, emissive);

    // 텍스처(우선순위: PBR → 레거시)
    md.texAlbedo = fetchTexture(mtl, aiTextureType_BASE_COLOR, TexUsage::ALBEDO);
    md.texNormal = fetchTexture(mtl, aiTextureType_NORMALS, TexUsage::NORMAL);
    if (md.texAlbedo < 0) md.texAlbedo = fetchTexture(mtl, aiTextureType_DIFFUSE, TexUsage::ALBEDO);
    if (md.texNormal < 0) md.texNormal = fetchTexture(mtl, aiTextureType_HEIGHT, TexUsage::NORMAL);
    // 일부 에셋이 노말을 height에 넣음

    md.texMetallic = fetchTexture(mtl, aiTextureType_METALNESS, TexUsage::METALLIC);
    md.texRoughness = fetchTexture(mtl, aiTextureType_DIFFUSE_ROUGHNESS, TexUsage::ROUGHNESS);
    md.texAo = fetchTexture(mtl, aiTextureType_AMBIENT_OCCLUSION, TexUsage::AO);
    md.texEmissive = fetchTexture(mtl, aiTextureType_EMISSIVE, TexUsage::EMISSIVE);

    // 플래그 구성
    if (md.texAlbedo >= 0) md.flags |= DescriptorBindFlags::ImageBaseColor;
    else md.flags |= DescriptorBindFlags::UseConstantBaseColor;
    if (md.texNormal >= 0) md.flags |= DescriptorBindFlags::ImageNormalMap;
    if (md.texMetallic >= 0) md.flags |= DescriptorBindFlags::ImageMetallicMap;
    if (md.texRoughness >= 0) md.flags |= DescriptorBindFlags::ImageRoughnessMap;
    if (md.texAo >= 0) md.flags |= DescriptorBindFlags::ImageAoMap;
    if (md.texEmissive >= 0) md.flags |= DescriptorBindFlags::ImageEmissionMap;

    // 더블사이드/블렌드 등(가능하면)
    int twoSided = 0;
    if (AI_SUCCESS == mtl->Get(AI_MATKEY_TWOSIDED, twoSided) && twoSided)
    {
      md.flags |= DescriptorBindFlags::DoubleSided;
    }

    out.materials[i] = md;
  }
}

int ImporterEx::addTexture(ImportResult &out, TexUsage slot, const string &path, int embeddedIndex)
{
  TextureDesc t{};
  t.flag = slot;
  t.path = path;
  t.embeddedIndex = embeddedIndex;
  out.textures.push_back(t);
  return static_cast<int>(out.textures.size() - 1);
}

void ImporterEx::processMeshes(const aiScene *scene, ImportResult &out)
{
  out.meshes.reserve(scene->mNumMeshes);
  for (unsigned m = 0; m < scene->mNumMeshes; ++m)
  {
    aiMesh *am = scene->mMeshes[m];
    MeshDesc md{};
    md.name = am->mName.C_Str();
    md.materialIndex = static_cast<int>(am->mMaterialIndex);

    md.vertices.resize(am->mNumVertices);
    for (unsigned i = 0; i < am->mNumVertices; ++i)
    {
      Vertex v{};
      // pos
      v.position[0] = am->mVertices[i].x;
      v.position[1] = am->mVertices[i].y;
      v.position[2] = am->mVertices[i].z;
      // nrm
      if (am->HasNormals())
      {
        v.normal[0] = am->mNormals[i].x;
        v.normal[1] = am->mNormals[i].y;
        v.normal[2] = am->mNormals[i].z;
      }
      // uv0
      if (am->HasTextureCoords(0))
      {
        v.uv[0] = am->mTextureCoords[0][i].x;
        v.uv[1] = am->mTextureCoords[0][i].y;
      } else
      {
        v.uv[0] = v.uv[1] = 0.f;
      }
      // tangent/bitangent
      if (am->HasTangentsAndBitangents())
      {
        v.tangent[0] = am->mTangents[i].x;
        v.tangent[1] = am->mTangents[i].y;
        v.tangent[2] = am->mTangents[i].z;
        v.bitangent[0] = am->mBitangents[i].x;
        v.bitangent[1] = am->mBitangents[i].y;
        v.bitangent[2] = am->mBitangents[i].z;
      }
      // color0
      if (am->HasVertexColors(0))
      {
        v.color[0] = am->mColors[0][i].r;
        v.color[1] = am->mColors[0][i].g;
        v.color[2] = am->mColors[0][i].b;
        v.color[3] = am->mColors[0][i].a;
      } else
      {
        v.color[0] = v.color[1] = v.color[2] = 1.f;
        v.color[3] = 1.f;
      }

      md.vertices[i] = v;
    }
    // indices
    md.indices.reserve(am->mNumFaces * 3);
    for (unsigned f = 0; f < am->mNumFaces; ++f)
    {
      const aiFace &face = am->mFaces[f];
      for (unsigned idx = 0; idx < face.mNumIndices; ++idx)
      {
        md.indices.push_back(face.mIndices[idx]);
      }
    }
    out.meshes.push_back(std::move(md));
  }
}

///
void ImporterEx::processMeshesWithOnlyTriangles(const aiScene *scene, ImportResult &out)
{
  out.meshes.reserve(scene->mNumMeshes);
  for (unsigned m = 0; m < scene->mNumMeshes; ++m)
  {
    /// todo:
    ///  rendering mode setting with line
    ///  now: not triangle-> not draw
    ///  need to set up with dynamic topology
    ///  need: topology
    ///  -> need type binding pipeline and draw call
    aiMesh *am = scene->mMeshes[m];
    MeshDesc md{};
    md.name = am->mName.C_Str();

    md.materialIndex = static_cast<int>(am->mMaterialIndex);
    md.primitives = am->mPrimitiveTypes;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (unsigned f = 0; f < am->mNumFaces; ++f)
    {
      const aiFace &face = am->mFaces[f];
      if (face.mNumIndices != 3)
      {
        spdlog::info("passed, need to pipeline update for muliy geometry");
        continue;
      }
      uint32_t startIndex = static_cast<uint32_t>(vertices.size());
      //ALWAYS INDEX : 3 CASE
      for (unsigned idx = 0; idx < 3; ++idx)
      {
        const auto &srcV = am->mVertices[face.mIndices[idx]];
        Vertex v{};
        v.position[0] = srcV.x;
        v.position[1] = srcV.y;
        v.position[2] = srcV.z;
        if (am->HasNormals())
        {
          v.normal[0] = am->mNormals[face.mIndices[idx]].x;
          v.normal[1] = am->mNormals[face.mIndices[idx]].y;
          v.normal[2] = am->mNormals[face.mIndices[idx]].z;
        }

        if (am->HasTextureCoords(0))
        {
          auto &uv = am->mTextureCoords[0][face.mIndices[idx]];
          v.uv[0] = uv.x;
          v.uv[1] = uv.y;
        } else
        {
          spdlog::info("no uv need to setting");
          v.uv[0] = v.uv[1] = 0.f;
        }
        if (am->HasTangentsAndBitangents())
        {
          v.tangent[0] = am->mTangents[face.mIndices[idx]].x;
          v.tangent[1] = am->mTangents[face.mIndices[idx]].y;
          v.tangent[2] = am->mTangents[face.mIndices[idx]].z;
          v.bitangent[0] = am->mBitangents[face.mIndices[idx]].x;
          v.bitangent[1] = am->mBitangents[face.mIndices[idx]].y;
          v.bitangent[2] = am->mBitangents[face.mIndices[idx]].z;
        }
        if (am->HasVertexColors(0))
        {
          v.color[0] = am->mColors[0][face.mIndices[idx]].r;
          v.color[1] = am->mColors[0][face.mIndices[idx]].g;
          v.color[2] = am->mColors[0][face.mIndices[idx]].b;
          v.color[3] = am->mColors[0][face.mIndices[idx]].a;
        } else
        {
          v.color[0] = v.color[1] = v.color[2] = 1.f;
          v.color[3] = 1.f;
        }
        vertices.push_back(v);
      }
      indices.push_back(startIndex);
      indices.push_back(startIndex + 1);
      indices.push_back(startIndex + 2);
    }
    md.vertices = vertices;
    md.indices = indices;
    out.meshes.push_back(std::move(md));
  }
}

// 유틸 변환
glm::mat4 ImporterEx::ToGlm(const aiMatrix4x4 &m)
{
  return glm::mat4(
  m.a1,
  m.b1,
  m.c1,
  m.d1,
  m.a2,
  m.b2,
  m.c2,
  m.d2,
  m.a3,
  m.b3,
  m.c3,
  m.d3,
  m.a4,
  m.b4,
  m.c4,
  m.d4
  );
}

glm::vec3 ImporterEx::ToGlm(const aiVector3D &v)
{
  return glm::vec3(v.x, v.y, v.z);
}

glm::quat ImporterEx::ToGlm(const aiQuaternion &q)
{
  return glm::quat(q.w, q.x, q.y, q.z);
}