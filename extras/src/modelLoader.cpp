
#include <LabRender/LabRender.h>
#include <LabRender/Model.h>
#include <LabRender/utils.h>

#define BUILDING_LABRENDER_MODELLOADER
#include "LabRenderModelLoader/modelLoader.h"


#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#ifdef HAVE_ASSIMP

#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#endif

#include <sstream>


namespace lab {
    using namespace std;
    using namespace Render;

    namespace {


        class ogzstream;
        class igzstream;


        namespace
        {
            class WeightPacker
            {
            public:
                WeightPacker()
                {
                }
                ~WeightPacker() {}

                void addWeight(float boneIndex, float boneWeight)
                {
                    if (weights.size() == 0)
                    {
                        indices.push_back(boneIndex);
                        weights.push_back(boneWeight);
                        return;
                    }
                    else
                    {
                        for (int i = 0; i<weights.size(); i++)
                        {
                            if (boneWeight>weights[i])
                            {
                                //insert at position i, shift everything else down
                                weights.insert(weights.begin() + i + 1, boneWeight);
                                indices.insert(indices.begin() + i + 1, boneIndex);
                                if (weights.size()>4)
                                {
                                    weights.resize(4);
                                    indices.resize(4);
                                }
                                return;
                            }
                        }
                        indices.push_back(boneIndex);
                        weights.push_back(boneWeight);
                    }
                }

                void getIndexAndWeights(v4f &index, v4f &weight)
                {
                    weights.resize(4, 0.0);
                    indices.resize(4, 0.0);
                    for (int i = 0; i<4; i++)
                    {
                        weight[i] = weights[i];
                        index[i] = indices[i];
                    }
                }

                std::vector<float> indices;
                std::vector<float> weights;
            };
        }



        namespace MeshFu
        {
            class PointCloud {
            public:
                virtual ~PointCloud() {}

                virtual void         write(ogzstream& stream) {}
                virtual void         read(igzstream& stream) {}

                string               mName;
                vector<v3f>    mVertices;
                vector<v3f>    mNormals;
            };

            class MeshTri;
            typedef std::shared_ptr<MeshTri> MeshTriRef;

            class MeshTri : public PointCloud {
            public:
                MeshTri() :
                    PointCloud(),
                    mHasTexture(false)
                {}

                virtual ~MeshTri() {}

                struct Face
                {
                public:
                    Face() {}
                    Face(int a, int b, int c)
                    {
                        arr[0] = a; arr[1] = b; arr[2] = c;
                    }

                    const int &operator[](int i) const
                    {
                        return arr[i];
                    }

                private:
                    int arr[3];
                };

                bool                     mHasTexture;
                vector<v2f>        mTextureCoords;
                vector<v4f>        mColors;


                vector<Face>             mFaces;
                vector<shared_ptr<PointCloud>>    mMorphs;
            };

            class Bone
            {
            public:
                ~Bone() {}

                struct VertexWeight
                {
                    int   idx;
                    float value;
                };

                string                mName;

                //! Matrix that transforms from mesh space to bone space in bind pose
                m44f           mOffsetMatrix;

                virtual void          write(ogzstream& stream) {}
                virtual void          read(igzstream& stream) {}
            };

            class Geometry : public MeshTri
            {
            public:
                Geometry() : MeshTri() {
                }

                virtual ~Geometry() {
                }

                void                  build(std::string modelDir) {}

                enum class ActiveFace { Front, Back, FrontBack };
                enum class TexWrapMode { Clamp, Repeat };

                struct textureStruct
                {
                    string relativeTexPath;
                };

                struct materialStruct
                {
                    ActiveFace face = { ActiveFace::Front };
                    v4f dcolor;
                    v4f scolor;
                    v4f acolor;
                    v4f ecolor;
                };

                struct materialFormatStruct
                {
                    TexWrapMode wrapS;
                    TexWrapMode wrapT;
                };

                string           mName;
                textureStruct         mTextureData;
                materialStruct        mMaterialData;
                materialFormatStruct  mMaterialFormatData;
                bool                  mTwoSided;


                vector<float>  mMorphWeights;
                bool                  mValidCache;
                vector<shared_ptr<Bone>>  mBones;

                vector<v4f>        mBoneIndices;
                vector<v4f>        mBoneWeights;
            };
        } // MeshFu

