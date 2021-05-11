#include "TextureBuffer.h"

TextureBuffer::TextureBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* origTexture)
	: texture(nullptr), valid(false), type(TextureBuffer::Type::Invalid)
{
	assert(origTexture != nullptr);

	origTexture->GetDesc(&desc);

	if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		type = Type::Depth;
	}
	else if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE || desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		type =  Type::ScreenShot;
	}

	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	HRESULT hr = pDevice->CreateTexture2D(&desc, nullptr, &texture);

	if (FAILED(hr))
	{
		PLOGE << "Error creating staging texture: " << ets2dc_utils::GetErrorDesc(hr);
		return;
	}

	pDeviceContext->CopyResource(texture, origTexture);
	hr = pDeviceContext->Map(texture, 0, D3D11_MAP_READ, 0, &mappedRes);

	if (FAILED(hr))
	{
		PLOGE << "Error mapping staging texture" << ets2dc_utils::GetErrorDesc(hr);
	}

	valid = true;
}

TextureBuffer::~TextureBuffer()
{
	if (texture != nullptr)
	{
		texture->Release();
	}
}

void TextureBuffer::saveScreenshot(const char* fileName)
{
	if (!valid)
	{
		PLOGE << "Invalid texture type";
		return;
	}

	BMHead header(desc.Width, desc.Height);

	std::string name(fileName);
	name += ".bmp";

	FILE* fp;
	fopen_s(&fp, name.c_str(), "wb");
	fwrite(&header, 1, sizeof(header), fp);

	int width = desc.Width;
	int height = desc.Height;
	int size = width * height;

	uint8_t* data = new uint8_t[3 * size];
	uint32_t* orig = reinterpret_cast<uint32_t*>(mappedRes.pData);

	int k_data = 0, k_orig = 0;

	for (int row = height - 1; row >= 0; row--)
	{
		for (int col = 0; col < width; col++) 
		{
			k_orig = row * width + col;

			data[k_data]     = (orig[k_orig] >> 16) & 0xff;
			data[k_data + 1] = (orig[k_orig] >> 8) & 0xff;
			data[k_data + 2] = (orig[k_orig] >> 0) & 0xff;

			k_data += 3;
		}
	}

	fwrite(data, sizeof(uint8_t), size * 3, fp);
	fclose(fp);

	delete[] data;
}

void TextureBuffer::saveDepth(const char* fileName)
{
	std::string name(fileName);
	name += ".depth.raw";

	DEPTHHead header(desc.Width, desc.Height);

	FILE* fp;
	fopen_s(&fp, name.c_str(), "wb");

	int width = desc.Width;
	int height = desc.Height;
	size_t size = width * height;

	uint8_t* data = new uint8_t[3 * size];
	uint32_t* orig = reinterpret_cast<uint32_t*>(mappedRes.pData);

	int k_data = 0, k_orig = 0;

	for (int row = height - 1; row >= 0; row--)
	{
		for (int col = 0; col < width; col++)
		{
			k_orig = row * width + col;

			float value = (float)(orig[k_orig] & 0x00ffffff) / 16777215.0f;
			if (value < header.min_val) header.min_val = value;
			if (value > header.max_val) header.max_val = value;
			
			data[k_data]     = (orig[k_orig] >> 0) & 0xff;
			data[k_data + 1] = (orig[k_orig] >> 8) & 0xff;
			data[k_data + 2] = (orig[k_orig] >> 16) & 0xff;

			k_data += 3;
		}
	}

	fwrite(&header, sizeof(header), 1, fp);
	fwrite(data, sizeof(uint8_t), size * 3, fp);
	fclose(fp);

	delete[] data;
}

void TextureBuffer::save(const char* fileName)
{
	if (!valid) {
		PLOGE << "Couldn't read texture";
		return;
	}

	if (type == Type::ScreenShot)
	{
		return saveScreenshot(fileName);
	}
	else if (type == Type::Depth) 
	{
		return saveDepth(fileName);
	}

	PLOGE << "Invalid texture type";
}

const size_t TextureBuffer::GetSize()
{
	if (valid)
	{
		return desc.Width * desc.Height;
	}

	return -1;
}
