#ifndef IMAGEDATA_TPP
#define IMAGEDATA_TPP


#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

template <typename T>
ImageData<T>::ImageData(T wv) : whiteValue(wv), width(0), height(0), colch(0), filePath("") {}

template <typename T>
ImageData<T>::ImageData(size_t w, size_t h, size_t colc, T wv) : data(w * h * colc), whiteValue(wv), width(w), height(h), colch(colc), filePath("") {}

template <typename T>
ImageData<T>::ImageData(const char* fileName, T wv) : whiteValue(wv), filePath(fileName) {
    int w, h, fileChannels;

    stbi_set_flip_vertically_on_load(false);
    uint8_t* pixels = stbi_load(fileName, &w, &h, &fileChannels, 0);

    if (!pixels) {
        std::cout << "Failed to load image: \"" << fileName << "\": " << stbi_failure_reason() << std::endl;
        width = height = colch = 0;
        return;
    }

    width  = static_cast<size_t>(w);
    height = static_cast<size_t>(h);
    colch  = static_cast<size_t>(fileChannels);

    const size_t totalPixels = width * height * colch;
    data.resize(totalPixels);

    const double scale = static_cast<double>(whiteValue) / 255.0;
    for (size_t i = 0; i < totalPixels; ++i)
        data[i] = static_cast<T>(pixels[i] * scale);

    stbi_image_free(pixels);
}

template <typename T>
size_t ImageData<T>::getWidth() const {
    return width;
}
template <typename T>
size_t ImageData<T>::getHeight() const {
    return height;
}
template <typename T>
size_t ImageData<T>::getChannels() const {
    return colch;
}
template <typename T>
double ImageData<T>::sampleNormalAlpha(size_t x, size_t y) const {
    if (colch < 4) return 1.0;
    return static_cast<double>(data[(y * width + x) * colch + 3]) / static_cast<double>(whiteValue);
}
template <typename T>
bool ImageData<T>::checkBounds(size_t x, size_t y) const {
    return x < width && y < height;
}
template <typename T>
bool ImageData<T>::checkBounds(int x, int y) const {
    return static_cast<size_t>(x) < width && static_cast<size_t>(y) < height && x >= 0 && y >= 0;
}
template <typename T>
void ImageData<T>::setToWhite(size_t x, size_t y) {
    T* pixel = &data[(y * width + x) * colch];
    for (size_t i = 0; i < colch; i++) {
        pixel[i] = whiteValue;
    }
}
template <typename T>
void ImageData<T>::setToZero(size_t x, size_t y) {
    T* pixel = &data[(y * width + x) * colch];
    for (size_t i = 0; i < colch; i++) {
        pixel[i] = T();
    }
}


template <typename T>
void ImageData<T>::convertColorChannels(size_t newColorChannels) {
    if (colch == newColorChannels) return;

    std::vector<T> newData(width * height * newColorChannels);
    for (size_t i = 0; i < width * height; i++) {
        for (size_t j = 0; j < newColorChannels; j++) {
            if (j < colch) {
                newData[i * newColorChannels + j] = data[i * colch + j];
            } else {
                newData[i * newColorChannels + j] = (j == 3) ? whiteValue : static_cast<T>(0);
            }
        }
    }

    data = newData;
    colch = newColorChannels;
}

template <typename T>
T ImageData<T>::getWhiteValue() const {
    return whiteValue;
}