        string parentPath(const string& path) {
            return path.substr(0, path.rfind('/'));
        }
        string filename(const string& path) {
            return path.substr(path.rfind('/') + 1, path.length());
        }




#ifdef HAVE_ASSIMP
        std::unique_ptr<ModelPart> convertMesh(const aiMesh *aim)
        {
            const int hasNormalsAttr = 1;
            const int hasTexcoordsAttr = 2;
            const int hasColorsAttr = 4;

            enum VertType {
                VertTypePoint,
                VertTypePN,
                VertTypePT,
                VertTypePTN,
                VertTypePC,
                VertTypePTC,
                VertTypePTNC
            };

            VertType vertTypes[8] = { VertTypePoint,
                VertTypePN,
                VertTypePT,
                VertTypePTN,
                VertTypePC,
                VertTypePTC,
                VertTypePTNC
            };

            int verttypei = (aim->HasNormals() ? hasNormalsAttr : 0) +
                (aim->GetNumUVChannels()>0 ? hasTexcoordsAttr : 0) +
                (aim->GetNumColorChannels()>0 ? hasColorsAttr : 0);

            VertType vt = vertTypes[verttypei];


            ModelPart* mesh = 0;

            Bounds bounds;
            bounds.first = { FLT_MAX, FLT_MAX, FLT_MAX };
            bounds.second = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            switch (vt) {
            case VertTypePoint: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertP>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    vertdata->vertexData<VertP>(true)->push_back(VertP(v));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                                break;
            case VertTypePN: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPN>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiVector3D &n = aim->mNormals[i];
                    vertdata->vertexData<VertPN>(true)->push_back(VertPN(v,
                        V3F(n.x, n.y, n.z)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                             break;
            case VertTypePT: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPT>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiVector3D &t = aim->mTextureCoords[0][i];
                    vertdata->vertexData<VertPT>(true)->push_back(VertPT(v,
                        V2F(t.x, t.y)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                             break;
            case VertTypePTN: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiVector3D &t = aim->mTextureCoords[0][i];
                    aiVector3D &n = aim->mNormals[i];
                    vertdata->vertexData<VertPTN>(true)->push_back(VertPTN(v,
                        V2F(t.x, t.y),
                        V3F(n.x, n.y, n.z)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                              break;
            case VertTypePC: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPC>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiColor4D &c = aim->mColors[0][i];
                    vertdata->vertexData<VertPC>(true)->push_back(VertPC(v,
                        V4F(c.r, c.g, c.b, c.a)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                             break;
            case VertTypePTC: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPTC>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiVector3D &t = aim->mTextureCoords[0][i];
                    aiColor4D &c = aim->mColors[0][i];
                    vertdata->vertexData<VertPTC>(true)->push_back(VertPTC(v,
                        V2F(t.x, t.y),
                        V4F(c.r, c.g, c.b, c.a)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                              break;
            case VertTypePTNC: {
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPTNC>>(BufferBase::BufferType::VertexBuffer));
                for (size_t i = 0; i < aim->mNumVertices; ++i) {
                    aiVector3D &vert = aim->mVertices[i];
                    v3f v = { vert.x, vert.y, vert.z };
                    bounds = extendBounds(bounds, v);
                    aiVector3D &t = aim->mTextureCoords[0][i];
                    aiVector3D &n = aim->mNormals[i];
                    aiColor4D &c = aim->mColors[0][i];
                    vertdata->vertexData<VertPTNC>(true)->push_back(VertPTNC(v,
                        V2F(t.x, t.y),
                        V3F(n.x, n.y, n.z),
                        V4F(c.r, c.g, c.b, c.a)));
                }
                mesh = new ModelPart();
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds); }
                               break;
            };

            VAO* verts = mesh->verts();
            std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();
            verts->setIndices(indices);

            for (unsigned i = 0; i < aim->mNumFaces; ++i)
            {
                //     if ( aim->mFaces[i].mNumIndices > 3 )
                //     {
                //       throw MeshFuExc( "non-triangular face found: model " +
                //           std::string( aim->mName.data ) + ", face #" +
                //           toString< unsigned >( i ) );
                //     }

                indices->push_back(aim->mFaces[i].mIndices[0]);
                indices->push_back(aim->mFaces[i].mIndices[1]);
                indices->push_back(aim->mFaces[i].mIndices[2]);
            }

            return unique_ptr<ModelPart>(mesh);
        }



        unique_ptr<ModelPart> convertAiMesh(const aiScene *scene,
            const aiMesh *mesh,
            std::string nameToUse,
            string baseDir) {
            shared_ptr<MeshFu::Geometry> meshFuRef = shared_ptr<MeshFu::Geometry>(new MeshFu::Geometry());

            meshFuRef->mName = nameToUse;//fromAssimp( mesh->mName );

                                         // Handle material info
            aiMaterial *mtl = scene->mMaterials[mesh->mMaterialIndex];

            aiString name;
            mtl->Get(AI_MATKEY_NAME, name);

            // Culling
            int twoSided;
            if ((AI_SUCCESS == mtl->Get(AI_MATKEY_TWOSIDED, twoSided)) && twoSided) {
                meshFuRef->mTwoSided = true;
                meshFuRef->mMaterialData.face = MeshFu::Geometry::ActiveFace::FrontBack;
            }
            else {
                meshFuRef->mTwoSided = false;
                meshFuRef->mMaterialData.face = MeshFu::Geometry::ActiveFace::Front;
            }

            // shouldn't ecolor default to black?
            aiColor4D dcolor(1, 1, 1, 1), scolor(1, 1, 1, 1), acolor(1, 1, 1, 1), ecolor(1, 1, 1, 1);
            if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_DIFFUSE, dcolor)) {
                //meshFuRef->mMaterialData.hasDcolor = true;
            }
            //else
            //     meshFuRef->mMaterialData.hasDcolor = false;
            meshFuRef->mMaterialData.dcolor = v4f(dcolor.r, dcolor.g, dcolor.b, dcolor.a);

            if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_SPECULAR, scolor)) {
                //     meshFuRef->mMaterialData.hasScolor = true;
            }
            //   else
            //     meshFuRef->mMaterialData.hasScolor = false;
            meshFuRef->mMaterialData.scolor = v4f(scolor.r, scolor.g, scolor.b, scolor.a);

            if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_AMBIENT, acolor)) {
                //     meshFuRef->mMaterialData.hasAcolor = true;
            }
            //   else
            //     meshFuRef->mMaterialData.hasAcolor = false;
            meshFuRef->mMaterialData.acolor = v4f(acolor.r, acolor.g, acolor.b, acolor.a);

            if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_EMISSIVE, ecolor)) {
                //     meshFuRef->mMaterialData.hasEcolor = false;
            }
            //   else
            //     meshFuRef->mMaterialData.hasEcolor = false;
            meshFuRef->mMaterialData.ecolor = v4f(ecolor.r, ecolor.g, ecolor.b, ecolor.a);

            // Load Textures
            int texIndex = 0;
            aiString texPath;

            // TODO: handle other aiTextureTypes
            if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {
                string texFsPath(texPath.data);
                string relTexPath = parentPath(texFsPath);
                string texFile = filename(texFsPath);
                string realPath = baseDir + "/" + relTexPath + "/" + texFile;
                string relTexLoc = relTexPath + "/" + texFile;
                //std::cout<<relTexLoc<<std::endl;
                meshFuRef->mTextureData.relativeTexPath = relTexLoc;

                // texture wrap
                meshFuRef->mMaterialFormatData.wrapS = MeshFu::Geometry::TexWrapMode::Repeat;
                int uwrap;
                if (AI_SUCCESS == mtl->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), uwrap))
                {
                    switch (uwrap)
                    {
                    case aiTextureMapMode_Wrap:
                        meshFuRef->mMaterialFormatData.wrapS = MeshFu::Geometry::TexWrapMode::Repeat;
                        break;

                    case aiTextureMapMode_Clamp:
                        //format.setWrapS( GL_CLAMP );
                        meshFuRef->mMaterialFormatData.wrapS = MeshFu::Geometry::TexWrapMode::Clamp;
                        //iOS compatibility
                        break;

                    case aiTextureMapMode_Decal:
                        // If the texture coordinates for a pixel are outside [0...1]
                        // the texture is not applied to that pixel.
                        meshFuRef->mMaterialFormatData.wrapS = MeshFu::Geometry::TexWrapMode::Clamp;
                        break;

                    case aiTextureMapMode_Mirror:
                        // A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2
                        // is zero and 1-(u%1)|1-(v%1) otherwise.
                        // TODO
                        meshFuRef->mMaterialFormatData.wrapS = MeshFu::Geometry::TexWrapMode::Repeat;
                        break;
                    }
                }

