#ifndef NTR_MESH_H
#define NTR_MESH_H

#include <string>
#include <vector>

#include <glad/glad.h>

#include "Material.h"
#include "Math.h"
#include "Texture.h"
#include "Transform.h"
#include "Vertex.h"

namespace ntr
{
	enum RenderUsage : GLenum
	{
		DYNAMIC	= GL_DYNAMIC_DRAW,	// The data store contents will be modified repeatedly and used many times.
		STATIC	= GL_STATIC_DRAW,	// The data store contents will be modified once and used many times. 
		STREAM	= GL_STREAM_DRAW	// The data store contents will be modified once and used at most a few times.
	};

	class Mesh
	{
	public:

		static const ScopedPointer<Mesh> EMPTY;

		Mesh();
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, RenderUsage usage = RenderUsage::DYNAMIC);
		Mesh(std::vector<Vertex>&& vertices, std::vector<GLuint>&& indices, RenderUsage usage = RenderUsage::DYNAMIC);

		Mesh(const Mesh& mesh)				= delete;
		Mesh& operator=(const Mesh& mesh)	= delete;

		Mesh(Mesh&& mesh)				noexcept;
		Mesh& operator=(Mesh&& mesh)	noexcept;
		
		~Mesh();

		GLuint vao() const;
		GLsizei	indexCount() const;

		void printVertices() const;
		void printIndices() const;
	
	private:
		
		GLuint					mVAO;
		GLuint					mVBO;
		GLuint					mEBO;
		
		std::vector<Vertex>		mVertices;
		std::vector<GLuint>		mIndices;
		RenderUsage				mRenderUsage;

		void initMesh();
	};

	struct MeshInstance
	{
		static MeshInstance EMPTY;

		MeshInstance(const Mesh* mesh = Mesh::EMPTY, const Material* material = Material::EMPTY, const Transform& transform = {});

		GLuint			vao;
		GLsizei			indexCount;
		const Material*	material;
		Transform		transform;
	};
} // namespace ntr

#endif