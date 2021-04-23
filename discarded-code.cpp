#pragma region Discarded DirectInput8 hook

#include "dinput8_hook/dinput8_hook.h"

typedef HRESULT(__stdcall* GetDeviceState)(IDirectInputDevice8*, DWORD, LPVOID);
static GetDeviceState oGetDeviceState = NULL;

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData)
{
    HRESULT hr = oGetDeviceState(pDevice, cbData, lpvData);
    std::cout << "GetDeviceState Hook called: " << std::hex << hr << std::endl;

    if (hr == DI_OK) {
         return hook->GetDeviceState(pDevice, cbData, lpvData);
    }

    return hr;
}

typedef HRESULT(__stdcall* GetDeviceData)(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
static GetDeviceData oGetDeviceData = NULL;

HRESULT _stdcall hkGetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    HRESULT hr = oGetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    std::cout << "GetDeviceData Hook called: " << std::hex << hr << std::endl;
    return hr;
}

/**
 * The game theoretically uses Direct Input 8, but all attempts to hook
 * it crashes the game
std::cout << "Hooking DInput8 GetDeviceState" << std::endl;
bool init_dinput_hook = false;
do
{
    if (dinput8_hook::init() == dinput8_hook::Status::Success)
    {
        dinput8_hook::bind(9, (void**)&oGetDeviceState, hkGetDeviceState);
        dinput8_hook::bind(10, (void**)&oGetDeviceData, hkGetDeviceData);
        init_dinput_hook = true;
    }
} while (!init_dinput_hook);
std::cout << "DInput8 GetDeviceState successfully hooked" << std::endl;

*/


/*
HRESULT ETS2Hook::GetDeviceState(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData)
{
	std::cout << "ETS2Hook::GetDeviceState called" << std::endl;

	if (cbData == sizeof(DIMOUSESTATE)) {
		LPDIMOUSESTATE mouseState = (LPDIMOUSESTATE)lpvData;
		if (mouseState->rgbButtons[0] != 0) {
			// mouse button 0 pressedstd
			std::cout << "[LMB]" << std::endl;
		}
		if (mouseState->rgbButtons[1] != 0) {
			// mouse button 1 pressed
			std::cout << "[RMB]" << std::endl;
		}
	}
	if (cbData == sizeof(DIMOUSESTATE2)) {
		LPDIMOUSESTATE2 mouseState = (LPDIMOUSESTATE2)lpvData;
		if (mouseState->rgbButtons[0] != 0) {
			// mouse button 0 pressed
			std::cout << "[LMB2]" << std::endl;
		}
		if (mouseState->rgbButtons[1] != 0) {
			// mouse button 1 pressed
			std::cout << "[RMB2]" << std::endl;
		}
	}

	return DI_OK;
}
*/
#pragma endregion

#pragma region Previous code to initialize after injection, discarded as ETS2 telemetry calls initialization after injection
//int DLLMainThread() {    
//    std::cout << "DLL Injected" << std::endl;
//
//    std::cout << "Hooking DX11 Present" << std::endl;
//
//    bool init_hook = false;
//    do
//    {
//        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
//        {
//            kiero::bind(8, (void**)&oPresent, hkPresent11);
//            init_hook = true;
//        }
//    } while (!init_hook);
//
//    std::cout << "DX11 Present successfully hooked" << std::endl;
//
//    return TRUE;
//}
#pragma endregion

#pragma region Screenshot through buffer copy
HRESULT ETS2Hook::TakeScreenshot(IDXGISwapChain* pSwapChain, const wchar_t* fileName)
{
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pScreenshotTexture);

	if (FAILED(hr) || pScreenshotTexture == NULL) {
		PLOGE << "Error capturing buffer for screenshot to " << fileName;
	}
	else {
		ID3D11Texture2D* pTextureCopy = NULL;
		D3D11_TEXTURE2D_DESC textureDescription;
		pScreenshotTexture->GetDesc(&textureDescription);
		textureDescription.BindFlags = 0;
		textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDescription.Usage = D3D11_USAGE_STAGING;

		hr = pDevice->CreateTexture2D(&textureDescription, NULL, &pTextureCopy);
		
		if (FAILED(hr) || pTextureCopy == NULL) {
			PLOGE << "Error creating texture copy for screenshot to " << fileName;
		}
		else {
			pDeviceContext->CopyResource(pTextureCopy, pScreenshotTexture);		
			//D3D11_MAPPED_SUBRESOURCE resource;
			//unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
			//hr = pDeviceContext->Map(pTextureCopy, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
			hr = SaveWICTextureToFile(pDeviceContext, pTextureCopy, GUID_ContainerFormatJpeg, fileName);
		}

		pTextureCopy->Release();
	}

	return hr;
}
#pragma endregion