                int vwrap;
                meshFuRef->mMaterialFormatData.wrapT = MeshFu::Geometry::TexWrapMode::Repeat;
                if (AI_SUCCESS == mtl->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), vwrap))
                {
                    switch (vwrap)
                    {
                    case aiTextureMapMode_Wrap:
                        meshFuRef->mMaterialFormatData.wrapT = MeshFu::Geometry::TexWrapMode::Repeat;
                        break;

                    case aiTextureMapMode_Clamp:
                        //format.setWrapT( GL_CLAMP );
                        //iOS compatibility
                        meshFuRef->mMaterialFormatData.wrapT = MeshFu::Geometry::TexWrapMode::Clamp;
                        break;

                    case aiTextureMapMode_Decal:
                        // If the texture coordinates for a pixel are outside [0...1]
                        // the texture is not applied to that pixel.
                        meshFuRef->mMaterialFormatData.wrapT = MeshFu::Geometry::TexWrapMode::Clamp;
                        break;

                    case aiTextureMapMode_Mirror:
                        // A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2
                        // is zero and 1-(u%1)|1-(v%1) otherwise.
                        // TODO
                        meshFuRef->mMaterialFormatData.wrapT = MeshFu::Geometry::TexWrapMode::Repeat;
                        break;
                    }
                }
            }

            unique_ptr<ModelPart> labmesh = convertMesh(mesh);

            meshFuRef->mValidCache = true;

            //   assimpMeshRef->mAnimatedPos.resize( mesh->mNumVertices );
            //   if ( mesh->HasNormals() )
            //   {
            //     assimpMeshRef->mAnimatedNorm.resize( mesh->mNumVertices );
            //   }
            //
            //
            //   meshFuRef->mIndices.resize( meshFuRef->mCachedMeshTri->mFaces.size()* 3 );
            //   unsigned j = 0;
            //   for ( unsigned x = 0; x < mesh->mNumFaces; ++x )
            //   {
            //     for ( unsigned a = 0; a < mesh->mFaces[x].mNumIndices; ++a)
            //     {
            //       meshFuRef->mIndices[ j++ ] = mesh->mFaces[ x ].mIndices[ a ];
            //     }
            //   }
            /*
            int nMorphs = mesh->mNumAnimMeshes;
            for (int nm = 0; nm < nMorphs; nm++)
            {
            aiAnimMesh* aiMesh = mesh->mAnimMeshes[nm];
            shared_ptr<MeshFu::PointCloud> morph = shared_ptr<MeshFu::PointCloud>(new MeshFu::PointCloud());
            morph->mName = aiMesh->mName.data;

            // copy vertices
            for ( unsigned i = 0; i < aiMesh->mNumVertices; ++i )
            {
            morph->mVertices.push_back( fromAssimp( aiMesh->mVertices[i] ) );
            }

            //TODO: something goes horribly wrong writing normals. investigate.
            //     if( aiMesh->HasNormals() )
            //     {
            //       for ( unsigned i = 0; i < aiMesh->mNumVertices; ++i )
            //       {
            //         morph->mNormals.push_back( fromAssimp( aiMesh->mNormals[i] ) );
            //       }
            //     }

            meshFuRef->mMorphs.push_back(morph);
            meshFuRef->mMorphWeights.push_back(0.0);
            }
            */
            size_t nVerts = meshFuRef->mVertices.size();
            std::vector<WeightPacker> packList;
            packList.resize(nVerts);
            meshFuRef->mBoneIndices.resize(nVerts);
            meshFuRef->mBoneWeights.resize(nVerts);

            int nBones = mesh->mNumBones;
            for (int nb = 0; nb < nBones; nb++)
            {
                aiBone* bone = mesh->mBones[nb];
                shared_ptr<MeshFu::Bone> boneRef = shared_ptr<MeshFu::Bone>(new MeshFu::Bone());
                boneRef->mName = bone->mName.data;

                int numWeights = bone->mNumWeights;
                for (int i = 0; i < numWeights; i++)
                {
                    int vertexIdx = bone->mWeights[i].mVertexId;
                    float weight = bone->mWeights[i].mWeight;
                    packList[vertexIdx].addWeight(float(nb), weight);
                    //boneRef->mWeights.push_back(fromAssimp(bone->mWeights[i]));
                }

                //boneRef->mOffsetMatrix = (m44f)bone->mOffsetMatrix;
                memcpy(&boneRef->mOffsetMatrix, &bone->mOffsetMatrix, sizeof(float) * 16);
                meshFuRef->mBones.push_back(boneRef);
            }
            for (int i = 0; i < nVerts; i++)
            {
                packList[i].getIndexAndWeights(meshFuRef->mBoneIndices[i], meshFuRef->mBoneWeights[i]);
            }

            // Assume mesh is visible
            //   meshFuRef->mVisible = true;

            return labmesh;
        }
