#pragma once
#include <d3d11.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

class Config;

class ETS2Hook
{
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11Texture2D* pScreenshotTexture;

	bool capturing;
	int totalFramesCaptured;

	// Configuration vars
	int consecutiveFramesCapture;
	int secondsBetweenCaptures; 	
	std::wstring imageFolder;
	std::wstring imageFileFormat;
	std::wstring imageFileName;

	std::wstringstream imageFile;
public:
	ETS2Hook();
	
	HRESULT Init(IDXGISwapChain* swapChain);
	HRESULT RenderIMGUI();
	HRESULT TakeScreenshot(IDXGISwapChain* swapChain, const wchar_t* fileName);
	HRESULT Present(IDXGISwapChain* swapChain);
};

