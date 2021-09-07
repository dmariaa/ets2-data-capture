#pragma once

namespace ets2dc_dx11hook 
{
	typedef void(__stdcall* ClearDepthStencilView)(ID3D11DeviceContext*, ID3D11DepthStencilView*, UINT, FLOAT, UINT8);
	typedef void(__stdcall* Draw)(ID3D11DeviceContext*, UINT, UINT);
	typedef long(__stdcall* CreateTexture2D)(ID3D11Device*, D3D11_TEXTURE2D_DESC*, D3D11_SUBRESOURCE_DATA*, ID3D11Texture2D**);
	typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
	typedef void(__stdcall* ClearRenderTargetView)(ID3D11DeviceContext*, ID3D11RenderTargetView*, const FLOAT[4]);

	void init();
	void shutdown();
}