#endif

        std::string intToString(int i)
        {
            std::stringstream ss;
            std::string s;
            ss << i;
            s = ss.str();
            return s;
        }


    } // anon

    namespace Render {

    std::shared_ptr<Model> loadMesh(const std::string& srcFilename)
    {
        std::string filename = lab::expandPath(srcFilename.c_str());
        std::string extension = filename.substr(filename.rfind('.') + 1);
        if (extension == "obj" || extension == "OBJ")
            return load_ObjMesh(srcFilename);

#ifdef HAVE_ASSIMP
        unsigned int flags =
            aiProcess_Triangulate
            | aiProcess_FlipUVs
            | aiProcess_FindInstances
            | aiProcess_ValidateDataStructure
            | aiProcess_OptimizeMeshes
            | aiProcess_CalcTangentSpace
            | aiProcess_GenSmoothNormals
            | aiProcess_JoinIdenticalVertices
            | aiProcess_ImproveCacheLocality
            | aiProcess_LimitBoneWeights
            | aiProcess_RemoveRedundantMaterials
            | aiProcess_SplitLargeMeshes
            | aiProcess_Triangulate
            | aiProcess_GenUVCoords
            | aiProcess_SortByPType
            //| aiProcess_FindDegenerates
            | aiProcess_FindInvalidData
            ;

        std::string filename = lab::expandPath(srcFilename.c_str());
        std::string baseDirectory = filename.substr(0, filename.rfind('/'));

        std::shared_ptr<Assimp::Importer> importer = std::shared_ptr< Assimp::Importer >(new Assimp::Importer());
        importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
        importer->SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, true);

        const aiScene* scene = importer->ReadFile(filename, flags);
        if (!scene) {
            std::cout << importer->GetErrorString() << std::endl;
            return std::shared_ptr<Model>();
        }

        vector<string> meshNames;
        shared_ptr<Model> mesh = std::make_shared<Model>();
        std::map<std::string, shared_ptr<Model>> meshMap;
        for (size_t i = 0; i < scene->mNumMeshes; ++i) {
            std::string name = scene->mMeshes[i]->mName.data;
            if (std::find(meshNames.begin(), meshNames.end(), name) != meshNames.end()) {
                name = name + "_" + intToString(int(i));
            }
            unique_ptr<ModelPart> modelPart = convertAiMesh(scene, scene->mMeshes[i], name, baseDirectory);
            mesh->addPart(std::move(modelPart));
            meshMap[name] = mesh;
            meshNames.push_back(name);
        }
        return mesh;
#endif
        return {};
    }

    namespace  // Local utility functions
    {
        void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
            float v10[3];
            v10[0] = v1[0] - v0[0];
            v10[1] = v1[1] - v0[1];
            v10[2] = v1[2] - v0[2];

            float v20[3];
            v20[0] = v2[0] - v0[0];
            v20[1] = v2[1] - v0[1];
            v20[2] = v2[2] - v0[2];

            N[0] = v10[1] * v20[2] - v10[2] * v20[1];
            N[1] = v10[2] * v20[0] - v10[0] * v20[2];
            N[2] = v10[0] * v20[1] - v10[1] * v20[0];

            float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
            if (len2 > 0.0f) {
                float len = sqrtf(len2);

                N[0] /= len;
                N[1] /= len;
                N[2] /= len;
            }
        }

        struct vec3 {
            float v[3];
            vec3() {
                v[0] = 0.0f;
                v[1] = 0.0f;
                v[2] = 0.0f;
            }
        };

        void normalizeVector(vec3& v) {
            float len2 = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
            if (len2 > 0.0f) {
                float len = sqrtf(len2);

                v.v[0] /= len;
                v.v[1] /= len;
                v.v[2] /= len;
            }
        }
    }

    static bool hasSmoothingGroup(const tinyobj::shape_t& shape)
    {
        for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
            if (shape.mesh.smoothing_group_ids[i] > 0) {
                return true;
            }
        }
        return false;
    }

    static void computeSmoothingNormals(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape,
        std::map<int, vec3>& smoothVertexNormals) {
        smoothVertexNormals.clear();
        std::map<int, vec3>::iterator iter;

        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            // Get the three indexes of the face (all faces are triangular)
            tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

            // Get the three vertex indexes and coordinates
            int vi[3];      // indexes
            float v[3][3];  // coordinates

            for (int k = 0; k < 3; k++) {
                vi[0] = idx0.vertex_index;
                vi[1] = idx1.vertex_index;
                vi[2] = idx2.vertex_index;
                assert(vi[0] >= 0);
                assert(vi[1] >= 0);
                assert(vi[2] >= 0);

                v[0][k] = attrib.vertices[3 * vi[0] + k];
                v[1][k] = attrib.vertices[3 * vi[1] + k];
                v[2][k] = attrib.vertices[3 * vi[2] + k];
            }

            // Compute the normal of the face
            float normal[3];
            CalcNormal(normal, v[0], v[1], v[2]);

            // Add the normal to the three vertexes
            for (size_t i = 0; i < 3; ++i) {
                iter = smoothVertexNormals.find(vi[i]);
                if (iter != smoothVertexNormals.end()) {
                    // add
                    iter->second.v[0] += normal[0];
                    iter->second.v[1] += normal[1];
                    iter->second.v[2] += normal[2];
                }
                else {
                    smoothVertexNormals[vi[i]].v[0] = normal[0];
                    smoothVertexNormals[vi[i]].v[1] = normal[1];
                    smoothVertexNormals[vi[i]].v[2] = normal[2];
                }
            }

        }  // f

        // Normalize the normals, that is, make them unit vectors
        for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
            iter++) {
            normalizeVector(iter->second);
        }

    }  // computeSmoothingNormals

    std::shared_ptr<Model> load_ObjMesh(const std::string& srcFilename_)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        float bmin[3];
        float bmax[3];

        auto model = std::make_shared<Model>();

        std::string srcFilename = lab::expandPath(srcFilename_.c_str());
        for (size_t i = 0; i < srcFilename.size(); ++i) {
            if (srcFilename[i] == '\\')
                srcFilename[i] = '/';
        }
        std::string base_dir = srcFilename.substr(0, srcFilename.find_last_of("/\\"));

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, srcFilename.c_str(),
            base_dir.c_str());
        if (!warn.empty()) {
            std::cout << "WARN: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }

        if (!ret) {
            std::cerr << "Failed to load " << filename << std::endl;
            return {};
        }

        printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
        printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
        printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
        printf("# of materials = %d\n", (int)materials.size());
        printf("# of shapes    = %d\n", (int)shapes.size());

        // Append `default` material
        materials.push_back(tinyobj::material_t());

        for (size_t i = 0; i < materials.size(); i++) {
            printf("material[%d].diffuse_texname = %s\n", int(i),
                materials[i].diffuse_texname.c_str());
        }

#if 0
        // Load diffuse textures
        {
            for (size_t m = 0; m < materials.size(); m++) {
                tinyobj::material_t* mp = &materials[m];

                if (mp->diffuse_texname.length() > 0) {
                    // Only load the texture if it is not already loaded
                    if (textures.find(mp->diffuse_texname) == textures.end()) {
                        GLuint texture_id;
                        int w, h;
                        int comp;

                        std::string texture_filename = mp->diffuse_texname;
                        if (!FileExists(texture_filename)) {
                            // Append base dir.
                            texture_filename = base_dir + mp->diffuse_texname;
                            if (!FileExists(texture_filename)) {
                                std::cerr << "Unable to find file: " << mp->diffuse_texname
                                    << std::endl;
                                exit(1);
                            }
                        }

                        unsigned char* image =
                            stbi_load(texture_filename.c_str(), &w, &h, &comp, STBI_default);
                        if (!image) {
                            std::cerr << "Unable to load texture: " << texture_filename
                                << std::endl;
                            exit(1);
                        }
                        std::cout << "Loaded texture: " << texture_filename << ", w = " << w
                            << ", h = " << h << ", comp = " << comp << std::endl;

                        glGenTextures(1, &texture_id);
                        glBindTexture(GL_TEXTURE_2D, texture_id);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        if (comp == 3) {
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                                GL_UNSIGNED_BYTE, image);
                        }
                        else if (comp == 4) {
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                                GL_UNSIGNED_BYTE, image);
                        }
                        else {
                            assert(0);  // TODO
                        }
                        glBindTexture(GL_TEXTURE_2D, 0);
                        stbi_image_free(image);
                        textures.insert(std::make_pair(mp->diffuse_texname, texture_id));
                    }
                }
            }
        }
