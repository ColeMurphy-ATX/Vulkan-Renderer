#pragma once

#include "window.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_renderer.hpp"

#include <memory>
#include <vector>

namespace ve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run();

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;
	private:

		void loadGameObjects();

		Window Window{ WIDTH, HEIGHT, "Hello Vulkan!" };

		veDevice veDevice{ Window };

		veRenderer veRenderer{ Window, veDevice };

		std::vector<veGameObject> gameObjects;
	};
}