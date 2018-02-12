#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/DepthTest.h>
#include <LabRender/TextureType.h>
#include <LabRender/Uniform.h>
#include <string>

namespace lab
{



	class RendererSpec
	{
	public:

		class Texture
		{
		public:
			Texture() {}
			Texture(const std::string& name, const std::string & filepath)
				: name(name), filepath(filepath) {}

			std::string name;
			std::string filepath;
			TextureType type = TextureType::none;
			float scale = 1.f;
		};

		class Buffer
		{
		public:
			std::string name;
			std::vector<Texture> render_textures;
			bool depth_buffer = false;
		};

		class Shader
		{
		public:
			class Var
			{
			public:
				std::string name;
				SemanticType type;
				std::string pass_texture;
				AutomaticUniform automatic = AutomaticUniform::none;
			};

			std::string vertex_shader_filepath;
			std::string fragment_shader_filepath;
			std::string fragment_shader_postamble_filepath;
			std::vector<Var> uniforms;
			std::vector<Var> varyings;
		};

		class IOBuffer
		{
		public:
			std::string name;
			std::vector<std::string> textures;
		};

		class Pass
		{
		public:
			enum class Draw {
				None, Quad, OpaqueGeometry,
			};

			std::string name;
			bool active = true;
			Draw draw = Draw::None;

			DepthTest depth_test = DepthTest::less;
			bool depth_write = false;
			bool clear_depth_buffer = false;

			Shader shader;

			std::vector<IOBuffer> inputs;

			IOBuffer output;
			bool clear_output_buffer = false;
		};



		void load(const std::string & filepath);
		void reset();

		std::string filepath;
		std::string name;
		std::string version;

		std::vector<Texture> textures;
		std::vector<Buffer> buffers;
		std::vector<Pass> passes;
	};

}
