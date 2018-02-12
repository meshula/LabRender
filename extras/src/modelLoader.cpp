
#include <LabRender/LabRender.h>
#include <LabRender/Model.h>
#include <LabRender/utils.h>

#define BUILDING_LABRENDER_MODELLOADER
#include "extras/modelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <sstream>


namespace lab
{
	using namespace std;

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

				void getIndexAndWeights(glm::vec4 &index, glm::vec4 &weight)
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
				vector<glm::vec3>    mVertices;
				vector<glm::vec3>    mNormals;
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
				vector<glm::vec2>        mTextureCoords;
				vector<glm::vec4>        mColors;


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
				glm::mat4x4           mOffsetMatrix;

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
					glm::vec4 dcolor;
					glm::vec4 scolor;
					glm::vec4 acolor;
					glm::vec4 ecolor;
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

				vector<glm::vec4>        mBoneIndices;
				vector<glm::vec4>        mBoneWeights;
			};
		} // MeshFu

		string parentPath(const string& path) {
			return path.substr(0, path.rfind('/'));
		}
		string filename(const string& path) {
			return path.substr(path.rfind('/') + 1, path.length());
		}

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
			meshFuRef->mMaterialData.dcolor = glm::vec4(dcolor.r, dcolor.g, dcolor.b, dcolor.a);

			if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_SPECULAR, scolor)) {
				//     meshFuRef->mMaterialData.hasScolor = true;
			}
			//   else
			//     meshFuRef->mMaterialData.hasScolor = false;
			meshFuRef->mMaterialData.scolor = glm::vec4(scolor.r, scolor.g, scolor.b, scolor.a);

			if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_AMBIENT, acolor)) {
				//     meshFuRef->mMaterialData.hasAcolor = true;
			}
			//   else
			//     meshFuRef->mMaterialData.hasAcolor = false;
			meshFuRef->mMaterialData.acolor = glm::vec4(acolor.r, acolor.g, acolor.b, acolor.a);

			if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_EMISSIVE, ecolor)) {
				//     meshFuRef->mMaterialData.hasEcolor = false;
			}
			//   else
			//     meshFuRef->mMaterialData.hasEcolor = false;
			meshFuRef->mMaterialData.ecolor = glm::vec4(ecolor.r, ecolor.g, ecolor.b, ecolor.a);

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

				//boneRef->mOffsetMatrix = (glm::mat4x4)bone->mOffsetMatrix;
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


		std::string intToString(int i)
		{
			std::stringstream ss;
			std::string s;
			ss << i;
			s = ss.str();
			return s;
		}


	} // anon

	std::shared_ptr<Model> loadMesh(const std::string& srcFilename)
	{
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
	}

}