template<typename T>
inline void ImageData<T>::getNormalizedPixelBW(size_t x, size_t y, float* gray) const {
    const float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: case 2: *gray = static_cast<float>(data[pixel]) * m; break; // BW or BWA: get first color
        case 3: case 4: *gray = m * (static_cast<float>(data[pixel]) * 0.299f + static_cast<float>(data[pixel + 1]) * 0.587f + static_cast<float>(data[pixel + 2]) * 0.114f); // RGB or RGBA: get gray
    }
}
template<typename T>
inline void ImageData<T>::getNormalizedPixelBWA (size_t x, size_t y, float* gray, float* alpha) const {
    const float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: case 2: *gray = static_cast<float>(data[pixel]) * m; break; // BW or BWA: get first color
        case 3: case 4: *gray = m * (static_cast<float>(data[pixel]) * 0.299f + static_cast<float>(data[pixel + 1]) * 0.587f + static_cast<float>(data[pixel + 2]) * 0.114f); // RGB or RGBA: get gray
    }
    switch (colch) {
        case 1: case 3: *alpha = 1.f; break; // BW or RGB: get full alpha
        case 2: *alpha = static_cast<float>(data[pixel + 1]) * m; break; // BWA: get alpha
        case 4: *alpha = static_cast<float>(data[pixel + 3]) * m; // RGBA: get alpha
    }
}
template<typename T>
inline void ImageData<T>::getNormalizedPixelRGB (size_t x, size_t y, float* red,  float* green, float* blue) const {
    const float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    float gray;
    switch (colch) {
        case 1: case 2: gray = static_cast<float>(data[pixel]) * m; *red = gray; *green = gray; *blue = gray; break; // BW or BWA: get first color for all 3
        case 3: case 4: *red = static_cast<float>(data[pixel]) * m; *green = static_cast<float>(data[pixel + 1]) * m; *blue = static_cast<float>(data[pixel + 2]) * m; // RGB or RGBA: get rgb
    }
}
template<typename T>
inline void ImageData<T>::getNormalizedPixelRGBA(size_t x, size_t y, float* red,  float* green, float* blue, float* alpha) const {
    const float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    float gray;
    switch (colch) {
        case 1: case 2: gray = static_cast<float>(data[pixel]) * m; *red = gray; *green = gray; *blue = gray; break; // BW or BWA: get first color for all 3
        case 3: case 4: *red = static_cast<float>(data[pixel]) * m; *green = static_cast<float>(data[pixel + 1]) * m; *blue = static_cast<float>(data[pixel + 2]) * m; // RGB or RGBA: get rgb
    }
    switch (colch) {
        case 1: case 3: *alpha = 1.f; break; // BW or RGB: get full alpha
        case 2: *alpha = static_cast<float>(data[pixel + 1]) * m; break; // BWA: get alpha
        case 4: *alpha = static_cast<float>(data[pixel + 3]) * m; // RGBA: get alpha
    }
}
template<typename T>
inline void ImageData<T>::getNormalizedPixelA(size_t x, size_t y, float* alpha) const {
    const float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: case 3: *alpha = 1.f; break; // BW or RGB: get full alpha
        case 2: *alpha = static_cast<float>(data[pixel + 1]) * m; break; // BWA: get alpha
        case 4: *alpha = static_cast<float>(data[pixel + 3]) * m; // RGBA: get alpha
    }
}

