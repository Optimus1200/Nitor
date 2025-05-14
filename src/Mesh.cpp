#include <iostream>

#include "Mesh.h"

namespace ntr
{
    const ScopedPointer<Mesh> Mesh::EMPTY = new Mesh();

    Mesh::Mesh()
        : mVAO{ 0 }
        , mVBO{ 0 }
        , mEBO{ 0 }
        , mVertices{}
        , mIndices{}
        , mRenderUsage{}
    {
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, RenderUsage usage)
		: mVertices{ vertices }
		, mIndices{ indices }
        , mRenderUsage{ usage }
	{
        initMesh();
	}

    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<GLuint>&& indices, RenderUsage usage)
        : mVertices{ std::move(vertices) }
        , mIndices{ std::move(indices) }
        , mRenderUsage{ usage }
    {
        initMesh();
    }
    
    Mesh::Mesh(Mesh&& mesh) noexcept
        : mVAO{ std::move(mesh.mVAO) }
        , mVBO{ std::move(mesh.mVBO) }
        , mEBO{ std::move(mesh.mEBO) }
        , mVertices{ std::move(mesh.mVertices) }
        , mIndices{ std::move(mesh.mIndices) }
        , mRenderUsage{ std::move(mesh.mRenderUsage) }
    {
        mesh.mVAO = 0;
        mesh.mVBO = 0;
        mesh.mEBO = 0;
    }

    Mesh& Mesh::operator=(Mesh&& mesh) noexcept
    {
        std::swap(mVAO, mesh.mVAO);
        std::swap(mVBO, mesh.mVBO);
        std::swap(mEBO, mesh.mEBO);
        std::swap(mVertices, mesh.mVertices);
        std::swap(mIndices, mesh.mIndices);
        std::swap(mRenderUsage, mesh.mRenderUsage);

        return *this;
    }

    Mesh::~Mesh()
    {
        if (mVAO != 0)
        {
            glDeleteVertexArrays(1, &mVAO);
        }
        if (mVBO != 0)
        {
            glDeleteBuffers(1, &mVBO);
        }
        if (mEBO != 0)
        {
            glDeleteBuffers(1, &mEBO);
        }
    }

    GLuint Mesh::vao() const
    {
        return mVAO;
    }

    GLsizei Mesh::indexCount() const
    {
        return static_cast<GLsizei>(mIndices.size());
    }

    void Mesh::printVertices() const
    {
        for (const Vertex& v : mVertices)
        {
            std::cout << "{ " << " { " << v.position.x << ", " << v.position.y << ", " << v.position.z << " }, "
                << "{ " << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << " }, "
                << "{ " << v.texCoords.x << ", " << v.texCoords.y << " }, "
                << "{ " << v.tangent.x << ", " << v.tangent.y << ", " << v.tangent.z << " }, "
                << "{ " << v.biTangent.x << ", " << v.biTangent.y << ", " << v.biTangent.z << " } " << " }, "
                << std::endl;
        }
    }

    void Mesh::printIndices() const
    {
        std::cout << "{ ";

        for (GLuint i : mIndices)
        {
            std::cout << i << ", ";
        }

        std::cout << " }";
    }
    
    // Private helper function
    
    void Mesh::initMesh()
    {
        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], mRenderUsage);

        glGenBuffers(1, &mEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), &mIndices[0], mRenderUsage);

        // set the vertex attribute pointers

        // vertex positions
        glEnableVertexAttribArray(Vertex::INDEX_POSITION);
        glVertexAttribPointer(Vertex::INDEX_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(Vertex::INDEX_NORMAL);
        glVertexAttribPointer(Vertex::INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(Vertex::INDEX_TEXCOORDS);
        glVertexAttribPointer(Vertex::INDEX_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        // vertex tangent
        glEnableVertexAttribArray(Vertex::INDEX_TANGENT);
        glVertexAttribPointer(Vertex::INDEX_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        // vertex bitangent
        glEnableVertexAttribArray(Vertex::INDEX_BITANGENT);
        glVertexAttribPointer(Vertex::INDEX_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, biTangent));
        // ids
        glEnableVertexAttribArray(Vertex::INDEX_BONE_IDS);
        glVertexAttribIPointer(Vertex::INDEX_BONE_IDS, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
        // weights
        glEnableVertexAttribArray(Vertex::INDEX_BONE_WEIGHTS);
        glVertexAttribPointer(Vertex::INDEX_BONE_WEIGHTS, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    MeshInstance MeshInstance::EMPTY(Mesh::EMPTY, Material::EMPTY);
    
    MeshInstance::MeshInstance(const Mesh* mesh, const Material* material, const Transform& transform)
        : vao{ mesh->vao() }
        , indexCount{ mesh-> indexCount() }
        , material{ material }
        , transform{ transform }
    {
    }
} // namespace ntr