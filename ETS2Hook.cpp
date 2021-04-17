#include "ETS2Hook.h"

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <wrl/client.h>
#include <wincodec.h>
#include <ShlObj_core.h>

#include "config.h"
#include "ScreenGrab.h"
#include "log.h"

using namespace DirectX;
using namespace Microsoft::WRL;

ETS2Hook::ETS2Hook() : capturing(false), totalFramesCaptured(0)
{
	// read config data
	Config config;
	config.load();

	imageFolder = config.get<std::wstring>("ImageFolder", L"images");
	imageFileFormat = config.get<std::wstring>("ImagefileFormat", L"jpg");
	imageFileName = config.get<std::wstring>("ImageFileName", L"file{date}{time}");
	consecutiveFramesCapture = config.get<int>("ConsecutiveFrames", 3);
	secondsBetweenCaptures = config.get<int>("SecondsBetweenCaptures", 1);	

	std::cout << "Configuration -------------------------------" << std::endl;
	std::wcout << "Image folder: " << imageFolder << std::endl;
	std::wcout << "Image file Format: " << imageFileFormat << std::endl;
	std::wcout << "Image file Name: " << imageFileName << std::endl;
	std::wcout << "Consecutive frames: " << consecutiveFramesCapture << std::endl;
	std::wcout << "Seconds Between Captures: " << secondsBetweenCaptures << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
}

HRESULT ETS2Hook::Init(IDXGISwapChain* pSwapChain)
{
	HRESULT hr = S_OK;

	if (pDevice == nullptr) {
		hr = pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);

		if (SUCCEEDED(hr)) {
			pDevice->GetImmediateContext(&pDeviceContext);
		}
	}

	return hr;
}

HRESULT ETS2Hook::RenderIMGUI()
{
	return E_NOTIMPL;
}

/// <summary>
/// Takes a screenshot from the current swapChain
/// </summary>
/// <param name="pSwapChain"></param>
/// <param name="fileName">L"C:\\Users\\david\\Documents\\ETS2_CAPTURE\\captures\\screenshot.jpg"</param>
/// <returns></returns>
HRESULT ETS2Hook::TakeScreenshot(IDXGISwapChain* pSwapChain, const wchar_t* fileName)
{
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pScreenshotTexture);

	if (FAILED(hr) || pScreenshotTexture == nullptr) {
		std::cout << "Error capturing buffer for screenshot to " << fileName << std::endl;
	}

	hr = SaveWICTextureToFile(pDeviceContext, pScreenshotTexture, GUID_ContainerFormatJpeg, fileName);
	DX::ThrowIfFailed(hr);

	return hr;
}

static int frame = 0;
static clock_t timer;

HRESULT ETS2Hook::Present(IDXGISwapChain* swapChain)
{
	if ((GetAsyncKeyState(VK_CONTROL) & 1) && (GetAsyncKeyState(0xDC) & 1)) {
		capturing = !capturing;
		timer = clock();
	}

	if (capturing) {
		if (frame < consecutiveFramesCapture) {
			imageFile.str(std::wstring());
			imageFile << imageFolder << L"\\" << imageFileName << totalFramesCaptured << L"." << imageFileFormat;
			
			std::wstring fileName = imageFile.str();

			 std::wcout << L"Saving ETS2 frame to " << fileName << std::endl;
			 TakeScreenshot(swapChain, fileName.c_str());

			frame++;
			totalFramesCaptured++;
		}


		if ((clock() - timer) / CLOCKS_PER_SEC >= secondsBetweenCaptures) {
			timer = clock();
			frame = 0;
		}
	}

	return E_NOTIMPL;
}
