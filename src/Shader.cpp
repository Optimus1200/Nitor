#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <glad/glad.h>

#include "Shader.h"

namespace ntr
{
    //#################################################################################################
    //
    // SHADER IMPLEMENTATION
    //
    //#################################################################################################

    Shader::Shader()
        : mID                        { 0 }
    {
    }
    
    Shader::Shader(const std::string& vertexFilepath, const std::string& fragmentFilepath, const std::string& geometryFilepath)
        : Shader{}
    {
        std::string vertexCode, fragmentCode, geometryCode;
        std::ifstream vShaderFile, fShaderFile, gShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vShaderFile.open(vertexFilepath);
            fShaderFile.open(fragmentFilepath);

            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if (geometryFilepath != "")
            {
                gShaderFile.open(geometryFilepath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR: " << e.what() << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "vertex");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "fragment");

        unsigned int geometry{};
        if (geometryFilepath != "")
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "geometry");
        }

        mID = glCreateProgram();
        glAttachShader(mID, vertex);
        glAttachShader(mID, fragment);

        if (geometryFilepath != "")
        {
            glAttachShader(mID, geometry);
        }

        glLinkProgram(mID);
        checkCompileErrors(mID, "program");

        // shaders are linked into program, so these are no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        if (geometryFilepath != "")
        {
            glDeleteShader(geometry);
        }
    }

    GLuint Shader::id() const
    {
        return mID;
    }

    void Shader::use() const
    {
        glUseProgram(mID);
    }

    void Shader::setBool(const std::string& name, bool value) const
    {
        glProgramUniform1i(mID, glGetUniformLocation(mID, name.c_str()), (int)value);
    }

    void Shader::setInt(const std::string& name, int value) const
    {
        glProgramUniform1i(mID, glGetUniformLocation(mID, name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const
    {
        glProgramUniform1f(mID, glGetUniformLocation(mID, name.c_str()), value);
    }

    void Shader::setVec2(const std::string& name, glm::vec2 value) const
    {
        glProgramUniform2fv(mID, glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec2(const std::string& name, float x, float y) const
    {
        glProgramUniform2f(mID, glGetUniformLocation(mID, name.c_str()), x, y);
    }

    void Shader::setVec3(const std::string& name, const glm::vec3& value) const
    {
        glProgramUniform3fv(mID, glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec3(const std::string& name, float x, float y, float z) const
    {
        glProgramUniform3f(mID, glGetUniformLocation(mID, name.c_str()), x, y, z);
    }
 
    void Shader::setVec4(const std::string& name, const glm::vec4& value) const
    {
        glProgramUniform4fv(mID, glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
    }
    
    void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
    {
        glProgramUniform4f(mID, glGetUniformLocation(mID, name.c_str()), x, y, z, w);
    }

    void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glProgramUniformMatrix2fv(mID, glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glProgramUniformMatrix3fv(mID, glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glProgramUniformMatrix4fv(mID, glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::draw(const Mesh& mesh)
    {
        glBindVertexArray(mesh.vao());
        glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Shader::draw(const Mesh* mesh)
    {
        glBindVertexArray(mesh->vao());
        glDrawElements(GL_TRIANGLES, mesh->indexCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Shader::draw(const MeshInstance& mesh)
    {
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Shader::draw(const Model& model)
    {
        const auto& meshes = model.meshes;

        for (const auto& [id, mesh] : meshes)
        {
            draw(mesh);
        }
    }

    void Shader::draw(const Model* model)
    {
        const auto& meshes = model->meshes;

        for (const auto& [id, mesh] : meshes)
        {
            draw(mesh);
        }
    }

    void Shader::bindTexture(GLint unit, const Texture& texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture.handle());
    }

    void Shader::bindTexture(GLint unit, TextureHandle texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void Shader::bindTexture(GLint unit, const DepthTexture2D& texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture.id());
    }

    void Shader::bindTexture(GLint unit, const std::string& name, const Texture& texture) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glUniform1i(glGetUniformLocation(mID, name.c_str()), unit);
        glBindTexture(GL_TEXTURE_2D, texture.handle());
    }

    void Shader::bindTexture(GLint unit, const std::string& name, TextureHandle texture) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glUniform1i(glGetUniformLocation(mID, name.c_str()), unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void Shader::bindTexture(GLint unit, const std::string& name, const DepthTexture2D& texture) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glUniform1i(glGetUniformLocation(mID, name.c_str()), unit);
        glBindTexture(GL_TEXTURE_2D, texture.id());
    }

    void Shader::bindTexture(GLint unit, const Texture2DArray& textureArray)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray.id());
    }

    void Shader::unbindTexture(GLint unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Shader::unbindTextures(GLint start_unit, GLint end_unit)
    {
        for (GLint i = start_unit; i <= end_unit; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    // Private helper functions

	void Shader::checkCompileErrors(const unsigned int& shaderID, const std::string& type) const
	{
        GLint success;
        GLchar infoLog[1024];
        if (type != "program")
        {
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
                std::cout << "ERROR: " << type << " shader compilation failed\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shaderID, 1024, NULL, infoLog);
                std::cout << "ERROR: " << type << " shader link failed\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
	}
} // namespace ntr


