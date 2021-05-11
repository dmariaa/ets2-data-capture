#pragma once
#include "globals.h"

#pragma pack(2)
/// <summary>
/// Screenshot header
/// This BMP head comes from
/// https://github.com/philkr/gamehook
/// </summary>
struct BMHead {
	char magic[2] = { 'B', 'M' };
	int size;
	int _reserved = 0;
	int offset = 0x36;
	int hsize = 40;
	int width, height;
	short planes = 1;
	short depth = 24;
	int compression = 0;
	int img_size = 0;
	int rx = 0x03C3,
		ry = 0x03C3;
	int ncol = 0,
		nimpcol = 0;
	BMHead(int W, int H) :width(W), height(-H) {
		int E = 4 - ((W * 3) % 4);
		img_size = (3 * W + E) * H;
		size = img_size + sizeof(BMHead);
	}
};

/// <summary>
/// Depth file Header
/// </summary>
struct DEPTHHead {
	char magic[2] = { 'D', 'P' };
	int size;
	int width;
	int height;
	float min_val;
	float max_val;
	int offset = 26;

	DEPTHHead(int W, int H) : width(W), height(H), min_val(1.0f), max_val(0.0f)
	{
		int data_size = width * height * 3;
		size = data_size + sizeof(DEPTHHead);
	}
};
#pragma pack()


class TextureBuffer
{
	std::vector<uint8_t> data;

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* texture;

	D3D11_MAPPED_SUBRESOURCE mappedRes;

	void saveScreenshot(const char* fileName);
	void saveDepth(const char* fileName);
public:
	enum Type
	{
		ScreenShot,
		Depth,
		Invalid
	};

	TextureBuffer::Type type;
	bool valid;

	TextureBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* origTexture);
	~TextureBuffer();

	void save(const char* fileName);	
	const size_t GetSize();
};

