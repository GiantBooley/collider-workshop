#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <nlohmann/json.hpp>

#include "math.hpp"
#include "sprite.hpp"
#include "IImageData.hpp"
#include "ImageData.hpp"

using json = nlohmann::json;

bool readFileText(std::string path, std::string* text) {
	std::ifstream file(path);
	if (file.fail()) {
		std::cout << "[ERROR] failed to load file \"" << path << "\"" << std::endl;
		return false;
	}
	std::string str;
	std::string file_contents = "";
	while (getline(file, str)) {
		file_contents += str;
		file_contents.push_back('\n');
	}
	*text = file_contents;
	return true;
}
std::string filePathToName(std::string path) {
	int forwardslash = path.find_last_of('/');
	int backslash = path.find_last_of('\\');
	int slash = std::max(forwardslash, backslash);
	int dot = path.find_last_of('.');
	if (dot == std::string::npos || slash == std::string::npos) return path;
	return path.substr(slash + 1, dot - (slash + 1));
}



class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath) {
		std::string vertexText;
		std::string fragmentText;
		std::ifstream vertexFile;
		std::ifstream fragmentFile;
		vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			vertexFile.open(vertexPath);
			fragmentFile.open(fragmentPath);
			std::stringstream vertexStream, fragmentStream;
			vertexStream << vertexFile.rdbuf();
			fragmentStream << fragmentFile.rdbuf();
			vertexFile.close();
			fragmentFile.close();
			vertexText = vertexStream.str();
			fragmentText = fragmentStream.str();
		} catch (std::ifstream::failure e) {
			std::cout << "[ERROR] failed to get fragment or vertex text" << std::endl;
		}
		const char* vertexCode = vertexText.c_str();
		const char* fragmentCode = fragmentText.c_str();

		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		//vertex n fragment
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexCode, NULL);
		glCompileShader(vertex);
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentCode, NULL);
		glCompileShader(fragment);

		//errors shaders
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "[ERROR] vertex shader compile failed\n" << infoLog << std::endl;
		}
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "[ERROR] fragment shader compile failed\n" << infoLog << std::endl;
		}

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		//errors
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "[ERROR] program failed linking\n" << infoLog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	void use() {
		glUseProgram(ID);
	}
};
class SpriteShader : public Shader {
public:
	unsigned int textureLoc, transformLoc, boundsLoc;
	SpriteShader(const char* vertexPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		textureLoc = glGetUniformLocation(ID, "texture");
		transformLoc = glGetUniformLocation(ID, "transform");
		boundsLoc = glGetUniformLocation(ID, "bounds");
	}
};
class LineShader : public Shader {
public:
	unsigned int transformLoc, colorLoc;
	LineShader(const char* vertexPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		transformLoc = glGetUniformLocation(ID, "transform");
		colorLoc = glGetUniformLocation(ID, "color");
	}
};
class CircleShader : public Shader {
public:
	unsigned int transformLoc, colorLoc;
	CircleShader(const char* vertexPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		transformLoc = glGetUniformLocation(ID, "transform");
		colorLoc = glGetUniformLocation(ID, "color");
	}
};

int frameWidth = 640;
int frameHeight = 480;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	frameWidth = width;
	frameHeight = height;
}
bool ctrlKeyDown = false;
bool sPressed = false;
bool gPressed = false;
bool rightPressed = false;
bool leftPressed = false;
bool shiftDown = false;
bool tabPressed = false;
bool spaceDown = false;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, true);
		}
		if (key == GLFW_KEY_LEFT_CONTROL) {
			ctrlKeyDown = true;
		}
		if (key == GLFW_KEY_S) {
			sPressed = true;
		}
		if (key == GLFW_KEY_G) {
			gPressed = true;
		}
		if (key == GLFW_KEY_RIGHT) {
			rightPressed = true;
		}
		if (key == GLFW_KEY_LEFT) {
			leftPressed = true;
		}
		if (key == GLFW_KEY_TAB) {
			tabPressed = true;
		}
		if (key == GLFW_KEY_SPACE) {
			spaceDown = true;
		}
		if (key == GLFW_KEY_LEFT_SHIFT) shiftDown = true;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_CONTROL) {
			ctrlKeyDown = false;
		}
		if (key == GLFW_KEY_LEFT_SHIFT) shiftDown = false;
		if (key == GLFW_KEY_SPACE) spaceDown = false;
	}
}
double yScroll = 0.;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	yScroll = yoffset;
}
double mouseX = 0.;
double mouseY = 0.;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	mouseX = xpos;
	mouseY = ypos;
}
bool didMousePress = false;
bool didMouseRelease = false;
bool didRightMousePress = false;
bool didRightMouseRelease = false;
//middle click
bool didMiddlePress = false;
bool didMiddleRelease = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) didMousePress = true;
		if (button == GLFW_MOUSE_BUTTON_RIGHT) didRightMousePress = true;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) didMiddlePress = true;
	} else if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {didMouseRelease = true; didMousePress = false;}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {didRightMouseRelease = true; didRightMousePress = false;}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {didMiddleRelease = true; didMiddlePress = false;}
	}
}
bool isPathLoaded = false;
std::string pathToLoad;
void drop_callback(GLFWwindow* window, int count, const char** paths) {
	if (count != 1) return;
	isPathLoaded = true;
	pathToLoad = paths[0];
}

