#include "first_app.hpp"

#include "simple_render_system.hpp"
#include "ve_camera.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "keyboard_movement_controller.hpp"

namespace ve {


	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp()
	{

	}

	void FirstApp::run() {

		SimpleRenderSystem simpleRenderSystem{ veDevice, veRenderer.getSwapChainRenderPass() };
        veCamera camera{};

        auto viewerObject = veGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!Window.shouldClose()) {
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(Window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = veRenderer.getAspectRatio();
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

			if (auto commandBuffer = veRenderer.beginFrame())
			{
				
				veRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);

				veRenderer.endSwapChainRenderPass(commandBuffer);
				veRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(veDevice.device()); //cpu will block until all gpu operations have completed, resources can be safely cleaned up since they will no longer be in use
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<veModel> veModel = veModel::createModelFromFile(veDevice, "models/MARLY1.obj");

        auto gameObject = veGameObject::createGameObject();
		gameObject.model = veModel;
		gameObject.transform.translation = { 0.0f, 0.0f, 100.0f };
		gameObject.transform.scale = glm::vec3{-1.0f};

        gameObjects.push_back(std::move(gameObject));
	}
}