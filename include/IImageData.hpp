#ifndef IIMAGEDATA_HPP
#define IIMAGEDATA_HPP

#include <cstddef>
#include <memory>
#include <string>

enum ImageType {
	FLOAT32,
	UINT8
};
class IImageData { // image data interface
public:
	virtual ~IImageData() = default;


	virtual void convertColorChannels(size_t newColorChannels) = 0;

	//virtual void saveToFile(std::string fileName) const = 0;

	virtual size_t getWidth() const = 0;
	virtual size_t getHeight() const = 0;
	virtual size_t getChannels() const = 0;
	virtual void setToZero(size_t x, size_t y) = 0;
	virtual void setToWhite(size_t x, size_t y) = 0;
	virtual double sampleNormalAlpha(size_t x, size_t y) const = 0;
	virtual bool checkBounds(size_t x, size_t y) const = 0;
	virtual bool checkBounds(int x, int y) const = 0;

	inline virtual void getNormalizedPixelBW  (size_t x, size_t y, float* gray) const = 0;
	inline virtual void getNormalizedPixelBWA (size_t x, size_t y, float* gray, float* alpha) const = 0;
	inline virtual void getNormalizedPixelRGB (size_t x, size_t y, float* red,  float* green, float* blue) const = 0;
	inline virtual void getNormalizedPixelRGBA(size_t x, size_t y, float* red,  float* green, float* blue, float* alpha) const = 0;
	inline virtual void getNormalizedPixelA   (size_t x, size_t y, float* alpha) const = 0;

	inline virtual void setNormalizedPixelBW  (size_t x, size_t y, float gray) = 0;
	inline virtual void setNormalizedPixelBWA (size_t x, size_t y, float gray, float alpha) = 0;
	inline virtual void setNormalizedPixelRGB (size_t x, size_t y, float red,  float green, float blue) = 0;
	inline virtual void setNormalizedPixelRGBA(size_t x, size_t y, float red,  float green, float blue, float alpha) = 0;
	inline virtual void setNormalizedPixelA   (size_t x, size_t y, float alpha) = 0;

	virtual void loadTexture() = 0;
	virtual void bindTexture() = 0;
	virtual void setTextureFilter(bool linear) = 0;

	virtual std::string getFilePath() const = 0;

	virtual std::shared_ptr<IImageData> convertTo(ImageType type) const = 0;

	virtual void saveToPngFile(std::string filePath) const = 0;
};

#endif // IIMAGEDATA_HPP