float vertices[] = {
	0.5f, 0.5f, 0.f, 1.f, 0.f,
	0.5f, -0.5f, 0.f, 1.f, 1.f,
	-0.5f, -0.5f, 0.f, 0.f, 1.f,
	-0.5f, 0.5f, 0.f, 0.f, 0.f
};
unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};

void splitPolygon(std::unique_ptr<Sprite>& sprite, int* selectedPolygon, Vec2f l1, Vec2f l2) {
	std::vector<Vec2f> polygonPoints;
	std::vector<uint8_t> isIntersectionPoints; // 0 normal 1 entry 2 exit
	bool isEntryPoint = true;
	int intersections = 0;
	for (unsigned int i = 0; i < sprite->colliders.at(*selectedPolygon).points.size(); i++) {
		polygonPoints.push_back(sprite->colliders.at(*selectedPolygon).points.at(i));
		isIntersectionPoints.push_back(0);
		Vec2f intersection;
		unsigned int second = (i + 1) % sprite->colliders.at(*selectedPolygon).points.size();
		if (lineIntersection(sprite->colliders.at(*selectedPolygon).points.at(i), sprite->colliders.at(*selectedPolygon).points.at(second), l1, l2, &intersection)) {
			polygonPoints.push_back(intersection);
			isIntersectionPoints.push_back(isEntryPoint ? 1 : 2);
			isEntryPoint = !isEntryPoint;
			intersections++;
		}
	}
	if (intersections != 2) return;
	for (unsigned int j = 0; j < polygonPoints.size(); j++) {
		if (!isIntersectionPoints.at(j)) continue;
		bool isSubPoly = true;
		Collider* poly = &sprite->colliders.at(*selectedPolygon);
		sprite->colliders.push_back({{}, poly->physicsMaterialAssetPath, poly->soundMaterial, poly->isCustomHitSound, poly->customHitSoundName, 128, 128, 128});
		for (unsigned int i = 0; i < polygonPoints.size(); i++) {
			bool oldIsSubPoly = isSubPoly;
			if (isIntersectionPoints.at((i + j) % isIntersectionPoints.size()) != 0 && !oldIsSubPoly) isSubPoly = true;
			if (!isSubPoly) continue;
			if (isIntersectionPoints.at((i + j) % isIntersectionPoints.size()) != 0 && oldIsSubPoly) isSubPoly = false;
			sprite->colliders.at(sprite->colliders.size() - 1).points.push_back(polygonPoints.at((i + j) % isIntersectionPoints.size()));
		}
	}
	sprite->colliders.erase(sprite->colliders.begin() + *selectedPolygon);
	bool doesLastPolygonHaveMorePoints = (sprite->colliders.at(sprite->colliders.size() - 1).points.size() > sprite->colliders.at(sprite->colliders.size() - 2).points.size());
	*selectedPolygon = sprite->colliders.size() - (doesLastPolygonHaveMorePoints ? 1 : 2);
}

void renderLine(Vec2f start, Vec2f end, float width, LineShader& lineShader, glm::mat4& viewMatrix, GLuint VAO, float r, float g, float b, float a) {
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3((start.x + end.x) / 2.f, (start.y + end.y) / 2.f, 0.f));
	model = glm::rotate(model, std::atan2((end.x - start.x), (end.y - start.y)), glm::vec3(0.f, 0.f, -1.f));
	float length = Vec2f::distance(start, end);
	model = glm::scale(model, glm::vec3(width, length, 1.f));

	glm::mat4 transformMatrix = viewMatrix * model;
	lineShader.use();
	glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(transformMatrix));
	glUniform4f(lineShader.colorLoc, r, g, b, a);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
