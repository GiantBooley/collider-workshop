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
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


using namespace std;

struct Point {
	float x, y;
};
float square(float x) {
	return x * x;
}
float distance(Point a, Point b) {
	return sqrt(square(b.y - a.y) + square(b.x - a.x));
}
int realMod(int a, int b) {
	if (a >= 0) return a % b; else return (b >= 0 ? b : -b) - 1 + (a + 1) % b;
}
float dot(Point p1, Point p2) {
	return p1.x * p2.x + p1.y * p2.y;
}
string readFileText(string path) {
	ifstream file(path);
	string str;
	string file_contents = "";
	while (getline(file, str)) {
		file_contents += str;
		file_contents.push_back('\n');
	}
	return file_contents;
}

float getAbsAngleFromThreePoints(Point p1, Point p2, Point p3) {// p1 vertex
	return abs(atan((p3.y - p1.y) / (p3.x - p1.x)) - atan((p2.y - p1.y) / (p2.x - p1.x)));
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
string hitSoundNames[10] = {
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
const int howmanycustomnames = 14;
string customHitSoundNames[howmanycustomnames] = {
	"LeafLight", "FluffySnow", "Chain", "Porcelain", "Bone", "Bottle", "Frosting", "Pumpkin", "HollowPaper", "Leaf", "Gingerbread", "SolidPlastic", "SmallPlastic", "RustyMetal"
};
const int howmanyfriction = 13;
string frictionNames[howmanyfriction] = {
	"25", "0.5", "1", "3", "25", "1000", "Anvil", "bouncy candy", "NoFriction", "PartialFriction", "SlidingFriction", "StaticFriction", "Superfriction"
};
class Polygon {
public:
	vector<Point> points;
	SoundMaterial::SoundMaterial hitSound;
	bool isCustomHitSound;
	string customHitSound;
	int friction;
	unsigned char r, g, b;

	Polygon(vector<Point> ponts, bool isCustom, SoundMaterial::SoundMaterial sond, string customsond, int fricton) {
		points = ponts;
		hitSound = sond;
		customHitSound = customsond;
		isCustomHitSound = isCustom;
		friction = fricton;
		r = 128;
		g = 128;
		b = 128;
	}
};

class Texture {
public:
	string path;
	int width, height, numChannels;
	unsigned int texture;
	unsigned char* data;
	Texture() {}
	Texture(const char* path) {
		load(path);
	}
	~Texture() {
		stbi_image_free(data);
	}
	void load(const char* patha) {
		path = patha;
		stbi_set_flip_vertically_on_load(true);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


		data = stbi_load(patha, &width, &height, &numChannels, 0);
		cout << "tex \"" << path << "\" has " << numChannels << " color channels" << endl;
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, numChannels == 4 ? GL_RGBA : GL_RGB, width, height, 0, numChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
			cout << "[ERROR] failed to load \"" << path << "\"" << endl;
		}
	}
	void use() {
		glBindTexture(GL_TEXTURE_2D, texture);
	}
};
vector<shared_ptr<Texture>> texturesLoaded = {};
shared_ptr<Texture> getTexture(const char* path) {
	filesystem::path file{path};
	for (shared_ptr<Texture> t : texturesLoaded) {
		filesystem::path texFile{t->path.c_str()};
		if (filesystem::equivalent(file, texFile)) {
			return t;
		}
	}
	shared_ptr<Texture> texture = make_shared<Texture>(path);
	texturesLoaded.push_back(texture);
	return texture;
}
class Sprite {
public:
	string fileName;
	string objectName;
	shared_ptr<Texture> tex;
	vector<Polygon> polygons;
	float width, height;
	float minX, minY, maxX, maxY;
	int pixelMinX, pixelMinY, pixelMaxX, pixelMaxY;
	Sprite(string objectnamea, string filenamea, float minx, float miny, float maxx, float maxy) {
		fileName = filenamea;
		tex = getTexture(fileName.c_str());
		minX = minx / (float)tex->width;
		minY = miny / (float)tex->height;
		maxX = maxx / (float)tex->width;
		maxY = maxy / (float)tex->height;
		pixelMinX = (int)minx;
		pixelMinY = (int)miny;
		pixelMaxX = (int)maxx;
		pixelMaxY = (int)maxy;
		objectName = objectnamea;
		width = (float)(pixelMaxX - pixelMinX) / 100.f;
		height = (float)(pixelMaxY - pixelMinY) / 100.f;

		polygons.push_back({{{-width / 2.f,-height / 2.f},{-width / 2.f,height / 2.f},{width / 2.f,height / 2.f},{width / 2.f,-height / 2.f}}, false, SoundMaterial::rock, "", 0});
	}
};
class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath) {
		string vertexText;
		string fragmentText;
		ifstream vertexFile;
		ifstream fragmentFile;
		vertexFile.exceptions(ifstream::failbit | ifstream::badbit);
		fragmentFile.exceptions(ifstream::failbit | ifstream::badbit);
		try {
			vertexFile.open(vertexPath);
			fragmentFile.open(fragmentPath);
			stringstream vertexStream, fragmentStream;
			vertexStream << vertexFile.rdbuf();
			fragmentStream << fragmentFile.rdbuf();
			vertexFile.close();
			fragmentFile.close();
			vertexText = vertexStream.str();
			fragmentText = fragmentStream.str();
		} catch (ifstream::failure e) {
			cout << "[ERROR] failed to get fragment or vertex text" << endl;
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
			cout << "[ERROR] vertex shader compile failed\n" << infoLog << endl;
		}
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			cout << "[ERROR] fragment shader compile failed\n" << infoLog << endl;
		}

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		//errors
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			cout << "[ERROR] program failed linking\n" << infoLog << endl;
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
class GlyphShader : public Shader {
public:
	unsigned int transformLoc, textColorLoc;
	GlyphShader(const char* vertexPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {
		transformLoc = glGetUniformLocation(ID, "transform");
		textColorLoc = glGetUniformLocation(ID, "textColor");
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
		if (button == GLFW_MOUSE_BUTTON_LEFT) didMouseRelease = true;
		if (button == GLFW_MOUSE_BUTTON_RIGHT) didRightMouseRelease = true;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) didMiddleRelease = true;
	}
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
float lerp(float a, float b, float t) {
	return (b - a) * t + a;
}
AABB getViewBounds(float zoom, float cameraX, float cameraY) {
	float aspect = (float)frameWidth / (float)frameHeight;
	return {-zoom / 2.f * aspect - cameraX, zoom / 2.f * aspect - cameraX, -zoom / 2.f - cameraY, zoom / 2.f - cameraY};
}
float distanceFromPointToLine(float x, float y, float x1, float y1, float x2, float y2) {
	if (x1 == x2) return abs(x - x1);
	float slope = (y2 - y1) / (x2 - x1);
	return abs(slope * (x - x1) - y + y1) / sqrt(slope * slope + 1.f);
}
float getPointProgressAlongLine(float x, float y, float x1, float y1, float x2, float y2) {
	if (x1 == x2) return (y - y1) / (y2 - y1);
	float slope = (y2 - y1) / (x2 - x1);
	return ((x + slope * (y - y1 + slope * x1)) / (slope * slope + 1.f) - x1) / (x2 - x1);
}
struct iVec2 {
	int x, y;
};
void generateCollider(Sprite* sprite) { // 0 1 2 3 clockwise
	const unsigned char alphaThreshold = 127;
	int w = sprite->tex->width;
	int h = sprite->tex->height;
	bool exit = false;
	for (int y = sprite->pixelMinY; y < sprite->pixelMaxY; y++) {
		for (int x = sprite->pixelMinX; x < sprite->pixelMaxX; x++) {
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
							(pointX - sprite->pixelMinX) / 100.f - sprite->width / 2.f,
							(pointY - sprite->pixelMinY) / 100.f - sprite->height / 2.f
						});
					}
				}
			}
			if (exit) break;
		}
		if (exit) break;
	}
}

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;
void renderText(GlyphShader &s, string text, float x, float y, float scale, glm::vec4 color, unsigned int VAO, glm::mat4* vp) {
    // activate corresponding render state
    s.use();
    glUniform4f(s.textColorLoc, color.x, color.y, color.z, color.w);
    glActiveTexture(GL_TEXTURE0);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(xpos, ypos, 0.f));
		model = glm::scale(model, glm::vec3(w, h, 1.f));
		model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.f));

		glm::mat4 mvp = *vp * model;
		glUniformMatrix4fv(s.transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// render quad
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
bool lineIntersection(Point p0, Point p1, Point p2, Point p3, Point* intersection) {
	Point s1, s2;
	s1.x = p1.x - p0.x;     s1.y = p1.y - p0.y;
	s2.x = p3.x - p2.x;     s2.y = p3.y - p2.y;

	float s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
	float t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);

	if (s >= 0.f && s <= 1.f && t >= 0.f && t <= 1.f) {
		// Collision detected
		intersection->x = p0.x + (t * s1.x);
		intersection->y = p0.y + (t * s1.y);
		return true;
	}

	return false; // No collision
}
void splitPolygon(Sprite* sprite, int* selectedPolygon, Point p1, Point p2) {
	vector<Point> polygonPoints;
	vector<int> isIntersectionPoints; // 0 normal 1 entry 2 exit
	bool isEntryPoint = true;
	int intersections = 0;
	for (unsigned int i = 0; i < sprite->polygons.at(*selectedPolygon).points.size(); i++) {
		polygonPoints.push_back(sprite->polygons.at(*selectedPolygon).points.at(i));
		isIntersectionPoints.push_back(0);
		Point intersection;
		unsigned int second = (i + 1) % sprite->polygons.at(*selectedPolygon).points.size();
		if (lineIntersection(sprite->polygons.at(*selectedPolygon).points.at(i), sprite->polygons.at(*selectedPolygon).points.at(second), p1, p2, &intersection)) {
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
		sprite->polygons.push_back({{}, poly->isCustomHitSound, poly->hitSound, poly->customHitSound, poly->friction});
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
	GLFWwindow* window = glfwCreateWindow(frameWidth, frameHeight, "Colliders", NULL, NULL);
	if (window == NULL) {
		cout << "[ERROR] Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "[ERROR] Failed to initialize glad" << endl;
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//stuff
	glViewport(0, 0, frameWidth, frameHeight);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	SpriteShader spriteShader{"vertex.vsh", "fragment.fsh"};
	LineShader lineShader{"vertex.vsh", "line.fsh"};
	GlyphShader glyphShader{"vertex.vsh", "glyph.fsh"};
	shared_ptr<Texture> buttonTex = getTexture("button.png");
	vector<Sprite*> sprites = {};

	string colliderFileText = readFileText("colliders.txt");
	istringstream iss(colliderFileText);

	int howman = 0;
	string objectName, spriteFileName;
	float minX, minY, maxX, maxY;
	for (std::string line; std::getline(iss, line);) {
		if (howman % 3 == 0) { // name
			objectName = line;
		} else if (howman % 3 == 1) { // sprite path
			spriteFileName = line;
		} else if (howman % 3 == 2) { // bounds
			stringstream boundsStringStream{line};
			string boundNumberString;
			vector<float> boundNumbers = {};
			while (getline(boundsStringStream, boundNumberString, ',')) {
				boundNumbers.push_back(stof(boundNumberString));
			}
			Sprite* sprite = new Sprite(objectName, spriteFileName, boundNumbers.at(0), boundNumbers.at(1), boundNumbers.at(2), boundNumbers.at(3));
			sprites.push_back(sprite);
		}
		howman++;
	}

	int currentSprite = 0;

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

	// init freetype
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	FT_Face face;
	if (FT_New_Face(ft, "./DMSans-VariableFont.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);
	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		return -1;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 32; c < 128; c++) {
		// load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}


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

	Point polygonSplitStart;
	Point polygonSplitEnd;
	bool isDrawingSplitLine = false;


	//llooop
	while (!glfwWindowShouldClose(window)) {
		float aspect = (float)frameWidth / (float)frameHeight;

		zoom /= pow(1.1, yScroll);
		zoom = min(max(zoom, 0.1f), 100.f);

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
			for (int i = 0; i < howmanyfriction; i++) {
				if (mouseEcks > (aspect - 0.6f) && mouseEcks < (aspect - 0.3f) && mouseWhy > -0.5f + (float)i * 0.1f && mouseWhy < -0.5f + (float)(i + 1) * 0.1f) {
					sprites[currentSprite]->polygons.at(selectedPolygon).friction = i;
					didMousePress = false;
					break;
				}
			}
			for (int i = 0; i < howmanycustomnames; i++) {
				if (mouseEcks < -aspect + 0.3f && mouseWhy > -0.9f + (float)i * 0.1f && mouseWhy < -0.9f + (float)(i + 1) * 0.1f) {
					sprites[currentSprite]->polygons.at(selectedPolygon).isCustomHitSound = true;
					sprites[currentSprite]->polygons.at(selectedPolygon).customHitSound = customHitSoundNames[i];
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
			sprites[currentSprite]->polygons.push_back({{}, false, SoundMaterial::rock, "", 0});
			generateCollider(sprites[currentSprite]);
			Polygon* poly = &sprites[currentSprite]->polygons.at(0);
			int howManyDecimationSteps = poly->points.size();
			float minDistance = 0.02f;
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

			// planar decimation

			int homwan = poly->points.size();
			for (int iteration = 0; iteration < homwan; iteration++) {
				poly = &sprites[currentSprite]->polygons.at(0);
				// find lowest angle
				float lowestAngle = 0.f;
				float lowestIndex = -1;
				for (int i = poly->points.size() - 1; i >= 0; i--) {
					Point p1 = poly->points.at(i);
					Point p2 = poly->points.at(realMod(i - 1, poly->points.size()));
					Point p3 = poly->points.at(realMod(i + 1, poly->points.size()));
					float angle = getAbsAngleFromThreePoints(p1, p2, p3);
					float minAngle = glm::radians(3.f / distance(p2, p3)); // lower number is high poly
					if ((lowestIndex == -1 || angle < lowestAngle) && angle < minAngle) {
						lowestIndex = i;
						lowestAngle = angle;
					}
				}
				if (lowestIndex == -1) break;
				poly->points.erase(poly->points.begin() + lowestIndex);
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

			float dist = sqrt(square(worldMouseX - x1) + square(worldMouseY - y1));
			if (dist < closestDistance || closestPoint == -1) {
				closestDistance = dist;
				closestPoint = j;
			}
		}
		if (!spaceDown && closestPoint != -1 && max(abs(sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint).x - worldMouseX), abs(sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint).y - worldMouseY)) < 0.2f * scale) {
			if (didMousePress) {
				if (ctrlKeyDown && polygonPoints > 3) {
					sprites[currentSprite]->polygons.at(selectedPolygon).points.erase(sprites[currentSprite]->polygons.at(selectedPolygon).points.begin() + closestPoint);
					polygonPoints--;
					closestPoint = -1;
				} else {
					draggingPoint = closestPoint;
				}
			}
		} else closestPoint = -1;

		//get closest line
		int closestLine = -1;
		float closestLineDistance = 0.f;
		for (int j = 0; j < polygonPoints; j++) {
			float x1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(j).x;
			float y1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(j).y;
			float x2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((j + 1) % polygonPoints).x;
			float y2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((j + 1) % polygonPoints).y;

			float dist = distanceFromPointToLine(worldMouseX, worldMouseY, x1, y1, x2, y2);
			float progress = getPointProgressAlongLine(worldMouseX, worldMouseY, x1, y1, x2, y2);
			if ((dist < closestLineDistance || closestLine == -1) && progress >= 0.f && progress <= 1.f) {
				closestLineDistance = dist;
				closestLine = j;
			}
		}
		if (!spaceDown && closestLine != -1 && closestLineDistance < 0.2f * scale && closestPoint == -1 && draggingPoint == -1) {
			if (didMousePress) {
				float x1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine).x;
				float y1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine).y;
				float x2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints).x;
				float y2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints).y;
				float progress = getPointProgressAlongLine(worldMouseX, worldMouseY, x1, y1, x2, y2);
				float ecks = lerp(x1, x2, progress);
				float why = lerp(y1, y2, progress);
				sprites[currentSprite]->polygons.at(selectedPolygon).points.insert(sprites[currentSprite]->polygons.at(selectedPolygon).points.begin() + closestLine + 1, {ecks, why});
				closestPoint = closestLine + 1;
				draggingPoint = closestLine + 1;
				closestLine = -1;
				polygonPoints++;
			}
		} else closestLine = -1;

		// select polygon
		if (didMousePress && closestLine == -1 && closestPoint == -1) {
			for (int i = 0; i < (int)sprites[currentSprite]->polygons.size(); i++) {
				int howManyIntersections = 0;
				int polygonPoints = sprites[currentSprite]->polygons.at(i).points.size();
				for (int j = 0; j < polygonPoints; j++) {
					Point intersection;
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
					break;
				}
			}
		}



		didMousePress = false;
		if (didMouseRelease) {
			draggingPoint = -1;
		}
		didMouseRelease = false;

		if (selectedPolygon != -1 && draggingPoint != -1) {
			sprites[currentSprite]->polygons.at(selectedPolygon).points.at(draggingPoint).x = worldMouseX;
			sprites[currentSprite]->polygons.at(selectedPolygon).points.at(draggingPoint).y = worldMouseY;
		}

		if (sPressed && ctrlKeyDown) {
			ofstream file;
			file.open("colliders_for_unity.txt");
			for (Sprite* s : sprites) {
				file << s->objectName << "\n";
				for (Polygon& poly : s->polygons) {
					file << "polygon\n";
					file << poly.isCustomHitSound << "\n";
					if (poly.isCustomHitSound) {
						file << poly.customHitSound << "\n";
					} else {
						file << (int)poly.hitSound << "\n";
					}
					file << frictionNames[poly.friction] << "\n";
					for (Point& p : poly.points) {
						file << p.x << "x" << p.y << "y";
					}
					file << "\n";
				}
			}
			file.close();
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
				float x1 = poly->points.at(j).x;
				float y1 = poly->points.at(j).y;
				averageX += x1;
				averageY += y1;
				float x2 = poly->points.at((j + 1) % polygonPoints).x;
				float y2 = poly->points.at((j + 1) % polygonPoints).y;
				glm::mat4 model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3((x1 + x2) / 2.f, (y1 + y2) / 2.f, 0.f));
				model = glm::rotate(model, atan((x2 - x1) / (y2 - y1)), glm::vec3(0.f, 0.f, -1.f));
				float length = sqrt(square(x2 - x1) + square(y2 - y1));
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
				model = glm::translate(model, glm::vec3(x1, y1, 0.f));
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

			renderText(glyphShader, poly->isCustomHitSound ? poly->customHitSound : hitSoundNames[poly->hitSound], averageX, averageY, 0.005f, glm::vec4(1.f, 1.f, 1.f, i == selectedPolygon ? 1.f : 1.f), VAO, &proj);
			renderText(glyphShader, frictionNames[poly->friction], averageX, averageY - 0.2f, 0.005f, glm::vec4(1.f, 1.f, 1.f, i == selectedPolygon ? 1.f : 1.f), VAO, &proj);
		}

		// split line
		if (isDrawingSplitLine) {
			glm::mat4 model = glm::mat4(1.f);
			model = glm::translate(model, glm::vec3((polygonSplitStart.x + polygonSplitEnd.x) / 2.f, (polygonSplitStart.y + polygonSplitEnd.y) / 2.f, 0.f));
			model = glm::rotate(model, atan((polygonSplitEnd.x - polygonSplitStart.x) / (polygonSplitEnd.y - polygonSplitStart.y)), glm::vec3(0.f, 0.f, -1.f));
			float length = sqrt(square(polygonSplitEnd.x - polygonSplitStart.x) + square(polygonSplitEnd.y - polygonSplitStart.y));
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
			float x1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine).x;
			float y1 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestLine).y;
			float x2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints).x;
			float y2 = sprites[currentSprite]->polygons.at(selectedPolygon).points.at((closestLine + 1) % polygonPoints).y;
			float progress = getPointProgressAlongLine(worldMouseX, worldMouseY, x1, y1, x2, y2);
			float ecks = lerp(x1, x2, progress);
			float why = lerp(y1, y2, progress);

			model = glm::mat4(1.f);
			model = glm::translate(model, glm::vec3(ecks, why, 0.f));
			model = glm::scale(model, glm::vec3(0.1f * scale, 0.1f * scale, 1.f));

			trans = proj * model;
			lineShader.use();
			glUniformMatrix4fv(lineShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
			glUniform4f(lineShader.colorLoc, 1.f, 1.f, 0.5f, 1.f);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}


		int howManyPoints = sprites[currentSprite]->polygons.at(selectedPolygon).points.size();
		string howManyPointsString = "Points: " + to_string(howManyPoints);

		string currentPolygonString = "Polygon " + to_string(selectedPolygon + 1) + "/" + to_string(sprites[currentSprite]->polygons.size());
		string currentSpriteString = "Sprite " + to_string(currentSprite + 1) + "/" + to_string(sprites.size());
		if (closestPoint != -1) {
			Point closest = sprites[currentSprite]->polygons.at(selectedPolygon).points.at(closestPoint);
			string currentPointString = "Point " + to_string(closestPoint) + "(" + to_string(closest.x) + ", " + to_string(closest.y) + ")";
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
		for (int i = 0; i < howmanyfriction; i++) {
			model = glm::mat4(1.f);
			model = glm::translate(model, glm::vec3(aspect - 0.6f, -0.5f + (float)i * 0.1f, 0.f));
			model = glm::scale(model, glm::vec3(0.3f, 0.1f, 1.f));
			model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.f));

			trans = screenSpaceProj * model;
			spriteShader.use();
			buttonTex->use();
			glUniformMatrix4fv(spriteShader.transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
			glUniform4f(spriteShader.boundsLoc, 0.f, 0.f, 1.f, 1.f);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			renderText(glyphShader, frictionNames[i], aspect - 0.6f + 0.02f, -0.47f + (float)i * 0.1f, 0.001f, glm::vec4(1.f, 1.f, 1.f, i == (int)poly->friction ? 1.f : 0.5f), VAO, &screenSpaceProj);
		}
		for (int i = 0; i < howmanycustomnames; i++) {
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

			renderText(glyphShader, customHitSoundNames[i], -aspect + 0.02f, 0.03f - 0.9f + (float)i * 0.1f, 0.001f, glm::vec4(1.f, 1.f, 1.f, poly->isCustomHitSound && poly->customHitSound == customHitSoundNames[i] ? 1.f : 0.5f), VAO, &screenSpaceProj);
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
	glfwTerminate();
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return 0;
}
