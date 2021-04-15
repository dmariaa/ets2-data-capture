#include "ETS2Hook.h"

ETS2Hook::ETS2Hook()
{
}

HRESULT ETS2Hook::Init(IDXGISwapChain* swapChain)
{
	HRESULT hr = S_OK;

	if (pDevice == nullptr) {
		hr = swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);

		if (SUCCEEDED(hr)) {
			pDevice->GetImmediateContext(&pDeviceContext);
		}
	}

	return hr;
}

HRESULT ETS2Hook::RenderIMGUI()
{
}

HRESULT ETS2Hook::TakeScreenshot(IDXGISwapChain* swapChain)
{
}