void renderCircle(Vec2f center, float radius, CircleShader& circleShader, glm::mat4& viewMatrix, GLuint VAO, float r, float g, float b, float a) {
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(center.x, center.y, 0.f));
	model = glm::scale(model, glm::vec3(radius * 2.f, radius * 2.f, 1.f));

	glm::mat4 trans = viewMatrix * model;
	circleShader.use();
	glUniformMatrix4fv(circleShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
	glUniform4f(circleShader.colorLoc, r, g, b, a);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main(void) {
	//init glfw ==========
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(frameWidth, frameHeight, "Collider Workshop V3", NULL, NULL);
	if (window == NULL) {
		std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init glad ==========
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "[ERROR] Failed to initialize glad" << std::endl;
		return -1;
	}

	// window hints

	// init imgui ==========
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");


	// set glfw callbacks ==========
	glfwSetKeyCallback        (window, key_callback            );
	glfwSetScrollCallback     (window, scroll_callback         );
	glfwSetCursorPosCallback  (window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback   );
	glfwSetDropCallback       (window, drop_callback           );
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// create cursors ===========
	GLFWcursor* arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	GLFWcursor* handCursor  = glfwCreateStandardCursor(GLFW_HAND_CURSOR );

	// stuff ==========
	glViewport(0, 0, frameWidth, frameHeight);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	SpriteShader spriteShader{"vertex.vsh", "fragment.fsh"};
	LineShader lineShader{"vertex.vsh", "line.fsh"};
	CircleShader circleShader{"vertex.vsh", "circle.fsh"};


	// init render stuff ===========

	//buffers
	GLuint VBO, EBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	float clearColor[3] = {0.22f, 0.18f, 0.2f};
	// wait until file is dropped =====================
	while (!isPathLoaded && !glfwWindowShouldClose(window)) {
		glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Begin("Collider workshop")) {
			ImGui::Text("Drag colliderworkshop.json into window");
		}
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// final stuff
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	std::ifstream jsonFileStream(pathToLoad);
	if (!glfwWindowShouldClose(window) && jsonFileStream.is_open()) {

		// load json file and sprites =========================
		json colliderFileJson = json::parse(jsonFileStream);

		std::filesystem::path projectAssetsPath = colliderFileJson.at("projectAssetsPath").get<std::string>();

		std::vector<std::unique_ptr<Sprite>> sprites;
		sprites.reserve(colliderFileJson.at("sprites").size());
		for (const auto& el : colliderFileJson.at("sprites")) {
			sprites.push_back(std::make_unique<Sprite>(
				el.at("objectPath").get<std::string>(),
				el.at("texturePath").get<std::string>(),
				el.at("pixelsPerUnit").get<float>(),
				Rect(
					el.at("rectMinX").get<float>(),
					el.at("rectMinY").get<float>(),
					el.at("rectMaxX").get<float>(),
					el.at("rectMaxY").get<float>()
				),
				Vec3f(
					el.at("posX").get<float>(),
					el.at("posY").get<float>(),
					el.at("posZ").get<float>()
				),
				Vec3f(
					el.at("rotX").get<float>(),
					el.at("rotY").get<float>(),
					el.at("rotZ").get<float>()
				),
				Vec3f(
					el.at("scaX").get<float>(),
					el.at("scaY").get<float>(),
					el.at("scaZ").get<float>()
				),
				el.at("sortingOrder").get<int>()
			));
		}

		// load images for sprites ==========
		std::unordered_map<std::string, std::shared_ptr<IImageData>> loadedImages;

		int textureLoadIndex = 0;
		for (std::unique_ptr<Sprite>& sprite : sprites) {
			std::string imagePath = (projectAssetsPath / sprite->texturePath).string();

			if (loadedImages.contains(imagePath)) {
				sprite->image = loadedImages.at(imagePath);
				std::cout << "Loaded sprite image " << (textureLoadIndex + 1) << "/" << sprites.size() << " from dictionary: " << sprite->texturePath << std::endl;
			} else {
				std::shared_ptr<ImageData<uint8_t>> imageData = std::make_shared<ImageData<uint8_t>>(imagePath.c_str(), 255);
				imageData->loadTexture();
				std::shared_ptr<IImageData> image = std::static_pointer_cast<IImageData>(imageData);
				sprite->image = image;
				loadedImages.insert_or_assign(imagePath, image);

				std::cout << "Loaded sprite image " << (textureLoadIndex + 1) << "/" << sprites.size() << ": " << sprite->texturePath << ", " << imageData->getWidth() << "x" << imageData->getHeight() << ", (" << sprite->rect.minX << ", " << sprite->rect.minY << ") to (" << sprite->rect.maxX << ", " << sprite->rect.maxY << ")" << std::endl;
			}

			textureLoadIndex++;
		}

		// load physics materials
		std::unordered_map<std::string, PhysicsMaterial2D> physicsMaterials;
		for (const auto& el : colliderFileJson.at("physicsMaterials")) {
			std::string assetPath = el.at("assetPath").get<std::string>();
			physicsMaterials.insert_or_assign(assetPath, PhysicsMaterial2D(el.at("friction").get<float>(), el.at("bounciness").get<float>(), assetPath));
		}

		// load custom hit sounds
		std::vector<std::string> customHitSounds;
		for (const auto& el : colliderFileJson.at("customHitSounds")) {
			customHitSounds.push_back(el.at("name").get<std::string>());
		}

		int currentSprite = 0;
		int selectedPolygon = 0;
		int draggingPoint = -1;
		Vec2f draggingPointScreenOffsetFromMouse;
		Vec2f polygonSplitStart;
		Vec2f polygonSplitEnd;
		bool isDrawingSplitLine = false;

		float worldMouseX = 0.f;
		float worldMouseY = 0.f;

		int frameCount = 0;
		int fps = 0;
		double lastFrameTime = glfwGetTime();

		// camera
		Rect viewBounds{0.f, 0.f, 10.f, 10.f * (static_cast<float>(frameHeight) / static_cast<float>(frameWidth))};
		float moveCameraStartWorldMouseX = 0.f;
		float moveCameraStartWorldMouseY = 0.f;
		bool isMovingCamera = false;

		// settings
		int colliderSmoothSteps = 1;
		float colliderAlphaThreshold = 0.5f;
		float douglasPeuckerEpsilon = 1.5f;

		float pointRadius = 4.f;
		float lineWidth = 1.5f;
		float pointSelectRadius = 16.f;
		float lineSelectDistance = 10.f;

		std::string defaultPhysicsMaterialAssetPath = "";
		SoundMaterial defaultSoundMaterial = SoundMaterial::rock;
		bool defaultIsCustomHitSound = false;
		std::string defaultCustomHitSoundName = "";

		auto worldToScreen = [&](Vec2f v) -> Vec2f {
			return Vec2f(
				mapRange(v.x, viewBounds.minX, viewBounds.maxX, 0.f, static_cast<float>(frameWidth)),
				mapRange(v.y, viewBounds.maxY, viewBounds.minY, 0.f, static_cast<float>(frameHeight))
			);
		};
		auto screenToWorld = [&](Vec2f v) -> Vec2f {
			return Vec2f(
				mapRange(v.x, 0.f, static_cast<float>(frameWidth) , viewBounds.minX, viewBounds.maxX),
				mapRange(v.y, 0.f, static_cast<float>(frameHeight), viewBounds.maxY, viewBounds.minY)
			);
		};

		//llooop
		while (!glfwWindowShouldClose(window)) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGuiIO& io = ImGui::GetIO();

			float aspect = (float)frameWidth / (float)frameHeight;

			// ========== camera movement ===============
			float correctedViewHeight = viewBounds.getWidth() / aspect;
			float currentViewCenterY = (viewBounds.minY + viewBounds.maxY) * 0.5f;
			viewBounds.minY = currentViewCenterY - correctedViewHeight * 0.5f;
			viewBounds.maxY = currentViewCenterY + correctedViewHeight * 0.5f;
			worldMouseX = std::lerp(viewBounds.minX, viewBounds.maxX, mouseX / (float)frameWidth);
			worldMouseY = std::lerp(viewBounds.minY, viewBounds.maxY, 1.f - mouseY / (float)frameHeight);
			if (!io.WantCaptureMouse) {
				// press middle button to start moving
				if (didMiddlePress) {
					isMovingCamera = true;
					moveCameraStartWorldMouseX = worldMouseX;
					moveCameraStartWorldMouseY = worldMouseY;
				}
				// zoom camera
				float zoomScale = std::powf(1.1f, static_cast<float>(-yScroll));
				yScroll = 0.;
				viewBounds.minX = std::lerp(worldMouseX, viewBounds.minX, zoomScale);
				viewBounds.maxX = std::lerp(worldMouseX, viewBounds.maxX, zoomScale);
				viewBounds.minY = std::lerp(worldMouseY, viewBounds.minY, zoomScale);
				viewBounds.maxY = std::lerp(worldMouseY, viewBounds.maxY, zoomScale);
			}
			if (isMovingCamera) {
				// move from worldmousex, to startworldmousex
				viewBounds.minX += moveCameraStartWorldMouseX - worldMouseX;
				viewBounds.maxX += moveCameraStartWorldMouseX - worldMouseX;
				viewBounds.minY += moveCameraStartWorldMouseY - worldMouseY;
				viewBounds.maxY += moveCameraStartWorldMouseY - worldMouseY;
			}
			// releases
			if (didMiddleRelease) isMovingCamera = false;
			didMiddleRelease = false;
			didMiddlePress = false;


			float screenToWorldScale = viewBounds.getWidth() / static_cast<float>(frameWidth);

			// ======= keybinds =========
			if (tabPressed) {
				currentSprite = realMod(currentSprite + (shiftDown ? -1 : 1), sprites.size());
				selectedPolygon = 0;
				tabPressed = false;
			}
			if (rightPressed) {
				selectedPolygon = (selectedPolygon + 1) % sprites[currentSprite]->colliders.size();
				rightPressed = false;
			}
			if (leftPressed) {
				selectedPolygon = realMod(selectedPolygon - 1, sprites[currentSprite]->colliders.size());
				leftPressed = false;
			}


			if (didRightMousePress) {
				polygonSplitStart = {worldMouseX, worldMouseY};
				isDrawingSplitLine = true;
			}
			if (isDrawingSplitLine) polygonSplitEnd = {worldMouseX, worldMouseY};
			if (didRightMouseRelease) {
				isDrawingSplitLine = false;
				splitPolygon(sprites[currentSprite], &selectedPolygon, polygonSplitStart, polygonSplitEnd);
			}
			didRightMousePress = false;
			didRightMouseRelease = false;

			if (gPressed) {
				std::unique_ptr<Sprite>& sprite = sprites[currentSprite];
				selectedPolygon = 0;

				std::vector<std::vector<Vec2f>> paths = Collider::generateCollidersFromImage(sprites[currentSprite]->image, sprites[currentSprite]->rect, colliderAlphaThreshold);

				sprite->colliders.clear();
				sprite->colliders.push_back({{}, defaultPhysicsMaterialAssetPath, defaultSoundMaterial, defaultIsCustomHitSound, defaultCustomHitSoundName, 128, 128, 128});
				Collider* poly = &sprite->colliders.at(0);

				if (paths.size() > 0) {
					poly->points = paths[0];
					Collider::simplifyPath(poly->points, douglasPeuckerEpsilon, colliderSmoothSteps);
					float invPixelsPerUnit = 1.f / sprite->pixelsPerUnit;
					float halfWidth = sprite->getWidth() / 2.f;
					float halfHeight = sprite->getHeight() / 2.f;
					std::for_each(poly->points.begin(), poly->points.end(), [&](Vec2f& point){
						point.x =  point.x * invPixelsPerUnit - halfWidth;
						point.y = -point.y * invPixelsPerUnit + halfHeight;
					});
				}

				// set color of collider
				float weight = 0.f;
				float averageR = 0.f;
				float averageG = 0.f;
				float averageB = 0.f;
				for (int y = sprite->rect.minY; y < sprite->rect.maxY; y += 5) {
					for (int x = sprite->rect.minX; x < sprite->rect.maxX; x += 5) {
						float r, g, b, a;
						sprite->image->getNormalizedPixelRGBA(x, y, &r, &g, &b, &a);
						averageR += r * a;
						averageG += g * a;
						averageB += b * a;
						weight += a;
					}
				}
				if (weight < 0.00001f) {
					weight = 1.f;
				}
				poly->r = static_cast<uint8_t>(averageR / weight * 255.f);
				poly->g = static_cast<uint8_t>(averageG / weight * 255.f);
				poly->b = static_cast<uint8_t>(averageB / weight * 255.f);


				gPressed = false;
			}

			//get closest points and lines for deleting points or moving
			int closestPoint = -1;
			int closestLine = -1;
			if (sprites.size() > 0 && sprites[currentSprite]->colliders.size() > 0) {
				std::unique_ptr<Sprite>& sprite = sprites[currentSprite];
				Collider& collider = sprite->colliders.at(selectedPolygon);

				// get closest point ================
				float closestDistance = 0.f;
				int polygonPoints = collider.points.size();
				for (int j = 0; j < polygonPoints; j++) {
					float x1 = collider.points.at(j).x;
					float y1 = collider.points.at(j).y;

					float dist = sqrt((worldMouseX - x1) * (worldMouseX - x1) + (worldMouseY - y1) * (worldMouseY - y1));
					if (dist < closestDistance || closestPoint == -1) {
						closestDistance = dist;
						closestPoint = j;
					}
				}

				// get closest line =====================
				float closestLineDistance = 0.f;
				for (int j = 0; j < polygonPoints; j++) {
					Vec2f p1 = collider.points.at(j);
					Vec2f p2 = collider.points.at((j + 1) % polygonPoints);

					float dist = distanceFromPointToLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
					float progress = getPointProgressAlongLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
					if ((dist < closestLineDistance || closestLine == -1) && progress >= 0.f && progress <= 1.f) {
						closestLineDistance = dist;
						closestLine = j;
					}
				}

				// start dragging point if clicked ==========
				if (!io.WantCaptureMouse && !spaceDown && closestPoint != -1 && std::max(std::abs(collider.points.at(closestPoint).x - worldMouseX), std::abs(collider.points.at(closestPoint).y - worldMouseY)) < screenToWorldScale * pointSelectRadius) {
					if (didMousePress) {
						if (ctrlKeyDown && polygonPoints > 3) {
							collider.points.erase(collider.points.begin() + closestPoint);

							polygonPoints--;
							closestPoint = -1;
						} else {
							draggingPoint = closestPoint;
							draggingPointScreenOffsetFromMouse = worldToScreen(collider.points.at(draggingPoint)) - Vec2f(mouseX, mouseY);
						}
						didMousePress = false;
					}
				} else closestPoint = -1;
				// add point if press on line
				if (!io.WantCaptureMouse && !spaceDown && !ctrlKeyDown && closestLine != -1 && closestLineDistance < screenToWorldScale * lineSelectDistance && closestPoint == -1 && draggingPoint == -1) {
					if (didMousePress) {
						Vec2f p1 = collider.points.at(closestLine);
						Vec2f p2 = collider.points.at((closestLine + 1) % polygonPoints);
						float progress = getPointProgressAlongLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
						Vec2f pointOnLine = Vec2f::lerp(p1, p2, progress);
						collider.points.insert(collider.points.begin() + closestLine + 1, pointOnLine);
						closestPoint = closestLine + 1;
						draggingPoint = closestLine + 1;
						draggingPointScreenOffsetFromMouse = worldToScreen(collider.points.at(draggingPoint)) - Vec2f(mouseX, mouseY);
						closestLine = -1;
						polygonPoints++;
						didMousePress = false;
					}
				} else closestLine = -1;

				// select polygon
				if (!io.WantCaptureMouse && didMousePress && closestLine == -1 && closestPoint == -1) {
					for (int i = 0; i < (int)sprite->colliders.size(); i++) {
						int howManyIntersections = 0;
						int polygonPoints = sprite->colliders.at(i).points.size();
						for (int j = 0; j < polygonPoints; j++) {
							Vec2f intersection;
							if (lineIntersection(
								sprite->colliders.at(i).points.at(j),
												 sprite->colliders.at(i).points.at((j + 1) % polygonPoints),
												 {worldMouseX, worldMouseY},
							{worldMouseX + 100.f, worldMouseY},
							&intersection
							)) {
								howManyIntersections++;
							}
						}
						if (howManyIntersections % 2 == 1) {
							selectedPolygon = i;
							didMousePress = false;
							break;
						}
					}
				}
			}

			if (!io.WantCaptureMouse) {
				if (closestPoint != -1) {
					glfwSetCursor(window, handCursor);
				} else {
					glfwSetCursor(window, arrowCursor);
				}
			}


			if (didMouseRelease) {
				draggingPoint = -1;
			}
			didMouseRelease = false;

			// drag point =======
			if (selectedPolygon != -1 && draggingPoint != -1) {
				sprites[currentSprite]->colliders.at(selectedPolygon).points.at(draggingPoint).x = worldMouseX + draggingPointScreenOffsetFromMouse.x * screenToWorldScale;
				sprites[currentSprite]->colliders.at(selectedPolygon).points.at(draggingPoint).y = worldMouseY - draggingPointScreenOffsetFromMouse.y * screenToWorldScale;
			}

			// ====== save json ==============
			if (sPressed && ctrlKeyDown) {
				json j;
				j["projectAssetsPath"] = projectAssetsPath.string();
				j["sprites"] = json::array();
				for (const std::unique_ptr<Sprite>& sprite : sprites) {
					json spriteObject;
					spriteObject["objectPath"] = sprite->objectPath;
					spriteObject["texturePath"] = sprite->texturePath;
					spriteObject["rectMinX"] = sprite->rect.minX;
					spriteObject["rectMinY"] = sprite->rect.minY;
					spriteObject["rectMaxX"] = sprite->rect.maxX;
					spriteObject["rectMaxY"] = sprite->rect.maxY;
					spriteObject["posX"] = sprite->position.x;
					spriteObject["posY"] = sprite->position.y;
					spriteObject["posZ"] = sprite->position.z;
					spriteObject["rotX"] = sprite->rotation.x;
					spriteObject["rotY"] = sprite->rotation.y;
					spriteObject["rotZ"] = sprite->rotation.z;
					spriteObject["scaX"] = sprite->scale.x;
					spriteObject["scaY"] = sprite->scale.y;
					spriteObject["scaZ"] = sprite->scale.z;
					spriteObject["pixelsPerUnit"] = sprite->pixelsPerUnit;
					spriteObject["sortingOrder"] = sprite->sortingOrder;
					spriteObject["colliders"] = json::array();
					for (const Collider& collider : sprite->colliders) {
						json colliderObject;
						colliderObject["xPositions"] = json::array();
						colliderObject["yPositions"] = json::array();
						for (const Vec2f& point : collider.points) {
							colliderObject["xPositions"].push_back(point.x);
							colliderObject["yPositions"].push_back(point.y);
						}
						colliderObject["physicsMaterialAssetPath"] = collider.physicsMaterialAssetPath;
						colliderObject["soundMaterial"] = (int)collider.soundMaterial;
						colliderObject["isCustomHitSound"] = collider.isCustomHitSound;
						colliderObject["customHitSoundName"] = collider.customHitSoundName;
						colliderObject["r"] = collider.r;
						colliderObject["g"] = collider.g;
						colliderObject["b"] = collider.b;

						spriteObject["colliders"].push_back(colliderObject);
					}

					j["sprites"].push_back(spriteObject);
				}
				std::string serializedJson = j.dump();

				std::ofstream jsonFile;
				jsonFile.open(projectAssetsPath / "colliderworkshop_unity.json");
				jsonFile << serializedJson;
				jsonFile.close();
			}
			sPressed = false;

			glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			glm::mat4 viewMatrix = glm::ortho(viewBounds.minX, viewBounds.maxX, viewBounds.minY, viewBounds.maxY, -1.f, 1.f);

			// RENDER SPRITE
			if (sprites.size() > 0) {
				const std::unique_ptr<Sprite>& sprite = sprites[currentSprite];

				glm::mat4 model = glm::mat4(1.f);
				model = glm::scale(model, glm::vec3(sprite->getWidth(), sprite->getHeight(), 1.f));
				glm::mat4 trans = viewMatrix * model;
				spriteShader.use();
				sprite->image->bindTexture();
				glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(
					spriteShader.boundsLoc,
					sprite->rect.minX / static_cast<float>(sprite->image->getWidth()),
					sprite->rect.minY / static_cast<float>(sprite->image->getHeight()),
					sprite->rect.maxX / static_cast<float>(sprite->image->getWidth()),
					sprite->rect.maxY / static_cast<float>(sprite->image->getHeight())
				);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


				// DRAW LINES
				for (unsigned int i = 0; i < sprite->colliders.size(); i++) {
					Collider* poly = &sprite->colliders.at(i);
					int polygonPoints = poly->points.size();
					for (int j = 0; j < polygonPoints; j++) {
						Vec2f p1 = poly->points.at(j);
						Vec2f p2 = poly->points.at((j + 1) % polygonPoints);

						bool isLineRed = ctrlKeyDown && (j == closestPoint || ((j + 1) % polygonPoints) == closestPoint);
						bool isLineClosest = j == closestLine;
						renderLine(
							p1, p2,
							isLineClosest ? screenToWorldScale * lineWidth * 1.5f : screenToWorldScale * lineWidth,
							lineShader, viewMatrix, VAO,
							1.f,
							isLineRed ? 0.f : 1.f,
							isLineClosest ? 1.f : 0.f,
							i == selectedPolygon ? 1.f : 0.2f
						);

						// draw points
						bool isPointRed = ctrlKeyDown && j == closestPoint;
						bool isPointHovered = j == draggingPoint || (j == closestPoint && draggingPoint == -1);
						renderCircle(
							p1,
							isPointHovered ? screenToWorldScale * pointRadius * 1.5f : screenToWorldScale * pointRadius,
							circleShader, viewMatrix, VAO,
							1.f,
							isPointRed ? 0.f : 1.f,
							isPointHovered ? 1.f : 0.f,
							i == selectedPolygon ? 0.5f : 0.1f
						);
					}
				}
			}

			// split line
			if (isDrawingSplitLine) {
				renderLine(polygonSplitStart, polygonSplitEnd, screenToWorldScale * lineWidth, lineShader, viewMatrix, VAO, 0.1f, 0.5f, 1.f, 1.f);
				renderCircle(polygonSplitStart, screenToWorldScale * pointRadius, circleShader, viewMatrix, VAO, 0.1f, 0.5f, 1.f, 1.f);
				renderCircle(polygonSplitEnd  , screenToWorldScale * pointRadius, circleShader, viewMatrix, VAO, 0.1f, 0.5f, 1.f, 1.f);
			}

			// point on line
			if (closestLine != -1) {
				Vec2f p1 = sprites[currentSprite]->colliders.at(selectedPolygon).points.at(closestLine);
				Vec2f p2 = sprites[currentSprite]->colliders.at(selectedPolygon).points.at((closestLine + 1) % sprites[currentSprite]->colliders.at(selectedPolygon).points.size());
				float progress = getPointProgressAlongLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
				Vec2f pointOnLine = Vec2f::lerp(p1, p2, progress);

				renderCircle(pointOnLine, screenToWorldScale * pointRadius, circleShader, viewMatrix, VAO, 1.f, 1.f, 0.5f, 1.f);
			}


			/*int howManyPoints = sprites[currentSprite]->colliders.at(selectedPolygon).points.size();
			std::string howManyPointsString = "Points: " + std::to_string(howManyPoints);

			std::string currentPolygonString = "Polygon " + std::to_string(selectedPolygon + 1) + "/" + std::to_string(sprites[currentSprite]->colliders.size());
			std::string currentSpriteString = "Sprite " + std::to_string(currentSprite + 1) + "/" + std::to_string(sprites.size());
			if (closestPoint != -1) {
				Vec2f closest = sprites[currentSprite]->colliders.at(selectedPolygon).points.at(closestPoint);
				std::string currentPointString = "Point " + std::to_string(closestPoint) + "(" + std::to_string(closest.x) + ", " + std::to_string(closest.y) + ")";
				renderText(glyphShader, currentPointString, 0.f, 0.75f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			}

			renderText(glyphShader, currentSpriteString, 0.f, 0.9f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, currentPolygonString, 0.f, 0.85f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, howManyPointsString, 0.f, 0.8f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);

			// controls
			renderText(glyphShader, "G - Generate collider", -aspect + 0.05f, 0.8f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, "RMB - Split", -aspect + 0.05f, 0.75f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, "<> - Switch polygon", -aspect + 0.05f, 0.7f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);*/

			// render ImGui
			if (ImGui::Begin("Collider workshop", nullptr, ImGuiWindowFlags_NoNav)) {
				if (ImGui::CollapsingHeader("UI settings")) {
					ImGui::ColorEdit3("Background color", clearColor);
					ImGui::SliderFloat("Line width", &lineWidth, 0.f, 5.f);
					ImGui::SliderFloat("Point radius", &pointRadius, 0.f, 10.f);
					ImGui::SliderFloat("Line select distance", &lineSelectDistance, 0.f, 30.f);
					ImGui::SliderFloat("Point select radius", &pointSelectRadius, 0.f, 30.f);
					static bool linear = true;
					if (ImGui::Checkbox("Texture linear filtering", &linear)) {
						for (auto& it : loadedImages) {
							it.second->setTextureFilter(linear);
						}
					}
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Collider generation settings")) {
					ImGui::SliderFloat("Alpha threshold", &colliderAlphaThreshold, 0.f, 1.f);
					ImGui::SliderInt("Smooth steps", &colliderSmoothSteps, 0, 5);
					ImGui::SliderFloat("Douglas-Peucker epsilon", &douglasPeuckerEpsilon, 0.f, 15.f);
					ImGui::Separator();
					ImGui::Text("Default settings:");

					std::string physicsMaterialName = "???";
					if (physicsMaterials.count(defaultPhysicsMaterialAssetPath)) {
						physicsMaterialName = physicsMaterials.at(defaultPhysicsMaterialAssetPath).displayName;
					}
					if (ImGui::BeginCombo("Physics material##default", physicsMaterialName.c_str())) {
						int n = 0;
						for (auto& it : physicsMaterials) {
							bool is_selected = it.first == defaultPhysicsMaterialAssetPath;
							if (ImGui::Selectable((it.second.displayName + "##" + std::to_string(n)).c_str(), is_selected)) {
								defaultPhysicsMaterialAssetPath = it.first;
							}
							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}
							n++;
						}
						ImGui::EndCombo();
					}

					// hit sound select
					ImGui::Checkbox("Custom hit sound##default", &defaultIsCustomHitSound);
					if (defaultIsCustomHitSound) {
						if (ImGui::BeginCombo("Custom hit sound##defaultcombo", defaultCustomHitSoundName.c_str())) {
							int n = 0;
							for (const std::string& customHitSound : customHitSounds) {
								bool is_selected = customHitSound == defaultCustomHitSoundName;
								if (ImGui::Selectable((customHitSound + "##" + std::to_string(n)).c_str(), is_selected)) {
									defaultCustomHitSoundName = customHitSound;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
								n++;
							}
							ImGui::EndCombo();
						}
					} else {
						if (ImGui::BeginCombo("Hit sound##defaultcombo", soundMaterialNames[(int)defaultSoundMaterial].c_str())) {
							for (int n = 0; n < 10; n++) {
								bool is_selected = (int)defaultSoundMaterial == n;
								if (ImGui::Selectable(soundMaterialNames[n].c_str(), is_selected)) {
									defaultSoundMaterial = (SoundMaterial)n;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}
					}
					ImGui::Separator();
				}
				ImGui::Text("Sprite: %d/%d", currentSprite + 1, (int)sprites.size());
				if (sprites.size() > 0) {
					std::unique_ptr<Sprite>& sprite = sprites[currentSprite];

					ImGui::Text("Sprite object: %s", sprite->objectPath.c_str());

					ImGui::Separator();
					ImGui::Text("Collider %d/%d", selectedPolygon + 1, (int)sprite->colliders.size());
					if (sprite->colliders.size() > 0) {
						Collider& collider = sprite->colliders[selectedPolygon];
						ImGui::Text("Points: %d", (int)collider.points.size());

						ImGui::Separator();
						// physics material select
						int n = 0;
						for (auto& it : physicsMaterials) {
							bool is_selected = it.first == collider.physicsMaterialAssetPath;
							if (ImGui::Selectable((it.second.displayName + "##" + std::to_string(n)).c_str(), is_selected)) {
								collider.physicsMaterialAssetPath = it.first;
							}
							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}
							n++;
						}

						ImGui::Separator();
						// hit sound select
						ImGui::Checkbox("Custom hit sound", &collider.isCustomHitSound);
						if (collider.isCustomHitSound) {
							int n = 0;
							for (const std::string& customHitSound : customHitSounds) {
								bool is_selected = customHitSound == collider.customHitSoundName;
								if (ImGui::Selectable((customHitSound + "##" + std::to_string(n)).c_str(), is_selected)) {
									collider.customHitSoundName = customHitSound;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
								n++;
							}
						} else {
							for (int n = 0; n < 10; n++) {
								bool is_selected = (int)collider.soundMaterial == n;
								if (ImGui::Selectable(soundMaterialNames[n].c_str(), is_selected)) {
									collider.soundMaterial = (SoundMaterial)n;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
						}
						float hitColor[3] = {static_cast<float>(collider.r) / 255.f, static_cast<float>(collider.g) / 255.f, static_cast<float>(collider.b) / 255.f};
						ImGui::ColorEdit3("Hit color", hitColor);
						collider.r = static_cast<uint8_t>(hitColor[0] * 255.f);
						collider.g = static_cast<uint8_t>(hitColor[1] * 255.f);
						collider.b = static_cast<uint8_t>(hitColor[2] * 255.f);
					}
				}
			}
			ImGui::End();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// deselect imgui window
			if ((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(2)) && !ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
				ImGui::SetWindowFocus(nullptr);      // un-focuses all windows
			}


			// final stuff
			glfwSwapBuffers(window);
			glfwPollEvents();

			frameCount++;
			double frameTime = glfwGetTime();
			if (frameTime > lastFrameTime + 1.) {
				lastFrameTime = frameTime;
				fps = frameCount;
				frameCount = 0;
			}
		}
	} // after dragged file into window

	// destroy cursors
	glfwDestroyCursor(arrowCursor);
	glfwDestroyCursor(handCursor);

	glfwTerminate();

	return 0;
}
