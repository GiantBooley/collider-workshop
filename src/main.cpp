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
#include <ft2build.h>
#include FT_FREETYPE_H

#include "math.hpp"
#include "IImageData.hpp"
#include "ImageData.hpp"


int realMod(int a, int b) {
	if (a >= 0) return a % b; else return (b >= 0 ? b : -b) - 1 + (a + 1) % b;
}
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

namespace SoundMaterial {
	enum SoundMaterial {
		rock,
		wood,
		metal,
		plastic,
		furniture,
		snow,
		cardboard,
		none,
		snake,
		solidmetal
	};
}
std::string hitSoundNames[10] = {
	"Rock",
	"Wood",
	"Metal",
	"Plastic",
	"Furniture",
	"Snow",
	"Cardboard",
	"None",
	"Snake",
	"SolidMetal"
};
class Polygon {
public:
	std::vector<Vec2f> points;
	SoundMaterial::SoundMaterial hitSound;
	bool isCustomHitSound;
	std::string customHitSoundName;
	std::string frictionAssetPath;
	unsigned char r, g, b;

	Polygon(
		std::vector<Vec2f> points2,
		 SoundMaterial::SoundMaterial hitSound2,
		 bool isCustomHitSound2,
		 std::string customHitSoundName2,
		 std::string frictionAssetPath2
	) : points(points2), hitSound(hitSound2), isCustomHitSound(isCustomHitSound2), customHitSoundName(customHitSoundName2), frictionAssetPath(frictionAssetPath2), r(128), g(128), b(128) {}
};

class Sprite {
public:
	std::string objectPath;
	std::filesystem::path texturePath;
	std::shared_ptr<IImageData> image;
	std::vector<Polygon> polygons;
	float pixelsPerUnit;
	float minX, minY, maxX, maxY;
	Sprite(
		std::string objectPath2,
		std::string texturePath2,
		float pixelsPerUnit2,
		float minX2,
		float minY2,
		float maxX2,
		float maxY2
	) : objectPath(objectPath2), texturePath(texturePath2), pixelsPerUnit(pixelsPerUnit2), minX(minX2), minY(minY2), maxX(maxX2), maxY(maxY2), polygons() {

		image = std::static_pointer_cast<IImageData>(std::make_shared<ImageData<uint8_t>>(texturePath.c_str(), 255));
		polygons.push_back({{{-width / 2.f,-height / 2.f},{-width / 2.f,height / 2.f},{width / 2.f,height / 2.f},{width / 2.f,-height / 2.f}}, false, SoundMaterial::rock, "", 0});
	}

