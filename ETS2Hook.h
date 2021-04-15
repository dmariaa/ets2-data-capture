#pragma once
#include <d3d11.h>

class ETS2Hook
{
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;

public:
	ETS2Hook();
	
	HRESULT Init(IDXGISwapChain* swapChain);
	HRESULT RenderIMGUI();
	HRESULT TakeScreenshot(IDXGISwapChain* swapChain);
};

