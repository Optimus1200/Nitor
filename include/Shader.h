#ifndef NTR_SHADER_H
#define NTR_SHADER_H

#include <filesystem>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "Pointer.h"
#include "Scene.h"
#include "Texture.h"
#include "Transform.h"

namespace ntr
{
	class Shader
	{
	public:

		Shader();
		Shader(const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& geometryFilepath = "");

		GLuint id() const;
		void use() const;

		void setBool(const std::string& name, bool value) const;
		void setInt(const std::string& name, int value) const;
		void setFloat(const std::string& name, float value) const;
		void setVec2(const std::string& name, glm::vec2 value) const;
		void setVec2(const std::string& name, float x, float y) const;
		void setVec3(const std::string& name, const glm::vec3& value) const;
		void setVec3(const std::string& name, float x, float y, float z) const;
		void setVec4(const std::string& name, const glm::vec4& value) const;
		void setVec4(const std::string& name, float x, float y, float z, float w) const;
		void setMat2(const std::string& name, const glm::mat2& mat) const;
		void setMat3(const std::string& name, const glm::mat3& mat) const;
		void setMat4(const std::string& name, const glm::mat4& mat) const;

		void draw(const Mesh& mesh);
		void draw(const Mesh* mesh);
		void draw(const MeshInstance& mesh);
		void draw(const Model& model);
		void draw(const Model* model);
		
		void bindTexture(GLint unit, const Texture& texture);
		void bindTexture(GLint unit, TextureHandle texture);
		void bindTexture(GLint unit, const DepthTexture2D& texture);
		void bindTexture(GLint unit, const std::string& name, const Texture& texture) const;
		void bindTexture(GLint unit, const std::string& name, TextureHandle texture) const;
		void bindTexture(GLint unit, const std::string& name, const DepthTexture2D& texture) const;
		void bindTexture(GLint unit, const Texture2DArray& textureArray);
		void unbindTexture(GLint unit);

		// Unbinds texture units in range [start, end].
		void unbindTextures(GLint start_unit, GLint end_unit);

	private:

		GLuint mID;

		void checkCompileErrors(const unsigned int& shaderID, const std::string& type) const;
	};
} // namespace ntr

#endif