template<typename T>
inline void ImageData<T>::setNormalizedPixelBW  (size_t x, size_t y, float gray) {
    const float m = static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    T grayCast = static_cast<T>(gray * m);
    switch (colch) {
        case 1: data[pixel] = grayCast; break; // BW: set to gray
        case 2: data[pixel] = grayCast; data[pixel + 1] = whiteValue; break; // BWA: set to gray with full alpha

        case 3: data[pixel] = grayCast; data[pixel + 1] = grayCast; data[pixel + 2] = grayCast; break; // RGB: set to gray
        case 4: data[pixel] = grayCast; data[pixel + 1] = grayCast; data[pixel + 2] = grayCast; data[pixel + 3] = whiteValue; // RGBA: set to gray with full alpha
    }
}
template<typename T>
inline void ImageData<T>::setNormalizedPixelBWA (size_t x, size_t y, float gray, float alpha) {
    const float m = static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    T grayCast = static_cast<T>(gray * m);
    switch (colch) {
        case 1: data[pixel] = grayCast; break; // BW: set to gray
        case 2: data[pixel] = grayCast; data[pixel + 1] = static_cast<T>(alpha * m); break; // BWA: set to gray with alpha

        case 3: data[pixel] = grayCast; data[pixel + 1] = grayCast; data[pixel + 2] = grayCast; break; // RGB: set to gray
        case 4: data[pixel] = grayCast; data[pixel + 1] = grayCast; data[pixel + 2] = grayCast; data[pixel + 3] = static_cast<T>(alpha * m); // RGBA: set to gray with alpha
    }
}
template<typename T>
inline void ImageData<T>::setNormalizedPixelRGB (size_t x, size_t y, float red,  float green, float blue) {
    const float m = static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: data[pixel] = static_cast<T>(m * (red * 0.299f + green * 0.587f + blue * 0.114f)); break; // BW: set to gray
        case 2: data[pixel] = static_cast<T>(m * (red * 0.299f + green * 0.587f + blue * 0.114f)); data[pixel + 1] = whiteValue; break; // BWA: set to gray with full alpha

        case 3: data[pixel] = static_cast<T>(red * m); data[pixel + 1] = static_cast<T>(green * m); data[pixel + 2] = static_cast<T>(blue * m); break; // RGB: set to rgb
        case 4: data[pixel] = static_cast<T>(red * m); data[pixel + 1] = static_cast<T>(green * m); data[pixel + 2] = static_cast<T>(blue * m); data[pixel + 3] = whiteValue; // RGBA: set to rgb with full alpha
    }
}
template<typename T>
inline void ImageData<T>::setNormalizedPixelRGBA(size_t x, size_t y, float red,  float green, float blue, float alpha) {
    const float m = static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: data[pixel] = static_cast<T>(m * (red * 0.299f + green * 0.587f + blue * 0.114f)); break; // BW: set to gray
        case 2: data[pixel] = static_cast<T>(m * (red * 0.299f + green * 0.587f + blue * 0.114f)); data[pixel + 1] = static_cast<T>(alpha * m); break; // BWA: set to gray with alpha

        case 3: data[pixel] = static_cast<T>(red * m); data[pixel + 1] = static_cast<T>(green * m); data[pixel + 2] = static_cast<T>(blue * m); break; // RGB: set to rgb
        case 4: data[pixel] = static_cast<T>(red * m); data[pixel + 1] = static_cast<T>(green * m); data[pixel + 2] = static_cast<T>(blue * m); data[pixel + 3] = static_cast<T>(alpha * m); // RGBA: set to rgb with alpha
    }
}
template<typename T>
inline void ImageData<T>::setNormalizedPixelA(size_t x, size_t y, float alpha) {
    const float m = static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 2: data[pixel] = data[pixel + 1] = static_cast<T>(alpha * m); break; // BWA: set with alpha
        case 4: data[pixel] = data[pixel + 3] = static_cast<T>(alpha * m); // RGBA: set with alpha
    }
}

template <typename T>
void ImageData<T>::loadTexture() {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ImageData<uint8_t> byteImage = convertToType<uint8_t>(255);

	glTexImage2D(GL_TEXTURE_2D, 0, colch == 1 ? GL_RED : (colch == 3 ? GL_RGB : GL_RGBA), width, height, 0, colch == 1 ? GL_RED : (colch == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, byteImage.data.data());
}

template <typename T>
void ImageData<T>::bindTexture() {
	glBindTexture(GL_TEXTURE_2D, texture);
}

template <typename T>
void ImageData<T>::setTextureFilter(bool linear) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}


template <typename T>
template <typename newType>
ImageData<newType> ImageData<T>::convertToType(newType newWhiteValue) const {
    ImageData<newType> newImageData{width, height, colch, newWhiteValue};
    newImageData.filePath = filePath;
    newImageData.data.resize(width * height * colch);
    for (size_t i = 0; i < width * height * colch; i++) {
        double intermediate = static_cast<double>(data[i]) / static_cast<double>(whiteValue);
        newImageData.data[i] = static_cast<newType>(intermediate * static_cast<double>(newWhiteValue));
    }
    return newImageData;
}

template <typename T>
std::string ImageData<T>::getFilePath() const {
    return filePath;
}


template <typename T>
std::shared_ptr<IImageData> ImageData<T>::convertTo(ImageType type) const {
    switch (type) {
    case FLOAT32:
        return std::make_shared<ImageData<float>>(convertToType<float>(1.f));
    case UINT8:
        return std::make_shared<ImageData<uint8_t>>(convertToType<uint8_t>(255));
    }
    // default uint8
    return std::make_shared<ImageData<uint8_t>>(convertToType<uint8_t>(255));
}

template <typename T>
void ImageData<T>::saveToPngFile(std::string filePath) const {
    int width = getWidth();
    int height = getHeight();
    int channels = getChannels();
    stbi_write_png(filePath.c_str(), width, height, channels, data.data(), width * channels);
}

#endif