#endif

        bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
        bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

        {
            for (size_t s = 0; s < shapes.size(); s++) {
                std::vector<float> buffer;  // pos(3float), normal(3float), color(3float)

                // Check for smoothing group and compute smoothing normals
                std::map<int, vec3> smoothVertexNormals;
                if (hasSmoothingGroup(shapes[s])) {
                    std::cout << "Compute smoothingNormal for shape [" << s << "]" << std::endl;
                    computeSmoothingNormals(attrib, shapes[s], smoothVertexNormals);
                }

                auto mesh = std::make_shared<ModelPart>();
                VAO* vertdata = new VAO(std::make_shared<Buffer<VertPTNC>>(BufferBase::BufferType::VertexBuffer));

                for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {
                    tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                    tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                    tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                    int current_material_id = shapes[s].mesh.material_ids[f];

                    if ((current_material_id < 0) ||
                        (current_material_id >= static_cast<int>(materials.size()))) {
                        // Invaid material ID. Use default material.
                        current_material_id =
                            materials.size() -
                            1;  // Default material is added to the last item in `materials`.
                    }
                    // if (current_material_id >= materials.size()) {
                    //    std::cerr << "Invalid material index: " << current_material_id <<
                    //    std::endl;
                    //}
                    //
                    float diffuse[3];
                    for (size_t i = 0; i < 3; i++) {
                        diffuse[i] = materials[current_material_id].diffuse[i];
                    }
                    float tc[3][2];
                    if (attrib.texcoords.size() > 0) {
                        if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                            (idx2.texcoord_index < 0)) {
                            // face does not contain valid uv index.
                            tc[0][0] = 0.0f;
                            tc[0][1] = 0.0f;
                            tc[1][0] = 0.0f;
                            tc[1][1] = 0.0f;
                            tc[2][0] = 0.0f;
                            tc[2][1] = 0.0f;
                        }
                        else {
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx0.texcoord_index + 1));
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx1.texcoord_index + 1));
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx2.texcoord_index + 1));

                            // Flip Y coord.
                            tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                            tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                            tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                            tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                            tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                            tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                        }
                    }
                    else {
                        tc[0][0] = 0.0f;
                        tc[0][1] = 0.0f;
                        tc[1][0] = 0.0f;
                        tc[1][1] = 0.0f;
                        tc[2][0] = 0.0f;
                        tc[2][1] = 0.0f;
                    }

                    float v[3][3];
                    for (int k = 0; k < 3; k++) {
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;
                        assert(f0 >= 0);
                        assert(f1 >= 0);
                        assert(f2 >= 0);

                        v[0][k] = attrib.vertices[3 * f0 + k];
                        v[1][k] = attrib.vertices[3 * f1 + k];
                        v[2][k] = attrib.vertices[3 * f2 + k];
                        bmin[k] = std::min(v[0][k], bmin[k]);
                        bmin[k] = std::min(v[1][k], bmin[k]);
                        bmin[k] = std::min(v[2][k], bmin[k]);
                        bmax[k] = std::max(v[0][k], bmax[k]);
                        bmax[k] = std::max(v[1][k], bmax[k]);
                        bmax[k] = std::max(v[2][k], bmax[k]);
                    }

                    float n[3][3];
                    {
                        bool invalid_normal_index = false;
                        if (attrib.normals.size() > 0) {
                            int nf0 = idx0.normal_index;
                            int nf1 = idx1.normal_index;
                            int nf2 = idx2.normal_index;

                            if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0)) {
                                // normal index is missing from this face.
                                invalid_normal_index = true;
                            }
                            else {
                                for (int k = 0; k < 3; k++) {
                                    assert(size_t(3 * nf0 + k) < attrib.normals.size());
                                    assert(size_t(3 * nf1 + k) < attrib.normals.size());
                                    assert(size_t(3 * nf2 + k) < attrib.normals.size());
                                    n[0][k] = attrib.normals[3 * nf0 + k];
                                    n[1][k] = attrib.normals[3 * nf1 + k];
                                    n[2][k] = attrib.normals[3 * nf2 + k];
                                }
                            }
                        }
                        else {
                            invalid_normal_index = true;
                        }

                        if (invalid_normal_index && !smoothVertexNormals.empty()) {
                            // Use smoothing normals
                            int f0 = idx0.vertex_index;
                            int f1 = idx1.vertex_index;
                            int f2 = idx2.vertex_index;

                            if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                                n[0][0] = smoothVertexNormals[f0].v[0];
                                n[0][1] = smoothVertexNormals[f0].v[1];
                                n[0][2] = smoothVertexNormals[f0].v[2];

                                n[1][0] = smoothVertexNormals[f1].v[0];
                                n[1][1] = smoothVertexNormals[f1].v[1];
                                n[1][2] = smoothVertexNormals[f1].v[2];

                                n[2][0] = smoothVertexNormals[f2].v[0];
                                n[2][1] = smoothVertexNormals[f2].v[1];
                                n[2][2] = smoothVertexNormals[f2].v[2];

                                invalid_normal_index = false;
                            }
                        }

                        if (invalid_normal_index) {
                            // compute geometric normal
                            CalcNormal(n[0], v[0], v[1], v[2]);
                            n[1][0] = n[0][0];
                            n[1][1] = n[0][1];
                            n[1][2] = n[0][2];
                            n[2][0] = n[0][0];
                            n[2][1] = n[0][1];
                            n[2][2] = n[0][2];
                        }

                        for (int k = 0; k < 3; k++) {

                            // Combine normal and diffuse to get color.
                            float normal_factor = 0.f;
                            float diffuse_factor = 1 - normal_factor;
                            float c[3] = { n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
                                           n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
                                           n[k][2] * normal_factor + diffuse[2] * diffuse_factor };
                            float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                            if (len2 > 0.0f) {
                                float len = sqrtf(len2);

                                c[0] /= len;
                                c[1] /= len;
                                c[2] /= len;
                            }

                            vertdata->vertexData<VertPTNC>(true)->push_back(VertPTNC(
                                v3f(v[k][0], v[k][1], v[k][2]),
                                V2F(tc[k][0], tc[k][1]),
                                V3F(n[k][0], n[k][1], n[k][2]),
                                V4F(c[0] * 0.5f + 0.5f, c[1] * 0.5f + 0.5f, c[2] * 0.5f + 0.5f, 1.f)));
                        }
                    }
                }

                Bounds bounds;
                bounds.first = { bmin[0], bmin[1], bmin[2] };
                bounds.second = { bmax[0], bmax[1], bmax[2] };
                mesh->setVAO(std::unique_ptr<VAO>(vertdata), bounds);
                std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();
                VAO* verts = mesh->verts();
                for (int tri_idx = 0; tri_idx < shapes[s].mesh.indices.size(); ++tri_idx)
                    indices->push_back(tri_idx);
                verts->setIndices(indices);

#if 0
                // OpenGL viewer does not support texturing with per-face material.
                if (shapes[s].mesh.material_ids.size() > 0 &&
                    shapes[s].mesh.material_ids.size() > s) {
                    o.material_id = shapes[s].mesh.material_ids[0];  // use the material ID
                                                                        // of the first face.
                }
                else {
                    o.material_id = materials.size() - 1;  // = ID for default material.
                }
                printf("shape[%d] material_id %d\n", int(s), int(o.material_id));
#endif

                model->addPart(mesh);
            }
        }

        printf("bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
        printf("bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);

        return model;
    }



    } // Render

}

