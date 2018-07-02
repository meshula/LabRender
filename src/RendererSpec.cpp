#pragma once

#include "LabRender/RendererSpec.h"
#include "LabRender/SemanticType.h"
#include "LabRender/Utils.h"
#include "json/json.h"
#include <fstream>

namespace lab {

	using namespace std;

	void RendererSpec::reset()
	{
		filepath.clear();
		name.clear();
		version.clear();
		textures.clear();
		buffers.clear();
		passes.clear();
	}

	void RendererSpec::load(const std::string & filepath_)
	{
		reset();
		string filepath = filepath_;
		string p = expandPath(filepath.c_str());
		std::ifstream in(p);
		Json::Value conf;
		in >> conf;

		name = conf["name"].asString();
		version = conf["version"].asString();

		printf("\nTextures:\n");
		for (Json::Value::iterator it = conf["textures"].begin(); it != conf["textures"].end(); ++it)
		{
			// { "id": "tex16", "path": "{ASSET_ROOT}/textures/shadertoy/tex16.png" }
			string id = (*it)["id"].asString();
			string path = (*it)["path"].asString();
			textures.emplace_back(Texture(id, path));
		}

		printf("\nBuffers:\n");
		for (Json::Value::iterator it = conf["buffers"].begin(); it != conf["buffers"].end(); ++it)
		{
			Buffer buffer;
			buffer.name = (*it)["name"].asString();
			printf(" %s\n", buffer.name.c_str());
			Json::Value depth_buffer = (*it)["depth"];
			if (depth_buffer.type() != Json::nullValue)
				buffer.depth_buffer = depth_buffer.asString() == "yes";

			for (Json::Value::iterator it2 = (*it)["render_textures"].begin(); it2 != (*it)["render_textures"].end(); ++it2)
			{
				Texture texture;
				texture.name = (*it2)["name"].asString();
				string typeStr = (*it2)["type"].asString();
				printf("  %s %s\n", texture.name.c_str(), typeStr.c_str());
				texture.type = stringToTextureType(typeStr);
				Json::Value scale = (*it2)["scale"];
				if (scale.type() != Json::nullValue)
					texture.scale = scale.asFloat();
				buffer.render_textures.emplace_back(texture);
			}
			buffers.emplace_back(buffer);
		}

		printf("\nPasses:\n");
		int passNumber = 0;
		for (Json::Value::iterator it = conf["passes"].begin(); it != conf["passes"].end(); ++it)
		{
			Pass pass;
			Json::Value passVal = (*it)["type"];
			string passType = passVal["run"].asString();
			pass.name = (*it)["name"].asCString();
			printf(" %s %s\n", pass.name.c_str(), passType.c_str());

			Json::Value shader = (*it)["shader"];
			if (shader.type() != Json::nullValue)
			{
				Json::Value vertex_shader_path = shader["vertex_shader_path"];
				if (vertex_shader_path.type() != Json::nullValue)
					pass.shader.vertex_shader_filepath = vertex_shader_path.asString();

				Json::Value fragment_shader_path = shader["fragment_shader_path"];
				if (fragment_shader_path.type() != Json::nullValue)
					pass.shader.fragment_shader_filepath = fragment_shader_path.asString();

				Json::Value fragment_shader_postamble_path = shader["fragment_shader_postamble_path"];
				if (fragment_shader_postamble_path.type() != Json::nullValue)
					pass.shader.fragment_shader_postamble_filepath = fragment_shader_postamble_path.asString();

				Json::Value uniforms = shader["uniforms"];
				if (uniforms.type() != Json::nullValue)
				{
					for (Json::Value::iterator uniform = uniforms.begin(); uniform != uniforms.end(); ++uniform)
					{
						Shader::Var var;
						var.name = (*uniform)["name"].asString();
						string typeStr = (*uniform)["type"].asString();
						var.type = semanticTypeNamed(typeStr.c_str());

						AutomaticUniform automatic = AutomaticUniform::none;
						Json::Value automaticValue = (*uniform)["auto"];
						if (automaticValue.type() != Json::nullValue)
						{
							auto s = automaticValue.asCString();
							var.automatic = stringToAutomaticUniform(s);
						}

						Json::Value v = (*uniform)["texture"];
						if (v.type() != Json::nullValue) {
							var.pass_texture = v.asString();
						}

						pass.shader.uniforms.emplace_back(var);
					}
				}
				Json::Value varyings = shader["varyings"];
				if (varyings.type() != Json::nullValue)
				{
					for (Json::Value::iterator varying = varyings.begin(); varying != varyings.end(); ++varying)
					{
						Shader::Var var;
						var.name = (*varying)["name"].asString();
						string typeStr = (*varying)["type"].asString();
						var.type = semanticTypeNamed(typeStr.c_str());
						pass.shader.varyings.emplace_back(var);
					}
				}
			}

			string drawType = passVal["draw"].asString();
			if (drawType == "quad")
				pass.draw = Pass::Draw::Quad;
			else if (drawType == "opaque-geometry")
				pass.draw = Pass::Draw::OpaqueGeometry;

			bool writeDepth = pass.depth_write;
			DepthTest dfunc = pass.depth_test;

			Json::Value depth = (*it)["depth"];
			if (depth.type() != Json::nullValue)
			{
				Json::Value val = depth["write"];
				if (val.type() != Json::nullValue)
					pass.depth_write = !strcmp(val.asCString(), "yes");
				val = depth["test"];
				if (val.type() != Json::nullValue) {
					string df = val.asString();
					pass.depth_test = stringToDepthTest(df);
				}
				val = depth["clear_buffer"];
				if (val.type() != Json::nullValue)
					pass.clear_depth_buffer = val.asString() == "yes";
			}

			Json::Value writeBuffer = (*it)["outputs"];
			if (writeBuffer.type() != Json::nullValue)
			{
				pass.output.name = writeBuffer["buffer"].asString();
				for (Json::Value::iterator attachment = writeBuffer["render_textures"].begin();
					attachment != writeBuffer["render_textures"].end(); ++attachment)
				{
					pass.output.textures.push_back((*attachment).asString());
				}

				Json::Value val = writeBuffer["clear_buffer"];
				if (val.type() != Json::nullValue)
					pass.clear_output_buffer = val.asString() == "yes";
			}

			Json::Value readBuffers = (*it)["inputs"];
			if (readBuffers.type() != Json::nullValue)
			{
				for (Json::Value::iterator buffer = readBuffers.begin(); buffer != readBuffers.end(); ++buffer)
				{
					IOBuffer io;
					io.name = (*buffer)["buffer"].asString();
					Json::Value attachments = (*buffer)["render_textures"];
					for (Json::Value::iterator attachment = attachments.begin(); attachment != attachments.end(); ++attachment) {
						string a = (*attachment).asString();
						io.textures.push_back(a);
					}
					pass.inputs.emplace_back(io);
				}
			}
		}
	}


}