	float getWidth() const {
		return (maxX - minX) / pixelsPerUnit;
	}
};
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
	yScroll = yoffset;
}
double mouseX = 0.;
double mouseY = 0.;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
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
	0.5f, 0.5f, 0.f, 1.f, 1.f,
	0.5f, -0.5f, 0.f, 1.f, 0.f,
	-0.5f, -0.5f, 0.f, 0.f, 0.f,
	-0.5f, 0.5f, 0.f, 0.f, 1.f
};
unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};
struct AABB {
	float l, r, b, t;
};
AABB getViewBounds(float zoom, float cameraX, float cameraY) {
	float aspect = (float)frameWidth / (float)frameHeight;
	return {-zoom / 2.f * aspect - cameraX, zoom / 2.f * aspect - cameraX, -zoom / 2.f - cameraY, zoom / 2.f - cameraY};
}
void generateCollider(Sprite* sprite) { // 0 1 2 3 clockwise
	const unsigned char alphaThreshold = 127;
	int w = sprite->tex->width;
	int h = sprite->tex->height;
	bool exit = false;
	for (int y = sprite->minY; y < sprite->maxY; y++) {
		for (int x = sprite->minX; x < sprite->maxX; x++) {
			if (sprite->tex->data[(y * w + x) * 4 + 3] > alphaThreshold) {
				exit = true;
				x--;
				int startX = x;
				int startY = y;
				int side = 1; // right
				bool matrix[9]; // 0,0: bottom left
				for (int i = 0; i < 100000; i++) {
					if (i != 0 && x == startX && y == startY && side == 1) break;
					for (int my = 0; my < 3; my++) {
						for (int mx = 0; mx < 3; mx++) {
							int ecks = x + mx - 1;
							int why = y + my - 1;
							matrix[my * 3 + mx] = (ecks >= 0 && ecks < w && why >= 0 && why < h) && (sprite->tex->data[(why * w + ecks) * 4 + 3] > alphaThreshold);
						}
					}
					float pointX = 0.f;
					float pointY = 0.f;
					bool doAddPoint = false;
					if (side == 0) { // top: top left, middle left
						pointX = (float)x;
						pointY = (float)y + 1.f;
						if (!matrix[2*3+0] && !matrix[1*3+0]) { // top left empty middle left empty
							x--;
							y++;
							side = 1;
							doAddPoint = true;
						}
						if (matrix[2*3+0] && !matrix[1*3+0]) { // top left full middle left empty
							x--;
						}
						if (matrix[1*3+0]) { // middle left full
							side = 3;
							doAddPoint = true;
						}
					}
					else if (side == 1) { // right: top right, top middle
						pointX = (float)x + 1.f;
						pointY = (float)y + 1.f;
						if (!matrix[2*3+2] && !matrix[2*3+1]) { // top right empty top middle empty
							x++;
							y++;
							side = 2;
							doAddPoint = true;
						}
						if (matrix[2*3+2] && !matrix[2*3+1]) { // top right full top middle empty
							y++;
						}
						if (matrix[2*3+1]) { // top middle full
							side = 0;
							doAddPoint = true;
						}
					}
					else if (side == 2) { // bottom: bottom right, middle right
						pointX = (float)x + 1.f;
						pointY = (float)y;
						if (!matrix[0*3+2] && !matrix[1*3+2]) { // bottom right empty middle right empty
							x++;
							y--;
							side = 3;
							doAddPoint = true;
						}
						if (matrix[0*3+2] && !matrix[1*3+2]) { // bottom right full middle right empty
							x++;
						}
						if (matrix[1*3+2]) { // middle right full
							side = 1;
							doAddPoint = true;
						}
					}
					else if (side == 3) { // left: bottom left, bottom middle
						pointX = (float)x;
						pointY = (float)y;
						if (!matrix[0*3+0] && !matrix[0*3+1]) { // bottom left empty bottom middle empty
							x--;
							y--;
							side = 0;
							doAddPoint = true;
						}
						if (matrix[0*3+0] && !matrix[0*3+1]) { // bottom left full bottom middle empty
							y--;
						}
						if (matrix[0*3+1]) { // bottom middle full
							side = 2;
							doAddPoint = true;
						}
					}
					if (doAddPoint) {
						sprite->polygons[0].points.push_back({
							(pointX - sprite->minX) / 100.f - sprite->width / 2.f,
							(pointY - sprite->minY) / 100.f - sprite->height / 2.f
						});
					}
				}
			}
			if (exit) break;
		}
		if (exit) break;
	}
}

