#ifndef NTR_APP_H
#define NTR_APP_H

#include <string>
#include <thread>
#include <unordered_set>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ArrayBuffer.h"
#include "Buffers.h"
#include "Gui.h"
#include "Pointer.h"
#include "Scene.h"
#include "Shader.h"
#include "Image.h"
#include "Texture.h"

namespace ntr
{
	struct StringID
	{
		std::string str;
	};

	struct Selected
	{
	};

	class App
	{
	public:

		App();
		~App();

		void run();

	private:

		const bool			M_OPENGL_DEBUG_MODE		= false;
		const int			M_OPENGL_VERSION_MAJOR	= 4;
		const int			M_OPENGL_VERSION_MINOR	= 6;
		const int			M_RESOLUTION_WIDTH		= 1920;
		const int			M_RESOLUTION_HEIGHT		= 1080;
		const std::string	M_WINDOW_TITLE			= "Nitor";
		const int			M_TARGET_FPS			= 0;
		const bool			M_VSYNC_ENABLED			= true;
		const int			M_SHADOW_RESOLUTION		= 8192;

		GLFWwindow*			mWindow;
		Shader				mShaderPBR;
		Shader				mShaderDepth;
		Shader				mShaderStencil;
		Shader				mDebugShaderShadows;
		Scene				mScene;

		std::vector<float>	mShadowCascadeLevels;
		FrameBuffer			mLightFBO;
		Texture2DArray		mLightDepthMaps;

		FileExplorer		mFileExplorer;

		GLFWwindow*	createWindow();
		void		centerWindowToScreen();

		bool isWindowMinimized() const;

		float getDeltaTimeSeconds() const;

		void setViewport(const Rect& rect);

		entt::entity addEntityModel3D(const std::string& id = "", const Model* model = Model::EMPTY, const Transform& transform = {});

		void	processViewerMovement(float deltaTimeSeconds);
		void	processViewerRotation();
		void	renderDepth(const FrameBuffer& lightFBO);
		void	renderModelPBR(const Model* model, const Transform& transform);
		
		void	renderGui();
		void	renderMenuBar();
		void	renderAssetsWindow();
		void	renderAssetsWindowSectionMeshes();
		void	renderAssetsWindowSectionTextures();
		void	renderAssetsWindowSectionModels();
		void	renderAssetsWindowSectionMaterials();
		void	renderSceneWindow();

		void renderDebugQuad();

		Rect getWindowFramebufferRect() const;

		std::vector<glm::vec4>	getFrustumCornersWorldSpace(const glm::mat4& projView);
		glm::mat4				getLightSpaceMatrix(const Camera& camera, const glm::vec3& lightDir, const float nearPlane, const float farPlane);
		std::vector<glm::mat4>	getLightSpaceMatrices(const Camera& camera, const glm::vec3& lightDir, const std::vector<float>& shadowCascadeLevels);

		static void glfw_error_callback(int error, const char* description);

		static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
			GLsizei length, const char* message, const void* userParam);
	};
}

#endif
