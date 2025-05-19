#include <iostream>
#include <random>

#include "App.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace ntr
{
	App::App()
		: mWindow{ createWindow() }
		, mShaderPBR{ "shaders/ntr_pbr.vs", "shaders/ntr_pbr.fs" }
		, mShaderDepth{ "shaders/ntr_shadows_depth.vs", "shaders/ntr_shadows_depth.fs", "shaders/ntr_shadows_depth.gs" }
		, mShaderStencil{ "shaders/ntr_stencil.vs", "shaders/ntr_stencil.fs" }
		, mDebugShaderShadows{ "shaders/ntr_debug_quad.vs", "shaders/ntr_debug_quad.fs" }
		, mScene{}
		, mShadowCascadeLevels{
			mScene.selectedCamera.zFar / 50.0f,
			mScene.selectedCamera.zFar / 25.0f,
			mScene.selectedCamera.zFar / 10.0f,
			mScene.selectedCamera.zFar / 2.0f
		}
		, mLightDepthMaps{ M_SHADOW_RESOLUTION, mShadowCascadeLevels.size() + 1 }
	{
		M_VSYNC_ENABLED ? glfwSwapInterval(1) : glfwSwapInterval(0);

		centerWindowToScreen();
		glfwShowWindow(mWindow);
		Gui::init(mWindow);
	}

	App::~App()
	{
		Gui::terminate();
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void App::run()
	{
		// configure Light FBO

		mLightFBO.bind();
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mLightDepthMaps.id(), 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}

		mLightFBO.unbind();

		// Configure SSBO

		ArrayBuffer<glm::mat4> ssboLightMatrices(16, nullptr, 0, GL_DYNAMIC_DRAW);
		ArrayBuffer<float> ssboCascadePlaneDistances(16, nullptr, 1, GL_DYNAMIC_COPY);

		// assets

		const Model* modelPlane = mScene.loadModel("Plane", "models/plane/Plane.fbx");
		const Model* modelCube = mScene.loadModel("Cube", "models/cube/Cube.fbx");

		// entities with model components

		Transform transformPlane;
		transformPlane.scale = { 16.0f, 1.0f, 16.0f };

		Transform transformCube;
		transformCube.position = { 0.0f, 1.0f, 0.0f };

		entt::entity entityPlane = addEntityModel3D("entity_plane_1", modelPlane, transformPlane);
		entt::entity entityCube = addEntityModel3D("entity_cube_1", modelCube, transformCube);

		Rect framebuffer = getWindowFramebufferRect();
		mScene.selectedCamera.viewport = { 0.0f, 0.0f, framebuffer.width, framebuffer.height };
		mScene.selectedCamera.position = { 16.0f, 8.0f, 16.0f };
		mScene.selectedCamera.rotation = { -23.0f, -135.0f, 0.0f };

		// shader config

		mShaderPBR.setInt("shadowMap", 5);
		mDebugShaderShadows.setInt("depthMap", 0);

		// time logic

		const float TARGET_FRAME_TIME_SECONDS = 1.0f / M_TARGET_FPS;

		// render loop
		
		while (!glfwWindowShouldClose(mWindow))
		{
			float deltaTimeSeconds = getDeltaTimeSeconds();

			while (M_TARGET_FPS != 0 && deltaTimeSeconds < TARGET_FRAME_TIME_SECONDS)
			{
				deltaTimeSeconds += getDeltaTimeSeconds();
			}

			glfwPollEvents();

			if (isWindowMinimized())
			{
				continue;
			}

			processViewerMovement(deltaTimeSeconds);
			processViewerRotation();

			mShadowCascadeLevels = {
				mScene.selectedCamera.zFar / 32.0f,
				mScene.selectedCamera.zFar / 16.0f,
				mScene.selectedCamera.zFar / 8.0f,
				mScene.selectedCamera.zFar / 4.0f,
				mScene.selectedCamera.zFar / 2.0f
			};

			// 0. SSBO setup

			const std::vector<glm::mat4> lightMatrices = getLightSpaceMatrices(mScene.selectedCamera, mScene.directionalLight.direction, mShadowCascadeLevels);
			ssboLightMatrices.bind();
			ssboLightMatrices.update(0, lightMatrices.size(), lightMatrices.data());
			ssboLightMatrices.unbind();

			// 1. Render Scene Depth

			renderDepth(mLightFBO);

			// 2. Render scene as normal

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // color range: [0.0f, 1.0f]
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mShaderPBR.use();
			mShaderPBR.setMat4("view", mScene.selectedCamera.view());
			mShaderPBR.setMat4("projection", mScene.selectedCamera.projection());
			mShaderPBR.setVec3("directionalLight.color", mScene.directionalLight.color);
			mShaderPBR.setVec3("directionalLight.direction", mScene.directionalLight.direction);
			mShaderPBR.setVec3("cameraPosition", mScene.selectedCamera.position);
			mShaderPBR.setFloat("cameraFarPlane", mScene.selectedCamera.zFar);

			size_t cascadeCount = mShadowCascadeLevels.size();

			std::vector<float> cascadePlaneDistances;
			cascadePlaneDistances.reserve(cascadeCount);

			for (size_t i = 0; i < cascadeCount; ++i)
			{
				cascadePlaneDistances.push_back(mShadowCascadeLevels[i]);
			}

			ssboCascadePlaneDistances.bind();
			ssboCascadePlaneDistances.update(0, cascadePlaneDistances.size(), cascadePlaneDistances.data());
			ssboCascadePlaneDistances.unbind();

			// enable stencil buffer writing, draw selected entity

			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);

			glClear(GL_STENCIL_BUFFER_BIT);

			const auto entitySelectedView = mScene.registry.view<ConstPointer<Model>, Transform, Selected>();

			for (const auto& [entity, model, transform] : entitySelectedView.each())
			{
				renderModelPBR(model, transform);
			}

			// disable stencil buffer writing, draw unselected entities

			glStencilMask(0x00);

			const auto entityView = mScene.registry.view<ConstPointer<Model>, Transform>(entt::exclude<Selected>);

			for (const auto& [entity, model, transform] : entityView.each())
			{
				renderModelPBR(model, transform);
			}

			//renderDebugQuad();

			renderGui();

			// update window title each second

			static float winTitleElapsedTimeSeconds = 0.0f;

			winTitleElapsedTimeSeconds += deltaTimeSeconds;

			if (winTitleElapsedTimeSeconds >= 1.0f)
			{
				glfwSetWindowTitle(mWindow, (M_WINDOW_TITLE + " (FPS: " + std::to_string(1.0f / deltaTimeSeconds) + ") (FT: " + std::to_string(deltaTimeSeconds * 1000.0f) + "ms)").c_str());
				winTitleElapsedTimeSeconds = 0.0f;
			}

			// --------------------------------

			glfwSwapBuffers(mWindow);
		}
	}

	GLFWwindow* App::createWindow()
	{
		glfwSetErrorCallback(glfw_error_callback);

		// Init GLFW to create window

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, M_OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, M_OPENGL_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		if (M_OPENGL_DEBUG_MODE)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		}

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		GLFWwindow* window = glfwCreateWindow(M_RESOLUTION_WIDTH, M_RESOLUTION_HEIGHT, M_WINDOW_TITLE.c_str(), nullptr, nullptr);
		if (!window)
		{
			std::cerr << "ERROR: failed to create window" << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window,
			[](GLFWwindow* window, int width, int height) {
				App* app = (App*)glfwGetWindowUserPointer(window);
				app->mScene.selectedCamera.viewport.width = (float)width;
				app->mScene.selectedCamera.viewport.height = (float)height;
				app->setViewport(app->mScene.selectedCamera.viewport);
			}
		);

		// Resize window such that framebuffer dimensions match resolution

		int windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		
		glfwSetWindowSize(
			window,
			windowWidth + (windowWidth - framebufferWidth),
			windowHeight + (windowHeight - framebufferHeight)
			);

		// Init GLAD to call OpenGL functions

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "ERROR: failed to initialize GLAD" << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		// Configure OpenGL

		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glEnable(GL_CULL_FACE);

		return window;
	}

	void App::centerWindowToScreen()
	{
		const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		int centerX = (vidMode->width - M_RESOLUTION_WIDTH) / 2;
		int centerY = (vidMode->height - M_RESOLUTION_HEIGHT) / 2;

		glfwSetWindowPos(mWindow, centerX, centerY);
	}

	bool App::isWindowMinimized() const
	{
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);
		return width == 0 || height == 0;
	}

	float App::getDeltaTimeSeconds() const
	{
		static float prevTimeSeconds = 0.0f;

		float currTimeSeconds	= (float)glfwGetTime();
		float deltaTimeSeconds	= currTimeSeconds - prevTimeSeconds;
		prevTimeSeconds			= currTimeSeconds;

		return deltaTimeSeconds;
	}

	void App::setViewport(const Rect& rect)
	{
		glViewport((GLint)rect.x, (GLint)rect.y, (GLsizei)rect.width, (GLsizei)rect.height);
	}

	entt::entity App::addEntityModel3D(const std::string& id, const Model* model, const Transform& transform)
	{
		static unsigned int entNum = 0;
		std::string idToUse = "";

		if (id == "")
		{
			idToUse = "entity_" + std::to_string(++entNum);
		}
		else
		{
			idToUse = id;
		}

		// handle duplicate id

		auto entityView = mScene.registry.view<StringID>();
		bool duplicateFound;

		do
		{
			duplicateFound = false;

			for (auto& [ent, id] : entityView.each())
			{
				if (idToUse == id.str)
				{
					duplicateFound = true;
					break;
				}
			}

			if (duplicateFound)
			{
				idToUse += "+";
			}
		} while (duplicateFound);

		// create entity
		entt::entity ent = mScene.registry.create();
		mScene.registry.emplace<StringID>(ent, idToUse);
		mScene.registry.emplace<ConstPointer<Model>>(ent, model);
		mScene.registry.emplace<Transform>(ent, transform);

		return ent;
	}

	void App::processViewerMovement(float deltaTimeSeconds)
	{
		// Only process camera movement if gui isn't using keyboard
		if (ImGui::GetIO().WantCaptureKeyboard)
		{
			return;
		}

		float speed = 10.0f;

		if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			speed *= 5.0f;
		}

		glm::vec3 velocity(0.0f);
		glm::vec3 cFront = mScene.selectedCamera.front();
		glm::vec3 cRight = mScene.selectedCamera.right();
		
		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
		{
			velocity += cFront * speed * deltaTimeSeconds;
		}
		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
		{
			velocity += -cRight * speed * deltaTimeSeconds;
		}
		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
		{
			velocity += -cFront * speed * deltaTimeSeconds;
		}
		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
		{
			velocity += cRight * speed * deltaTimeSeconds;
		}

		mScene.selectedCamera.position += velocity;
	}

	void App::processViewerRotation()
	{
		// Only process rotation if ImGui isn't using the mouse
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return;
		}

		static double lastX = 0; // Center of the screen
		static double lastY = 0; // Center of the screen
		static bool firstMouse = true;

		double xpos = 0.0;
		double ypos = 0.0;

		glfwGetCursorPos(mWindow, &xpos, &ypos);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top

		lastX = xpos;
		lastY = ypos;

		if (glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
		{
			return;
		}

		xoffset *= mScene.selectedCamera.sensitivity;
		yoffset *= mScene.selectedCamera.sensitivity;

		glm::vec3 rotation = mScene.selectedCamera.rotation;
		rotation.y += (float)xoffset;
		rotation.x += (float)yoffset;

		// Clamp the pitch angle to prevent flipping
		if (rotation.x > 89.0f)
		{
			rotation.x = 89.0f;
		}
		if (rotation.x < -89.0f)
		{
			rotation.x = -89.0f;
		}

		mScene.selectedCamera.rotation = rotation;
	}

	void App::renderDepth(const FrameBuffer& lightFBO)
	{
		// Render depth of scene to texture (from light's perpective)

		mLightFBO.bind();
		setViewport({ 0.0f, 0.0f, (float)M_SHADOW_RESOLUTION, (float)M_SHADOW_RESOLUTION });
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// Save original state before modifying
		GLint cullFaceMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);

		glCullFace(GL_FRONT); // peter panning

		mShaderDepth.use();

		auto entityView = mScene.registry.view<ConstPointer<Model>, Transform>();

		for (const auto& [entity, model, transform] : entityView.each())
		{
			glm::mat4 modelMatrix = transform.matrix();

			const auto& meshes = model->meshes;

			for (const auto& [id, mesh] : meshes)
			{
				mShaderDepth.setMat4("model", modelMatrix * mesh.transform.matrix());
				mShaderDepth.draw(mesh);
			}
		}

		// Restore original state
		glCullFace(cullFaceMode);

		mLightFBO.unbind();

		// reset viewport
		setViewport(mScene.selectedCamera.viewport);
	}

	void App::renderModelPBR(const Model* model, const Transform& transform)
	{
		glm::mat4 modelMatrix = transform.matrix();

		const auto& meshes = model->meshes;

		for (const auto& [id, mesh] : meshes)
		{
			glm::mat4 finalMatrix = modelMatrix * mesh.transform.matrix();

			mShaderPBR.setMat4("model", finalMatrix);
			mShaderPBR.setMat3("normal", glm::transpose(glm::inverse(glm::mat3(finalMatrix))));
			
			mShaderPBR.bindTexture(0, "material.albedo",    mesh.material->albedo);
			mShaderPBR.bindTexture(1, "material.normal",    mesh.material->normal);
			mShaderPBR.bindTexture(2, "material.roughness", mesh.material->roughness);
			mShaderPBR.bindTexture(3, "material.metallic",  mesh.material->metallic);
			mShaderPBR.bindTexture(4, "material.occlusion", mesh.material->occlusion);
			mShaderPBR.bindTexture(5, mLightDepthMaps);
			mShaderPBR.draw(mesh);
		}
	}
	
	void App::renderGui()
	{
		Gui::clear();

		if (mFileExplorer.isOpen())
		{
			mFileExplorer.render();
		}

		renderMenuBar();
		renderAssetsWindow();
		renderSceneWindow();

		Gui::draw();
	}

	void App::renderMenuBar()
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Import Model"))
			{
				mFileExplorer.setTitle("Import Model");
				mFileExplorer.setOpenButtonTitle("Import");
				mFileExplorer.filterFileTypes({ "obj", "fbx", "gltf", "glb"});

				mFileExplorer.setOnFileOpenCallback([this]()
					{
						const auto& files = mFileExplorer.getSelectedFiles();

						for (const auto& path : files)
						{
							std::string filename = path.filename().string();
							std::string modelID = filename.substr(0, filename.find_last_of('.'));

							while (mScene.findModel(modelID) != Model::EMPTY)
							{
								modelID += "+";
							}

							Model* model = mScene.loadModel(modelID, path);

							addEntityModel3D(modelID, model);
						}
					});

				mFileExplorer.open();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("[Entity] 3D Model"))
			{
				addEntityModel3D();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void App::renderAssetsWindow()
	{
		ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoMove);

		ImGui::SetWindowPos(ImVec2(0, 18));

		renderAssetsWindowSectionMeshes();
		renderAssetsWindowSectionTextures();
		renderAssetsWindowSectionModels();
		renderAssetsWindowSectionMaterials();

		ImGui::End();
	}

	void App::renderAssetsWindowSectionMeshes()
	{
		if (ImGui::CollapsingHeader("Meshes"))
		{
			static const Mesh* selectedMesh = Mesh::EMPTY;
			static bool showRenameError = false;
			static std::string duplicateID = "";
			
			Gui::EditableSelectableResult selectedResult;
			const auto& MESH_MAP = mScene.getMeshMap();

			// Iterate through selectable Meshes

			for (const auto& [ID, mesh] : MESH_MAP)
			{
				ImGui::Bullet();
				ImGui::SameLine();

				bool isSelected = selectedMesh == mesh;

				ImGui::PushID(mesh);

				auto result = Gui::EditableSelectable(ID, isSelected);

				ImGui::PopID();

				if (result.shouldSelect)
				{
					selectedMesh = mesh;
					selectedResult = result;
				}
			}

			// Process selected Mesh
			
			if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
			{
				selectedMesh = Mesh::EMPTY;
				showRenameError = false;
			}
			else if (selectedResult.shouldRename && selectedMesh != Mesh::EMPTY)
			{
				if (mScene.findMesh(selectedResult.newName) == Mesh::EMPTY)
				{
					mScene.replaceMeshID(mScene.findMeshID(selectedMesh), selectedResult.newName);
					showRenameError = false;
				}
				else
				{
					showRenameError = true;
					duplicateID = selectedResult.newName;
				}
			}
			else if (selectedResult.shouldDelete && selectedMesh != Mesh::EMPTY)
			{
				mScene.removeMesh(mScene.findMeshID(selectedMesh));
				selectedMesh = Mesh::EMPTY;
				showRenameError = false;
			}

			if (showRenameError)
			{
				Gui::showErrorTooltip("Mesh ID \'" + duplicateID + "\' already exists!");
			}
		}
	}

	void App::renderAssetsWindowSectionTextures()
	{
		if (ImGui::CollapsingHeader("Textures"))
		{
			static TextureHandle textureSelected = Texture::EMPTY;
			static bool showRenameError = false;
			static std::string duplicateID = "";
			
			Gui::EditableSelectableResult selectedResult;
			const auto& textureMap = mScene.getTextureMap();

			for (const auto& [ID, TEXTURE] : textureMap)
			{
				ImGui::Bullet();
				ImGui::SameLine();

				bool isSelected = TEXTURE.handle() == textureSelected;

				auto currResult = Gui::EditableSelectable(ID, isSelected);

				if (currResult.shouldSelect)
				{
					textureSelected = TEXTURE.handle();
					selectedResult = currResult;
				}
			}

			
			if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
			{
				textureSelected = Texture::EMPTY;
				showRenameError = false;

			}
			else if (selectedResult.shouldRename && textureSelected != Texture::EMPTY)
			{
				if (mScene.findTexture(selectedResult.newName) == Texture::EMPTY)
				{
					mScene.replaceTextureID(mScene.findTextureID(textureSelected), selectedResult.newName);
				}
				else
				{
					showRenameError = true;
					duplicateID = selectedResult.newName;
					
				}
			}
			else if (selectedResult.shouldDelete && textureSelected != Texture::EMPTY)
			{
				mScene.removeTexture(mScene.findTextureID(textureSelected));
				textureSelected = Texture::EMPTY;
				showRenameError = false;
			}

			if (showRenameError)
			{
				Gui::showErrorTooltip("Texture ID \'" + duplicateID + "\' already exists!");
			}

			if (ImGui::Button("Load"))
			{
				mFileExplorer.setTitle("Load Texture");
				mFileExplorer.setOpenButtonTitle("Load");
				mFileExplorer.filterFileTypes({ "jpg", "png" });

				mFileExplorer.setOnFileOpenCallback([this]()
					{
						const auto& files = mFileExplorer.getSelectedFiles();

						for (const auto& path : files)
						{
							std::string textureID = path.filename().string();

							while (mScene.findTexture(textureID) != Texture::EMPTY)
							{
								textureID += "+";
							}

							mScene.loadTexture(textureID, path);
						}
					});

				mFileExplorer.open();
			}
		}
	}

	void App::renderAssetsWindowSectionModels()
	{
		if (ImGui::CollapsingHeader("Models"))
		{
			static Model* selectedModel = Model::EMPTY;
			static bool showModelRenameError = false;
			static std::string duplicateModelID = "";
			Gui::EditableTreeNodeResult selectedModelResult;

			bool addMeshButtonClicked = false;

			auto& meshMap = mScene.getMeshMap();
			auto& modelMap = mScene.getModelMap();
			auto& materialMap = mScene.getMaterialMap();

			size_t numModels = modelMap.size();
			size_t numMaterials = materialMap.size();

			unsigned int meshHeaderID = 0;
			unsigned int transformHeaderID = 0;
			unsigned int materialHeaderID = 0;

			for (auto& [modID, model] : modelMap)
			{
				static bool showModMeshRenameError = false;
				static std::string duplicateModMeshID = "";

				bool isModelSelected = (model == selectedModel);

				ImGui::PushID(("##Model" + modID).c_str());

				Gui::EditableTreeNodeResult currModelResult = Gui::EditableTreeNode(modID.c_str(), isModelSelected);

				ImGui::PopID();
				
				if (currModelResult.shouldSelect)
				{
					selectedModel = model;
					selectedModelResult = currModelResult;
				}

				if (currModelResult.isOpen)
				{
					static std::string selectedModMeshID = "None";
					static MeshInstance* selectedModMesh = nullptr;
					Gui::EditableTreeNodeResult selectedModMeshResult;

					ImGui::Indent();

					auto& meshes = model->meshes;

					for (auto& [modMeshID, modMesh] : meshes)
					{
						bool isMeshSelected = (selectedModMesh == &modMesh);

						ImGui::PushID(("##ModelMesh" + modMeshID).c_str());

						auto currMeshResult = Gui::EditableTreeNode(modMeshID.c_str(), isMeshSelected);

						ImGui::PopID();

						if (currMeshResult.shouldSelect)
						{
							selectedModMeshID = modMeshID;
							selectedModMesh = &modMesh;
							selectedModMeshResult = currMeshResult;
						}

						if (currMeshResult.isOpen)
						{
							MeshInstance& mutModMesh = model->meshes[modMeshID];

							ImGui::Indent();

							if (ImGui::TreeNode(("Mesh##" + std::to_string(++meshHeaderID)).c_str()))
							{
								if (ImGui::BeginCombo("##MeshData", mScene.findMeshID(modMesh).c_str()))
								{
									bool noMeshSelected = true;

									for (const auto& [name, mesh] : meshMap)
									{
										const bool IS_SELECTED = (mesh->vao() == modMesh.vao);

										if (ImGui::Selectable(name.c_str(), IS_SELECTED))
										{
											mutModMesh.vao = mesh->vao();
											mutModMesh.indexCount = mesh->indexCount();
											noMeshSelected = false;
										}
									}

									if (ImGui::Selectable("None", noMeshSelected))
									{
										mutModMesh.vao = Mesh::EMPTY->vao();
										mutModMesh.indexCount = Mesh::EMPTY->indexCount();
									}

									ImGui::EndCombo(); // MeshData
								}

								ImGui::TreePop(); // Mesh
							}

							if (ImGui::TreeNode(("Transform##" + std::to_string(++transformHeaderID)).c_str()))
							{
								MeshInstance& mutModMesh = model->meshes[modMeshID];

								ImGui::DragFloat3("Position", &mutModMesh.transform.position.x, 0.1f);
								ImGui::DragFloat3("Rotation", &mutModMesh.transform.rotation.x, 0.1f);
								ImGui::DragFloat3("Scale", &mutModMesh.transform.scale.x, 0.1f);

								ImGui::TreePop();
							}

							if (ImGui::TreeNode(("Material##" + std::to_string(++materialHeaderID)).c_str()))
							{
								if (ImGui::BeginCombo("##MaterialData", mScene.findMaterialID(modMesh.material).c_str()))
								{
									bool noMaterialSelected = true;

									for (const auto& [name, material] : materialMap)
									{
										const bool IS_SELECTED = (material == modMesh.material);

										if (ImGui::Selectable(name.c_str(), IS_SELECTED))
										{
											mutModMesh.material = material;
											noMaterialSelected = false;
										}
									}

									if (ImGui::Selectable("None", noMaterialSelected))
									{
										mutModMesh.material = mScene.getDefaultMaterial();
									}

									ImGui::EndCombo(); // Material
								}

								ImGui::TreePop(); // Material
							}

							ImGui::Unindent(); // CurrMesh
						}
					}

					if (ImGui::Button(("Add Mesh##" + std::string(modID)).c_str()))
					{
						static unsigned int meshNum = 0;

						model->meshes.try_emplace(
							"Mesh " + std::to_string(++meshNum),
							Mesh::EMPTY,
							mScene.getDefaultMaterial()
						);
					}

					ImGui::Unindent(); // CurrModel

					// Process selected mod mesh

					// Deselect if clicked outside
					if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
					{
						selectedModMeshID = "None";
						selectedModMesh = nullptr;
						showModMeshRenameError = false;
						duplicateModMeshID = "";
					}
					else if (selectedModMeshResult.shouldRename && selectedModMesh != nullptr)
					{
						auto itr = model->meshes.find(selectedModMeshResult.newName);

						if (itr == model->meshes.end())
						{
							model->meshes[selectedModMeshResult.newName] = std::move(*selectedModMesh);
							model->meshes.erase(selectedModMeshID);
							showModMeshRenameError = false;
							duplicateModMeshID = "";
						}
						else
						{
							
							showModMeshRenameError = true;
							duplicateModMeshID = selectedModMeshResult.newName;
						}
					}
					else if (selectedModMeshResult.shouldDelete && selectedModMesh != nullptr)
					{
						model->meshes.erase(selectedModMeshID);
						selectedModMeshID = "None";
						showModelRenameError = false;
						duplicateModMeshID = "";
					}

					if (showModMeshRenameError)
					{
						Gui::showErrorTooltip("Model Mesh ID \'" + duplicateModMeshID + "\' for Model already exists!");
					}
				}
			}

			if (selectedModelResult.shouldDelete && selectedModel != Model::EMPTY)
			{
				mScene.removeModel(mScene.findModelID(selectedModel));
				selectedModel = Model::EMPTY;
				showModelRenameError = false;
				duplicateModelID = "";
			}
			else if (selectedModelResult.shouldRename && selectedModel != Model::EMPTY)
			{
				if (mScene.findModel(selectedModelResult.newName) == Model::EMPTY)
				{
					mScene.replaceModelID(mScene.findModelID(selectedModel), selectedModelResult.newName);
					showModelRenameError = false;
					duplicateModelID = "";
				}
				else
				{
					showModelRenameError = true;
					duplicateModelID = selectedModelResult.newName;
				}
			}
			// Deselect if clicked outside
			else if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
			{
				selectedModel = Model::EMPTY;
				showModelRenameError = false;
				duplicateModelID = "";
			}

			if (showModelRenameError)
			{
				Gui::showErrorTooltip("Model ID \'" + duplicateModelID + "\' alreaady exists!");
			}
		}
	}

	void App::renderAssetsWindowSectionMaterials()
	{
		if (ImGui::CollapsingHeader("Materials"))
		{
			static bool showRenameError;
			static std::string duplicateID = "";

			ImGui::Bullet();

			static char textBuffer[256] = "";

			if ( ( ImGui::InputTextWithHint("##text", "Add Material...", textBuffer, sizeof(textBuffer), ImGuiInputTextFlags_EnterReturnsTrue) || (ImGui::SameLine(), ImGui::Button("Enter")) )
				&& textBuffer[0] != '\0')
			{
				if (mScene.findMaterial(textBuffer) == Material::EMPTY)
				{
					mScene.addMaterial(textBuffer, {});
					textBuffer[0] = '\0';
					showRenameError = false;
				}
				else
				{
					showRenameError = true;
					duplicateID = textBuffer;
				}
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				textBuffer[0] = '\0';
				showRenameError = false;
			}

			static Material* selectedMaterial = Material::EMPTY;

			Gui::EditableTreeNodeResult selectedResult;

			const auto& textureMap = mScene.getTextureMap();
			const auto& MATERIAL_MAP = mScene.getMaterialMap();

			const Material* const defaultMaterial = mScene.getDefaultMaterial();

			size_t numMaterials = MATERIAL_MAP.size();

			for (auto& [matID, material] : MATERIAL_MAP)
			{
				bool isSelected = material == selectedMaterial;

				auto currResult = Gui::EditableTreeNode(matID.c_str(), isSelected);

				if (currResult.shouldSelect)
				{
					selectedMaterial = material;
					selectedResult = currResult;
				}

				if (currResult.isOpen)
				{
					// Dropdown menus previewing currently selected textures for current material

					ImGui::Indent();

					if (ImGui::BeginCombo(("Albedo##" + matID).c_str(), mScene.findTextureID(material->albedo).c_str()))
					{
						bool noneSelected = true;

						for (const auto& [texID, texture] : textureMap)
						{
							const bool IS_SELECTED = (texture.handle() == material->albedo);

							if (ImGui::Selectable(texID.c_str(), IS_SELECTED))
							{
								material->albedo = texture.handle();
								noneSelected = false;
							}
						}

						if (ImGui::Selectable(("None##" + matID).c_str(), noneSelected))
						{
							material->albedo = defaultMaterial->albedo;
						}

						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo(("Normal##" + matID).c_str(), mScene.findTextureID(material->normal).c_str()))
					{
						bool noneSelected = true;

						for (const auto& [texID, texture] : textureMap)
						{
							const bool IS_SELECTED = (texture.handle() == material->normal);

							if (ImGui::Selectable(texID.c_str(), IS_SELECTED))
							{
								material->normal = texture.handle();
								noneSelected = false;
							}
						}

						if (ImGui::Selectable(("None##" + matID).c_str(), noneSelected))
						{
							material->normal = defaultMaterial->normal;
						}

						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo(("Roughness##" + matID).c_str(), mScene.findTextureID(material->roughness).c_str()))
					{
						bool noneSelected = true;

						for (const auto& [texID, texture] : textureMap)
						{
							const bool IS_SELECTED = (texture.handle() == material->roughness);

							if (ImGui::Selectable(texID.c_str(), IS_SELECTED))
							{
								material->roughness = texture.handle();
								noneSelected = false;
							}
						}

						if (ImGui::Selectable(("None##" + matID).c_str(), noneSelected))
						{
							material->roughness = defaultMaterial->roughness;
						}

						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo(("Metallic##" + matID).c_str(), mScene.findTextureID(material->metallic).c_str()))
					{
						bool noneSelected = true;

						for (const auto& [texID, texture] : textureMap)
						{
							const bool IS_SELECTED = (texture.handle() == material->metallic);

							if (ImGui::Selectable(texID.c_str(), IS_SELECTED))
							{
								material->metallic = texture.handle();
								noneSelected = false;
							}
						}

						if (ImGui::Selectable(("None##" + matID).c_str(), noneSelected))
						{
							material->metallic = defaultMaterial->metallic;
						}

						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo(("Occlusion##" + matID).c_str(), mScene.findTextureID(material->occlusion).c_str()))
					{
						bool noneSelected = true;

						for (const auto& [texID, texture] : textureMap)
						{
							const bool IS_SELECTED = (texture.handle() == material->occlusion);

							if (ImGui::Selectable(texID.c_str(), IS_SELECTED))
							{
								material->occlusion = texture.handle();
								noneSelected = false;
							}
						}

						if (ImGui::Selectable(("None##" + matID).c_str(), noneSelected))
						{
							material->occlusion = defaultMaterial->occlusion;
						}

						ImGui::EndCombo();
					}

					ImGui::Unindent();
				}
			}

			// Deselect if clicked outside
			if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
			{
				selectedMaterial = Material::EMPTY;
				showRenameError = false;
			}
			else if (selectedResult.shouldRename && selectedMaterial != Material::EMPTY)
			{
				if (mScene.findMaterial(selectedResult.newName) == Material::EMPTY)
				{
					mScene.replaceMaterialID(mScene.findMaterialID(selectedMaterial), selectedResult.newName);
					showRenameError = false;
				}
				else
				{
					showRenameError = true;
					duplicateID = selectedResult.newName;
					
				}
			}
			else if (selectedResult.shouldDelete && selectedMaterial != Material::EMPTY)
			{
				mScene.removeMaterial(mScene.findMaterialID(selectedMaterial));
				selectedMaterial = Material::EMPTY;
				showRenameError = false;
			}

			if (showRenameError)
			{
				Gui::showErrorTooltip("Material ID \'" + duplicateID + "\' already exists!");
			}
		}
	}

	void App::renderSceneWindow()
	{
		ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove);

		// LIGHT

		if (ImGui::CollapsingHeader("Sunlight"))
		{
			ImGui::Text("Color    ");
			ImGui::SameLine();
			ImGui::DragFloat3("##Color", &mScene.directionalLight.color.x, 0.01f);

			mScene.directionalLight.color.x = std::max(0.0f, mScene.directionalLight.color.x);
			mScene.directionalLight.color.y = std::max(0.0f, mScene.directionalLight.color.y);
			mScene.directionalLight.color.z = std::max(0.0f, mScene.directionalLight.color.z);

			ImGui::Text("Direction");
			ImGui::SameLine();
			ImGui::DragFloat3("##Direction", &mScene.directionalLight.direction.x, 0.001f);

			mScene.directionalLight.direction.x = std::clamp<float>(mScene.directionalLight.direction.x, -1.0f, 1.0f);
			mScene.directionalLight.direction.y = std::clamp<float>(mScene.directionalLight.direction.y, -1.0f, 1.0f);
			mScene.directionalLight.direction.z = std::clamp<float>(mScene.directionalLight.direction.z, -1.0f, 1.0f);
		}

		// CAMERA

		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::Text("Position");
			ImGui::SameLine();
			ImGui::DragFloat3("##Position", &mScene.selectedCamera.position.x, 0.1f);

			ImGui::Text("Rotation");
			ImGui::SameLine();
			ImGui::DragFloat3("##Rotation", &mScene.selectedCamera.rotation.x, 0.1f);

			ImGui::Text("FOV     ");
			ImGui::SameLine();
			ImGui::DragFloat("##FOV", &mScene.selectedCamera.fovY, 0.01f);
		}

		// ENTITIES 

		static entt::entity	entitySelected = entt::null;
		static entt::entity	entityDeselected = entt::null;

		Gui::EditableSelectableResult selectedResult;

		Rect fbRect = getWindowFramebufferRect();
		ImVec2 winSize = ImGui::GetWindowSize();

		ImGui::SetWindowPos(ImVec2(fbRect.width - winSize.x, 18));

		if (ImGui::CollapsingHeader("Entities"))
		{
			static bool showRenameError = false;
			static std::string duplicateID = "";

			auto entityView = mScene.registry.view<StringID>();

			for (auto [entity, id] : entityView.each())
			{
				const bool IS_SELECTED = (entity == entitySelected);

				ImGui::Bullet();

				auto currentResult = Gui::EditableSelectable(id.str, IS_SELECTED);

				if (currentResult.shouldSelect)
				{
					entityDeselected = entitySelected;
					entitySelected = entity;
					selectedResult = currentResult;
				}
			}

			// Process selected result

			// Deselect if clicked outside
			if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
			{
				entityDeselected = entitySelected;
				entitySelected = entt::null;
				showRenameError = false;
			}
			else if (selectedResult.shouldDelete && entitySelected != entt::null)
			{
				mScene.registry.destroy(entitySelected);
				entityDeselected = entt::null;
				entitySelected = entt::null;
				showRenameError = false;
			}
			else if (selectedResult.shouldRename && entitySelected != entt::null)
			{
				for (auto [entity, id] : entityView.each())
				{
					if (id.str == selectedResult.newName)
					{
						showRenameError = true;
						duplicateID = selectedResult.newName;
						break;
					}
				}

				if (!showRenameError)
				{
					StringID& id = mScene.registry.get<StringID>(entitySelected);
					id.str = selectedResult.newName;
				}
			}

			if (showRenameError)
			{
				Gui::showErrorTooltip("Entity ID \'" + duplicateID + "\' already exists!");
			}
		}
		
		if (entitySelected != entt::null)
		{
			auto& model = mScene.registry.get<ConstPointer<Model>>(entitySelected);
			auto& transform = mScene.registry.get<Transform>(entitySelected);

			// STENCIL OUTLINE RENDER

			// Save original state before modifying
			GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
			GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			// draw borders of selected entity
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);

			mShaderStencil.use();

			mShaderStencil.setMat4("view", mScene.selectedCamera.view());
			mShaderStencil.setMat4("projection", mScene.selectedCamera.projection());
			mShaderStencil.setFloat("outlineThickness", 50.0f);

			glm::mat4 modelMatrix = transform.matrix();

			const auto& meshes = model->meshes;

			for (const auto& [id, mesh] : meshes)
			{

				mShaderStencil.setMat4("model", modelMatrix * mesh.transform.matrix());
				
				mShaderStencil.draw(mesh);
			}

			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 0, 0xFF);

			// Restore original state 
			if (depthTestEnabled) { glEnable(GL_DEPTH_TEST); }
			if (cullFaceEnabled) { glEnable(GL_CULL_FACE); }

			// PROPERTIES TAB

			if (ImGui::CollapsingHeader("Properties"))
			{
				// render Transform section

				if (ImGui::TreeNode("Transform"))
				{
					ImGui::Text("Position");
					ImGui::SameLine();
					ImGui::DragFloat3("##Position", &transform.position.x, 0.1f);

					ImGui::Text("Rotation");
					ImGui::SameLine();
					ImGui::DragFloat3("##Rotation", &transform.rotation.x, 0.1f);

					ImGui::Text("Scale   ");
					ImGui::SameLine();
					ImGui::DragFloat3("##Scale", &transform.scale.x, 0.1f);

					ImGui::TreePop();
				}

				// render Model section

				if (ImGui::TreeNode("Model"))
				{
					if (ImGui::BeginCombo("##model", mScene.findModelID(model).c_str()))
					{
						const auto& MODEL_MAP = mScene.getModelMap();
						bool noneSelected = true;

						for (const auto& [id, curr_model] : MODEL_MAP)
						{
							const bool IS_SELECTED = (curr_model == model);

							if (ImGui::Selectable(id.c_str(), IS_SELECTED))
							{
								model = curr_model;
								noneSelected = false;
							}
						}

						if (ImGui::Selectable("None", noneSelected))
						{
							model = Model::EMPTY;
						}

						ImGui::EndCombo();
					}

					ImGui::TreePop();
				}
			}
		}

		// Process selected/deselected entity
		if (entityDeselected != entt::null)
		{
			mScene.registry.erase<Selected>(entityDeselected);
			entityDeselected = entt::null;
		}
		if (entitySelected != entt::null)
		{
			mScene.registry.emplace_or_replace<Selected>(entitySelected);
		}

		ImGui::End();
	}

	void App::renderDebugQuad()
	{
		static GLuint quadVAO = 0;
		static GLuint quadVBO;
		static int debugLayer = 0;

		mDebugShaderShadows.use();
		mDebugShaderShadows.setInt("layer", debugLayer);
		mDebugShaderShadows.bindTexture(0, mLightDepthMaps);

		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}

		glViewport(
			(GLint)mScene.selectedCamera.viewport.x, 
			(GLint)mScene.selectedCamera.viewport.y,
			(GLsizei)(mScene.selectedCamera.viewport.width / 5), 
			(GLsizei)(mScene.selectedCamera.viewport.height / 5)
		);
		
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		
		setViewport(mScene.selectedCamera.viewport);
	}

	Rect App::getWindowFramebufferRect() const
	{
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);
		return Rect(0, 0, (float)width, (float)height);
	}

	std::vector<glm::vec4> App::getFrustumCornersWorldSpace(const glm::mat4& projView)
	{
		const glm::mat4 INV = glm::inverse(projView);

		std::vector<glm::vec4> frustumCorners;
		frustumCorners.reserve(8);

		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 PT = 
						INV * glm::vec4(
							2.0f * x - 1.0f, 
							2.0f * y - 1.0f, 
							2.0f * z - 1.0f, 
							1.0f);
					frustumCorners.emplace_back(PT / PT.w);
				}
			}
		}

		return frustumCorners;
	}

	glm::mat4 App::getLightSpaceMatrix(const Camera& camera, const glm::vec3& lightDir, const float nearPlane, const float farPlane)
	{
		Rect framebuffer = getWindowFramebufferRect();

		const glm::mat4 proj = glm::perspective(
			glm::radians(camera.fovY), framebuffer.width / framebuffer.height, nearPlane, farPlane
		);

		const std::vector<glm::vec4> corners = getFrustumCornersWorldSpace(proj * camera.view());

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners)
		{
			center += glm::vec3(v);
		}
		center /= corners.size();

		const glm::mat4 lightView = glm::lookAt(center + lightDir, center, Camera::WORLD_UP);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene
		constexpr float zMult = 10.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		return lightProjection * lightView;
	}

	std::vector<glm::mat4> App::getLightSpaceMatrices(const Camera& camera, const glm::vec3& lightDir, const std::vector<float>& shadowCascadeLevels)
	{
		size_t sclSize = shadowCascadeLevels.size();
		size_t retSize = sclSize + 1;

		std::vector<glm::mat4> ret;
		ret.reserve(retSize);

		for (size_t i = 0; i < retSize; ++i)
		{
			if (i == 0)
			{
				ret.push_back(getLightSpaceMatrix(camera, -lightDir, camera.zNear, shadowCascadeLevels[i]));
			}
			else if (i < sclSize)
			{
				ret.push_back(getLightSpaceMatrix(camera, -lightDir, shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
			}
			else
			{
				ret.push_back(getLightSpaceMatrix(camera, -lightDir, shadowCascadeLevels[i - 1], camera.zFar));
			}
		}
		
		return ret;
	}

	void App::glfw_error_callback(int error, const char* description)
	{
		std::cerr << "GLFW ERROR " << error << ": " << description << std::endl;
	}

	void ntr::App::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
	{
		{
			// ignore non-significant error/warning codes
			if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

			std::cout << "---------------" << std::endl;
			std::cout << "Debug message (" << id << "): " << message << std::endl;

			switch (source)
			{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
			} std::cout << std::endl;

			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
			} std::cout << std::endl;

			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
			} std::cout << std::endl;
			std::cout << std::endl;
		}
	}
} // namespace ntr