void splitPolygon(Sprite* sprite, int* selectedPolygon, Vec2f l1, Vec2f l2) {
	std::vector<Vec2f> polygonPoints;
	std::vector<uint8_t> isIntersectionPoints; // 0 normal 1 entry 2 exit
	bool isEntryPoint = true;
	int intersections = 0;
	for (unsigned int i = 0; i < sprite->polygons.at(*selectedPolygon).points.size(); i++) {
		polygonPoints.push_back(sprite->polygons.at(*selectedPolygon).points.at(i));
		isIntersectionPoints.push_back(0);
		Vec2f intersection;
		unsigned int second = (i + 1) % sprite->polygons.at(*selectedPolygon).points.size();
		if (lineIntersection(sprite->polygons.at(*selectedPolygon).points.at(i), sprite->polygons.at(*selectedPolygon).points.at(second), l1, l2, &intersection)) {
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
		Polygon* poly = &sprite->polygons.at(*selectedPolygon);
		sprite->polygons.push_back({{}, poly->hitSound, poly->isCustomHitSound, poly->customHitSoundName, poly->frictionAssetPath});
		for (unsigned int i = 0; i < polygonPoints.size(); i++) {
			bool oldIsSubPoly = isSubPoly;
			if (isIntersectionPoints.at((i + j) % isIntersectionPoints.size()) != 0 && !oldIsSubPoly) isSubPoly = true;
			if (!isSubPoly) continue;
			if (isIntersectionPoints.at((i + j) % isIntersectionPoints.size()) != 0 && oldIsSubPoly) isSubPoly = false;
			sprite->polygons.at(sprite->polygons.size() - 1).points.push_back(polygonPoints.at((i + j) % isIntersectionPoints.size()));
		}
	}
	sprite->polygons.erase(sprite->polygons.begin() + *selectedPolygon);
	bool doesLastPolygonHaveMorePoints = (sprite->polygons.at(sprite->polygons.size() - 1).points.size() > sprite->polygons.at(sprite->polygons.size() - 2).points.size());
	*selectedPolygon = sprite->polygons.size() - (doesLastPolygonHaveMorePoints ? 1 : 2);
}

int main(void) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//init glfw
	GLFWwindow* window = glfwCreateWindow(frameWidth, frameHeight, "Collider Workshop V2", NULL, NULL);
	if (window == NULL) {
		std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "[ERROR] Failed to initialize glad" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetDropCallback(window, drop_callback);

	//stuff
	glViewport(0, 0, frameWidth, frameHeight);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	SpriteShader spriteShader{"vertex.vsh", "fragment.fsh"};
	LineShader lineShader{"vertex.vsh", "line.fsh"};
	std::vector<Sprite*> sprites = {};

	
	std::vector<std::string> customHitSoundNames = {};
	std::vector<std::string> frictionPaths = {};
	std::vector<std::string> frictionNames = {};

	// init render stuff ===========

	//buffers
	unsigned int VBO, EBO, VAO;

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


	// wait until file is dropped =====================
	while (!isPathLoaded && !glfwWindowShouldClose(window)) {
		glClearColor(0.4f, 0.4f, 0.4f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		float aspect = (float)frameWidth / (float)frameHeight;
		glm::mat4 screenSpaceProj = glm::ortho(-aspect, aspect, -1.f, 1.f);
		// render text


		// final stuff
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	std::string colliderFileText;
	std::string diasd = pathToLoad;
	if (!glfwWindowShouldClose(window) && readFileText(pathToLoad, &colliderFileText)) {
		std::istringstream iss(colliderFileText);

		int linemode = 0;
		int howmanycustomnamesleft;
		int howmanyfrictionpathsleft;
		std::string objectName, spriteFileName;
		// 0: howmanycustomnames, 1: custom names, 2: howmanyfrictionpaths, 3: friction names, 4: object name, 5: sprite path, 6: sprite bounds
		for (std::string line; std::getline(iss, line);) {
			switch (linemode) {
				case 0:{ // howmanycustomnames
					howmanycustomnamesleft = stoi(line);
					linemode = howmanycustomnamesleft == 0 ? 2 : 1; // if no custom names then go to friction
					break;
				}
				case 1:{ // custom hit sound names
					customHitSoundNames.push_back(line);
					howmanycustomnamesleft--;
					if (howmanycustomnamesleft == 0) linemode = 2;
					break;
				}
				case 2:{ // howmanyfrictionpaths
					howmanyfrictionpathsleft = stoi(line);
					linemode = (howmanyfrictionpathsleft == 0) ? 4 : 3; // if no friction paths then go to objects
					break;
				}
				case 3:{ // friction paths
					frictionPaths.push_back(line);
					frictionNames.push_back(filePathToName(line));
					howmanyfrictionpathsleft--;
					if (howmanyfrictionpathsleft == 0) linemode = 4;
					break;
				}
				case 4:{ // object name
					objectName = line;
					linemode = 5;
					break;
				}
				case 5:{ // sprite file name
					spriteFileName = line;
					linemode = 6;
					break;
				}
				case 6:{ // sprite bounds
					std::stringstream boundsStringStream{line};
					std::string boundNumberString;
					vector<float> boundNumbers = {};
					while (getline(boundsStringStream, boundNumberString, ',')) {
						boundNumbers.push_back(stof(boundNumberString));
					}
					Sprite* sprite = new Sprite(objectName, spriteFileName, boundNumbers.at(0), boundNumbers.at(1), boundNumbers.at(2), boundNumbers.at(3));
					sprites.push_back(sprite);
					linemode = 4;
					break;
				}
			}
		}

		int currentSprite = 0;



		float worldMouseX = 0.f;
		float worldMouseY = 0.f;


		int selectedPolygon = 0;
		int draggingPoint = -1;

		int frameCount = 0;
		int fps = 0;
		double lastFrameTime = glfwGetTime();
		float zoom = 10.f;
		float cameraX = 0.f;
		float cameraY = 0.f;
		float moveCameraStartWorldMouseX = 0.f;
		float moveCameraStartWorldMouseY = 0.f;
		bool isMovingCamera = false;

		Vec2f polygonSplitStart;
		Vec2f polygonSplitEnd;
		bool isDrawingSplitLine = false;


		//llooop
		while (!glfwWindowShouldClose(window)) {
			float aspect = (float)frameWidth / (float)frameHeight;

			zoom /= std::pow(1.1, yScroll);
			zoom = std::min(std::max(zoom, 0.1f), 300.f);

			yScroll = 0.;

			AABB bounds = getViewBounds(zoom, cameraX, cameraY);
			worldMouseX = lerp(bounds.l, bounds.r, mouseX / (float)frameWidth);
			worldMouseY = lerp(bounds.b, bounds.t, 1.f - mouseY / (float)frameHeight);
			if (didMiddlePress) {
				isMovingCamera = true;
				moveCameraStartWorldMouseX = worldMouseX;
				moveCameraStartWorldMouseY = worldMouseY;
			}
			if (didMiddleRelease) isMovingCamera = false;
			didMiddlePress = false;
			didMiddleRelease = false;

			if (isMovingCamera) {
				cameraX += worldMouseX - moveCameraStartWorldMouseX;
				cameraY += worldMouseY - moveCameraStartWorldMouseY;
			}

			float scale = zoom / 10.f;

			if (tabPressed) {
				currentSprite = realMod(currentSprite + (shiftDown ? -1 : 1), sprites.size());
				selectedPolygon = 0;
				tabPressed = false;
			}

			if (rightPressed) {
				selectedPolygon = (selectedPolygon + 1) % sprites[currentSprite]->polygons.size();
				rightPressed = false;
			}
			if (leftPressed) {
				selectedPolygon = realMod(selectedPolygon - 1, sprites[currentSprite]->polygons.size());
				leftPressed = false;
			}
			float mouseEcks = (mouseX / (float)frameWidth * 2.f - 1.f) * aspect;
			float mouseWhy = (1.f - mouseY / (float)frameHeight) * 2.f - 1.f;
			if (didMousePress) { // hit sound
				for (int i = 0; i < 10; i++) {
					if (mouseEcks > (aspect - 0.3f) && mouseWhy > -0.9f + (float)(9 - i) * 0.1f && mouseWhy < -0.9f + (float)((9 - i) + 1) * 0.1f) {
						sprites[currentSprite]->polygons.at(selectedPolygon).isCustomHitSound = false;
						sprites[currentSprite]->polygons.at(selectedPolygon).hitSound = (SoundMaterial::SoundMaterial)i;
						didMousePress = false;
						break;
					}
				}
				for (int i = 0; i < (int)frictionPaths.size(); i++) {
					if (mouseEcks > (aspect - 0.6f) && mouseEcks < (aspect - 0.3f) && mouseWhy > -0.8f + (float)i * 0.1f && mouseWhy < -0.8f + (float)(i + 1) * 0.1f) {
						sprites[currentSprite]->polygons.at(selectedPolygon).frictionIndex = i;
						didMousePress = false;
						break;
					}
				}
				for (int i = 0; i < (int)customHitSoundNames.size(); i++) {
					if (mouseEcks < -aspect + 0.3f && mouseWhy > -0.9f + (float)i * 0.1f && mouseWhy < -0.9f + (float)(i + 1) * 0.1f) {
						sprites[currentSprite]->polygons.at(selectedPolygon).isCustomHitSound = true;
						sprites[currentSprite]->polygons.at(selectedPolygon).customHitSound = customHitSoundNames.at(i);
						didMousePress = false;
						break;
					}
				}
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
				selectedPolygon = 0;

				sprites[currentSprite]->polygons.clear();
				sprites[currentSprite]->polygons.push_back({{}, false, SoundMaterial::cardboard, "", 2});
				generateCollider(sprites[currentSprite]);
				Polygon* poly = &sprites[currentSprite]->polygons.at(0);
				int howManyDecimationSteps = poly->points.size();
				float minDistance = 0.02f; // old: 0.02f
				for (int iteration = 0; iteration < howManyDecimationSteps; iteration++) {
					poly = &sprites[currentSprite]->polygons.at(0);
					// find lowest distance
					float lowestDistance = 0.f;
					float lowestIndex = -1;
					for (int i = poly->points.size() - 1; i >= 0; i--) {
						Point p1 = poly->points.at(i);
						Point p2 = poly->points.at(realMod(i + 1, poly->points.size()));
						float dist = distance(p1, p2);
						if ((lowestIndex == -1 || dist < lowestDistance) && dist < minDistance) {
							lowestIndex = i;
							lowestDistance = dist;
						}
					}
					if (lowestIndex == -1) break;
					Point* lowest = &poly->points.at(lowestIndex);
					Point* lowestplusone = &poly->points.at(realMod(lowestIndex + 1, poly->points.size()));
					lowest->x = (lowest->x + lowestplusone->x) * 0.5f;
					lowest->y = (lowest->y + lowestplusone->y) * 0.5f;
					poly->points.erase(poly->points.begin() + realMod(lowestIndex + 1, poly->points.size()));
				}


				gPressed = false;
			}

			int closestPoint = -1;
			//get closest point for ctrl or move
			float closestDistance = 0.f;
			int polygonPoints = sprites[currentSprite]->polygons.at(selectedPolygon).points.size();
			for (int j = 0; j < polygonPoints; j++) {
				float x1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(j).x;
				float y1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(j).y;

				float dist = sqrt((worldMouseX - x1) * (worldMouseX - x1) + (worldMouseY - y1) * (worldMouseY - y1));
				if (dist < closestDistance || closestPoint == -1) {
					closestDistance = dist;
					closestPoint = j;
				}
			}
			if (!spaceDown && closestPoint != -1 && std::max(std::abs(sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint).x - worldMouseX), std::abs(sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint).y - worldMouseY)) < 0.2f * scale) {
				if (didMousePress) {
					if (ctrlKeyDown && polygonPoints > 3) {
						sprites[currentSprite]->polygons.at(selectedPolygon).points.erase(sprites[currentSprite]->polygons.at(selectedPolygon).points.begin() + closestPoint);

						polygonPoints--;
						closestPoint = -1;
					} else {
						draggingPoint = closestPoint;
					}
					didMousePress = false;
				}
			} else closestPoint = -1;

			//get closest line
			int closestLine = -1;
			float closestLineDistance = 0.f;
			for (int j = 0; j < polygonPoints; j++) {
				Vec2f p1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(j);
				Vec2f p2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((j + 1) % polygonPoints);

				float dist = distanceFromPointToLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
				float progress = getPointProgressAlongLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
				if ((dist < closestLineDistance || closestLine == -1) && progress >= 0.f && progress <= 1.f) {
					closestLineDistance = dist;
					closestLine = j;
				}
			}
			// add point if press on line
			if (!spaceDown && !ctrlKeyDown && closestLine != -1 && closestLineDistance < 0.2f * scale && closestPoint == -1 && draggingPoint == -1) {
				if (didMousePress) {
					Vec2f p1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine);
					Vec2f p2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints).;
					float progress = getPointProgressAlongLine(Vec2f::(worldMouseX, worldMouseY), p1, p2);
					Vec2f pointOnLine = Vec2f::lerp(p1, p2, progress);
					sprites[currentSprite]->polygons.at(selectedPolygon).points.insert(sprites[currentSprite]->polygons.at(selectedPolygon).points.begin() + closestLine + 1, pointOnLine);
					closestPoint = closestLine + 1;
					draggingPoint = closestLine + 1;
					closestLine = -1;
					polygonPoints++;
					didMousePress = false;
				}
			} else closestLine = -1;

			// select polygon
			if (didMousePress && closestLine == -1 && closestPoint == -1) {
				for (int i = 0; i < (int)sprites[currentSprite]->polygons.size(); i++) {
					int howManyIntersections = 0;
					int polygonPoints = sprites[currentSprite]->polygons.at(i).points.size();
					for (int j = 0; j < polygonPoints; j++) {
						Vec2f intersection;
						if (lineIntersection(
							sprites[currentSprite]->polygons.at(i).points.at(j),
							sprites[currentSprite]->polygons.at(i).points.at((j + 1) % polygonPoints),
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


			if (didMouseRelease) {
				draggingPoint = -1;
			}
			didMouseRelease = false;

			if (selectedPolygon != -1 && draggingPoint != -1) {
				sprites[currentSprite]->polygons.at(selectedPolygon).points.at(draggingPoint).x = worldMouseX;
				sprites[currentSprite]->polygons.at(selectedPolygon).points.at(draggingPoint).y = worldMouseY;
			}

			if (sPressed && ctrlKeyDown) {
			}
			sPressed = false;

			glClearColor(0.4f, 0.4f, 0.4f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);


			// RENDER SPRITE
			glm::mat4 model = glm::mat4(1.f);
			model = glm::scale(model, glm::vec3(sprites[currentSprite]->width, sprites[currentSprite]->height, 1.f));
			glm::mat4 proj = glm::ortho(bounds.l, bounds.r, bounds.b, bounds.t, -1.f, 1.f);
			glm::mat4 trans = proj * model;
			spriteShader.use();
			sprites[currentSprite]->tex->use();
			glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
			glUniform4f(spriteShader.boundsLoc, sprites[currentSprite]->minX, sprites[currentSprite]->minY, sprites[currentSprite]->maxX, sprites[currentSprite]->maxY);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


			// DRAW LINES
			glm::mat4 screenSpaceProj = glm::ortho(-aspect, aspect, -1.f, 1.f);
			for (unsigned int i = 0; i < sprites[currentSprite]->polygons.size(); i++) {
				Polygon* poly = &sprites[currentSprite]->polygons.at(i);
				float averageX = 0.f;
				float averageY = 0.f;
				int polygonPoints = poly->points.size();
				for (int j = 0; j < polygonPoints; j++) {
					Vec2f p1 = poly->points.at(j);
					Vec2f p2 = poly->points.at((j + 1) % polygonPoints);

					averageX += p1.x;
					averageY += p1.y;
					glm::mat4 model = glm::mat4(1.f);
					model = glm::translate(model, glm::vec3((p1.x + p2.x) / 2.f, (p1.y + p2.y) / 2.f, 0.f));
					model = glm::rotate(model, atan((p2.x - p1.x) / (p2.y - p1.y)), glm::vec3(0.f, 0.f, -1.f));
					float length = Vec2f::distance(p1, p2);
					model = glm::scale(model, glm::vec3(0.05f * scale, length, 1.f));

					glm::mat4 trans = proj * model;
					lineShader.use();
					glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
					bool isLineRed = ctrlKeyDown && (j == closestPoint || ((j + 1) % polygonPoints) == closestPoint);
					bool isLineClosest = j == closestLine;
					glUniform4f(lineShader.colorLoc, 1.f, isLineRed ? 0.f : 1.f, isLineClosest ? 1.f : 0.f, i == selectedPolygon ? 1.f : 0.2f);

					glBindVertexArray(VAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					// point

					model = glm::mat4(1.f);
					model = glm::translate(model, glm::vec3(p1.x, p1.y, 0.f));
					model = glm::scale(model, glm::vec3(0.1f * scale, 0.1f * scale, 1.f));

					trans = proj * model;
					lineShader.use();
					glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
					bool isPointRed = ctrlKeyDown && j == closestPoint;
					bool isPointSelected = j == draggingPoint || (j == closestPoint && draggingPoint == -1);
					glUniform4f(lineShader.colorLoc, 1.f, isPointRed ? 0.f : 1.f, isPointSelected ? 1.f : 0.f, i == selectedPolygon ? 0.5f : 0.1f);

					glBindVertexArray(VAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}
				averageX /= (float)poly->points.size();
				averageY /= (float)poly->points.size();
			}

			// split line
			if (isDrawingSplitLine) {
				glm::mat4 model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3((polygonSplitStart.x + polygonSplitEnd.x) / 2.f, (polygonSplitStart.y + polygonSplitEnd.y) / 2.f, 0.f));
				model = glm::rotate(model, atan((polygonSplitEnd.x - polygonSplitStart.x) / (polygonSplitEnd.y - polygonSplitStart.y)), glm::vec3(0.f, 0.f, -1.f));
				float length = Vec2f::distance(polygonSplitStart, polygonSplitEnd);
				model = glm::scale(model, glm::vec3(0.05f * scale, length, 1.f));

				glm::mat4 trans = proj * model;
				lineShader.use();
				glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(lineShader.colorLoc, 0.1f, 1.f, 1.f, 1.f);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
			// point on line

			if (closestLine != -1) {
				Vec2f p1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine);
				Vec2f p2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints);
				float progress = getPointProgressAlongLine(Vec2f(worldMouseX, worldMouseY), p1, p2);
				Vec2f pointOnLine = Vec2f::lerp(p1, p2, progrees);

				model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3(pointOnLine.x, pointOnLine.y, 0.f));
				model = glm::scale(model, glm::vec3(0.1f * scale, 0.1f * scale, 1.f));

				trans = proj * model;
				lineShader.use();
				glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(lineShader.colorLoc, 1.f, 1.f, 0.5f, 1.f);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}


			int howManyPoints = sprites[currentSprite]->polygons.at(selectedPolygon).points.size();
			std::string howManyPointsString = "Points: " + std::to_string(howManyPoints);

			std::string currentPolygonString = "Polygon " + std::to_string(selectedPolygon + 1) + "/" + to_string(sprites[currentSprite]->polygons.size());
			std::string currentSpriteString = "Sprite " + std::to_string(currentSprite + 1) + "/" + to_string(sprites.size());
			if (closestPoint != -1) {
				Vec2f closest = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint);
				std::string currentPointString = "Point " + to_string(closestPoint) + "(" + to_string(closest.x) + ", " + to_string(closest.y) + ")";
				renderText(glyphShader, currentPointString, 0.f, 0.75f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			}

			renderText(glyphShader, currentSpriteString, 0.f, 0.9f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, currentPolygonString, 0.f, 0.85f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, howManyPointsString, 0.f, 0.8f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);

			// controls
			renderText(glyphShader, "G - Generate collider", -aspect + 0.05f, 0.8f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, "RMB - Split", -aspect + 0.05f, 0.75f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);
			renderText(glyphShader, "<> - Switch polygon", -aspect + 0.05f, 0.7f, 0.001f, glm::vec4(1.f, 1.f, 1.f, 1.f), VAO, &screenSpaceProj);

			Polygon* poly = &sprites[currentSprite]->polygons.at(selectedPolygon);
			for (int i = 0; i < 10; i++) {
				model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3(aspect - 0.3f, -0.9f + (float)(9 - i) * 0.1f, 0.f));
				model = glm::scale(model, glm::vec3(0.3f, 0.1f, 1.f));
				model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.f));

				trans = screenSpaceProj * model;
				spriteShader.use();
				buttonTex->use();
				glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(spriteShader.boundsLoc, 0.f, 0.f, 1.f, 1.f);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				renderText(glyphShader, hitSoundNames[i], aspect - 0.3f + 0.02f, 0.03f - 0.9f + (float)(9 - i) * 0.1f, 0.001f, glm::vec4(1.f, 1.f, 1.f, !poly->isCustomHitSound && i == (int)poly->hitSound ? 1.f : 0.5f), VAO, &screenSpaceProj);
			}
			for (int i = 0; i < (int)frictionNames.size(); i++) {
				model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3(aspect - 0.6f, -0.8f + (float)i * 0.1f, 0.f));
				model = glm::scale(model, glm::vec3(0.3f, 0.1f, 1.f));
				model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.f));

				trans = screenSpaceProj * model;
				spriteShader.use();
				buttonTex->use();
				glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(spriteShader.boundsLoc, 0.f, 0.f, 1.f, 1.f);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				renderText(glyphShader, frictionNames.at(i), aspect - 0.6f + 0.02f, -0.77f + (float)i * 0.1f, 0.001f, glm::vec4(1.f, 1.f, 1.f, i == (int)poly->frictionIndex ? 1.f : 0.5f), VAO, &screenSpaceProj);
			}
			for (int i = 0; i < (int)customHitSoundNames.size(); i++) {
				model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3(-aspect, -0.9f + (float)i * 0.1f, 0.f));
				model = glm::scale(model, glm::vec3(0.3f, 0.1f, 1.f));
				model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.f));

				trans = screenSpaceProj * model;
				spriteShader.use();
				buttonTex->use();
				glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
				glUniform4f(spriteShader.boundsLoc, 0.f, 0.f, 1.f, 1.f);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				renderText(glyphShader, customHitSoundNames.at(i), -aspect + 0.02f, 0.03f - 0.9f + (float)i * 0.1f, 0.001f, glm::vec4(1.f, 1.f, 1.f, poly->isCustomHitSound && poly->customHitSound == customHitSoundNames.at(i) ? 1.f : 0.5f), VAO, &screenSpaceProj);
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
	glfwTerminate();

	return 0;
}
